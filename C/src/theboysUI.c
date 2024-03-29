/*
* GUI aprimorada para projeto 'The Boys'
* Autor: Fernando de Barros Castro
* Data: 16/01/2024
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ncurses.h>
#include "conjunto.h"
#include "lef.h"
#include "fila.h"
#include "movement.h"

#define N_HABILIDADES 10
#define N_HEROIS N_HABILIDADES * 4
#define N_BASES 8
#define T_INICIO 0
#define T_FIM_DO_MUNDO 5
#define T_MIN_CHEGADA_H 0
#define T_MAX_CHEGADA_H 0
#define N_MISSOES T_FIM_DO_MUNDO 
#define MIN_HABILIDADES_H 1
#define MAX_HABILIDADES_H 3
#define MIN_VELOCIDADE_H 3
#define MAX_VELOCIDADE_H 5
#define MIN_PACIENCIA_H 0
#define MAX_PACIENCIA_H 5
#define MIN_HABILIDADES_M 2
#define MAX_HABILIDADES_M 6
#define MIN_HEROIS_B 3
#define MAX_HEROIS_B 10

#define MIN_X 2
#define MIN_Y 2
#define MAIOR_DIST 180

struct heroi
{
    int id;
    int base;
    int experiencia;
    int paciencia;
    int velocidade;
    struct coordenadas posicao;
    struct conjunto *habilidades;
};

struct base
{
    int id;
    int lotacaoMax;
    struct coordenadas local;
    struct conjunto *presentes;
    struct fila *filaEspera;
};

struct missao
{
    int id;
    int resolvida;
    int nAgendamentos;
    struct coordenadas local;
    struct conjunto *habilidades;
};

struct mundo
{
    int nHerois;
    int nBases;
    int nMissoes;
    int nMissoesResolvidas;
    int nHabilidades;
    int relogio;
    struct heroi herois[N_HEROIS];
    struct base bases[N_BASES];
    struct missao missoes[N_MISSOES];
};

/* inicio funcoes auxiliares */

/* retorna um pseudo aleatorio do conjunto fechado [min, max] */
int aleat (int min, int max)
{
    return min + rand() % (max - min + 1);
}

int calculaDist (struct coordenadas a, struct coordenadas b)
{
    return (int)sqrt ((a.col - b.col) * (a.col - b.col) + (a.lin - b.lin) * (a.lin - b.lin));
}

/* retorna a posicao do menor elemento de vet */
int achaMenorPosVet (int vet[], int tam)
{
    int i, menorPos;

    menorPos = 0;
    for (i = 1; i < tam; i++)
        if (vet[menorPos] > vet[i])
            menorPos = i;

    return menorPos;
}

/* sorteia 'y' da base e retorna 0 se alguma 
 * base ja tem o mesmo 'y' e 1 caso contrario */
int ajustaY (struct mundo *s, struct base *b, int num)
{
    int i = 0, verifica = 1;

    b->local.lin = aleat (MIN_Y, LINES - 2);
    while (verifica && i < num)
    {
        verifica = (s->bases[i].local.lin != b->local.lin);
        i++;
    }

    return verifica;
}

/* fim funcoes auxiliares */

struct heroi criaHeroi (int id)
{
    struct heroi h;

    h.id = id;
    h.experiencia = 0;
    h.posicao.col = h.posicao.lin = MIN_X;
    h.velocidade = aleat (MIN_VELOCIDADE_H, MAX_VELOCIDADE_H);
    h.paciencia = aleat (MIN_PACIENCIA_H, MAX_PACIENCIA_H);
    h.habilidades = cria_cjt (aleat (MIN_HABILIDADES_H, MAX_HABILIDADES_H));
    while (insere_cjt (h.habilidades, aleat (0, N_HABILIDADES - 1)));

    return h;
}

struct base criaBase (int id, struct coordenadas pos, WINDOW *j)
{
    struct base b;

    b.id = id;
    b.lotacaoMax = aleat (MIN_HEROIS_B, MAX_HEROIS_B);
    b.filaEspera = fila_cria ();
    b.presentes = cria_cjt (b.lotacaoMax);
    b.local.col = pos.col;
    b.local.lin = pos.lin;

    id += 65;
    mvwaddch (j, b.local.lin, b.local.col, (char)id);
    mvwaddch(j, b.local.lin - 1, b.local.col - 2, ACS_ULCORNER);
    mvwaddch(j, b.local.lin - 1, b.local.col + 2, ACS_URCORNER);
    mvwaddch(j, b.local.lin + 1, b.local.col - 2, ACS_LLCORNER);
    mvwaddch(j, b.local.lin + 1, b.local.col + 2, ACS_LRCORNER);
    mvwaddch(j, b.local.lin, b.local.col - 2, ACS_VLINE);
    mvwaddch(j, b.local.lin, b.local.col + 2, ACS_VLINE);
    mvwhline (j, b.local.lin - 1, b.local.col - 1, ACS_HLINE, 3);
    mvwhline (j, b.local.lin + 1, b.local.col - 1, ACS_HLINE, 3);

    return b;
}   

struct missao criaMissao (int id)
{
    struct missao m;

    m.id = id;
    m.nAgendamentos = 1;
    m.resolvida = 0;
    m.local.col = aleat (MIN_X, COLS - 2);
    m.local.lin = aleat (MIN_Y, LINES - 2);
    m.habilidades = cria_cjt (aleat (MIN_HABILIDADES_M, MAX_HABILIDADES_M));
    while (insere_cjt (m.habilidades, aleat (0, N_HABILIDADES - 1)));

    return m;  
}

void inicializaMundo (struct mundo *s, struct lef_t *eventos, WINDOW *j)
{
    int i, b;
    struct coordenadas posBases[N_BASES];

    s->nHabilidades = N_HABILIDADES;
    s->nHerois = N_HEROIS;
    s->nBases = N_BASES;
    s->nMissoes = N_MISSOES;
    s->nMissoesResolvidas = s->relogio = T_INICIO;

    mvwprintw (j, LINES / 2, (COLS / 2) - 4, "THE BOYS");
    wrefresh (j);
    napms (1000);

    werase (j);
    box (j, 0, 0);
    wrefresh (j);
    
    /* insere na lef no tempo T_FIM_DO_MUNDO 
     * o evento que termina a simulacao */
    insere_lef (eventos, cria_evento (T_FIM_DO_MUNDO, 9, 0, 0));

    /* cria e posiciona as bases na janela */
    posBases[0].col = 0.08 * COLS; posBases[0].lin = 0.13 * LINES; 
    posBases[1].col = 0.21 * COLS; posBases[1].lin = 0.32 * LINES;
    posBases[2].col = 0.31 * COLS; posBases[2].lin = 0.54 * LINES;
    posBases[3].col = 0.43 * COLS; posBases[3].lin = 0.37 * LINES;
    posBases[4].col = 0.54 * COLS; posBases[4].lin = 0.56 * LINES;
    posBases[5].col = 0.66 * COLS; posBases[5].lin = 0.72 * LINES;
    posBases[6].col = 0.78 * COLS; posBases[6].lin = 0.54 * LINES;
    posBases[7].col = 0.89 * COLS; posBases[7].lin = 0.32 * LINES;

    for (i = 0; i < s->nBases; i++)
        s->bases[i] = criaBase (i, posBases[i], j);
    wrefresh (j);

    /* cria os herois e sorteia o tempo de chegada 
     * de cada um em uma base tambem sorteada */
    for (i = 0; i < s->nHerois; i++)
    {
        s->herois[i] = criaHeroi (i);
        b = aleat (0, s->nBases - 1);
        insere_lef (eventos, cria_evento (aleat (T_MIN_CHEGADA_H, T_MAX_CHEGADA_H), 3, i, b));
    }

    /* cria as missoes e sorteia para cada 
     * uma quando devera ser realizada */
    for (i = 0; i < s->nMissoes; i++)
    {
        s->missoes[i] = criaMissao (i);
        insere_lef (eventos, cria_evento (i, 2, i, 0));
    }
}

void primeiroHeroiChega (struct mundo *s, struct evento_t *eventoTemp, struct lef_t *e, WINDOW *j)
{
    int h = eventoTemp->dado1, b = eventoTemp->dado2;
    int c = cardinalidade_cjt (s->bases[b].presentes);
    int p = s->herois[h].paciencia;
    int lMax = s->bases[b].lotacaoMax;
    struct fila *f = s->bases[b].filaEspera;
    struct coordenadas *cAux = &s->herois[h].posicao;

    if ((c < lMax && fila_vazia (f)) || p > fila_tamanho (f))
    {
        cAux->lin = s->bases[b].local.lin + fila_tamanho(s->bases[b].filaEspera) + 2;
        cAux->col = s->bases[b].local.col;
        mvwprintw (j, cAux->lin, cAux->col, "%d", h);
        wrefresh (j);
        napms (50);

        enqueue (s->bases[b].filaEspera, h);
        insere_lef (e, cria_evento (s->relogio, 5, h, b));
        
        return;
    }

    insere_lef (e, cria_evento (s->relogio, 4, h, b));

    return;
}

void viajaEchega (struct mundo *s, struct evento_t *eventoTemp, struct lef_t *e, WINDOW *j)
{
    int h = eventoTemp->dado1;
    int b = eventoTemp->dado2;
    int vel = s->herois[h].velocidade; 
    int c = cardinalidade_cjt (s->bases[b].presentes);
    int p = s->herois[h].paciencia;
    int lMax = s->bases[b].lotacaoMax;
    struct fila *f = s->bases[b].filaEspera;
    struct coordenadas posDest;

    posDest.col = s->bases[b].local.col;
    posDest.lin = s->bases[b].local.lin + fila_tamanho (s->bases[b].filaEspera) + 2;

    mvwprintw (j, s->herois[h].posicao.lin, s->herois[h].posicao.col, "%d", h);
    wrefresh (j);
    napms (100 / vel);

    movimentaHeroi (posDest, s->herois[h].posicao, j, h, vel);
    s->herois[h].posicao = posDest;
    s->herois[h].base = b;

    napms (200);

    if ((c < lMax && fila_vazia (f)) || p > fila_tamanho (f))
    {
        /* adicionado a fila de espera e porteiro avisado de sua chegada */
        insere_lef (e, cria_evento (s->relogio, 5, h, b));
        enqueue (s->bases[b].filaEspera, h);

        return;
    }

    insere_lef (e, cria_evento (s->relogio, 4, h, b));
    mvwprintw (j, s->herois[h].posicao.lin, s->herois[h].posicao.col, "  ");
    wrefresh (j);
    
    return;
}

void avisaEentra (struct mundo *s, struct evento_t *eventoTemp, struct lef_t *e, WINDOW *j)
{
    int i, tempo;
    int t = s->relogio;
    int h = eventoTemp->dado1; 
    int b = eventoTemp->dado2;
    int c = cardinalidade_cjt (s->bases[b].presentes);
    int lMax = s->bases[b].lotacaoMax;
    struct coordenadas *ptrPos;

    /* porteiro libera entrada se houver vaga
     * na base e heroi(s) esperando na fila */
    while (c < lMax && dequeue (s->bases[b].filaEspera, &h))
    {
        mvwprintw (j, s->herois[h].posicao.lin, s->herois[h].posicao.col, "  ");
        wrefresh (j);
        napms (100);

        insere_cjt (s->bases[b].presentes, h);
        c++;

        tempo = t + s->herois[h].paciencia + 1;
        insere_lef (e, cria_evento (tempo, 7, h, b));
    }

    i = 1;
    h = retorna_elemento_fila (s->bases[b].filaEspera, i);

    while (h != -1)
    {
        ptrPos = &s->herois[h].posicao;
        while ((int)(A_CHARTEXT & mvwinch (j, ptrPos->lin - 1, ptrPos->col)) == 32)
        {
            mvwprintw (j, ptrPos->lin, ptrPos->col, "  ");
            mvwprintw (j, --ptrPos->lin, ptrPos->col, "%d", h);
            wrefresh (j);
            napms (50);
        }

        i++;
        h = retorna_elemento_fila (s->bases[b].filaEspera, i);
    }

    if (i != 1)
        napms (200);
}

void heroiDesiste (struct mundo *s, struct evento_t *eventoTemp, struct lef_t *e)
{
    int h = eventoTemp->dado1;
    int b = eventoTemp->dado2;

    /* uma base qualquer eh sorteada como novo destino do heroi */
    insere_lef (e, cria_evento (s->relogio + aleat(1, 5), 1, h, aleat (0, s->nBases - 1)));

    /* posiciona a esquerda da base */
    s->herois[h].posicao.col = s->bases[b].local.col - 4;
    s->herois[h].posicao.lin = s->bases[b].local.lin;
}

void heroiSai (struct mundo *simulacao, struct evento_t *eventoTemp, struct lef_t *e)
{
    int t = simulacao->relogio;
    int h = eventoTemp->dado1;
    int b = eventoTemp->dado2;

    /* heroi eh retirado do conjunto de presentes da base */
    retira_cjt (simulacao->bases[b].presentes, h);

    /* uma nova base destino eh sorteada para heroi e o 
     * porteiro avisado de que ha mais uma vaga na base */
    insere_lef (e, cria_evento (t, 1, h, aleat (0, simulacao->nBases - 1)));
    insere_lef (e, cria_evento (t, 5, h, b));
    /* posiciona a direita da base */
    simulacao->herois[h].posicao.col = simulacao->bases[b].local.col + 3;
    simulacao->herois[h].posicao.lin = simulacao->bases[b].local.lin;
}

void missao (struct mundo *s, struct evento_t *eventoTemp, struct lef_t *e, WINDOW **janela)
{
    int dists[s->nBases];
    int j, menorPos, hId, achou, l, c;
    int mId = eventoTemp->dado1;
    struct conjunto *aux, *habilidadesB;
    int ch;
    WINDOW *janelaSalva = newwin (LINES, COLS, 0, 0);

    ch = mvwinch (*janela, s->missoes[mId].local.lin, s->missoes[mId].local.col);
    
    wattron (*janela, A_BOLD);
    mvwaddch (*janela, s->missoes[mId].local.lin, s->missoes[mId].local.col, 'M');
    wrefresh (*janela);
    napms (700);

    wattroff (*janela, A_BOLD);
    mvwaddch (*janela, s->missoes[mId].local.lin, s->missoes[mId].local.col, ch);
    wrefresh (*janela);

    /* a distancia da base 'i' ate a missao eh guardada em dists[i] */
    for (j = 0; j < s->nBases; j++)
        dists[j] = calculaDist (s->bases[j].local, s->missoes[mId].local);

    /* loop procura pela base mais proxima que possa executar a missao */
    achou = j = 0;
    while (!achou && j < s->nBases)
    {
        habilidadesB = cria_cjt (s->nHabilidades);
        menorPos = achaMenorPosVet (dists, s->nBases);
        /* apos achar a base 'i' mais perto, ela eh descartada 
         * como possibilidade para a proxima iteracao */
        dists[menorPos] = MAIOR_DIST;

        /* as diferentes habilidades dos herois da base 
         * 'i' sao reunidas no conjunto habilidadesB */
        inicia_iterador_cjt (s->bases[menorPos].presentes);
        while (incrementa_iterador_cjt (s->bases[menorPos].presentes, &hId))
        {
            aux = habilidadesB;
            habilidadesB = uniao_cjt (habilidadesB, s->herois[hId].habilidades);
            destroi_cjt (aux);
        }

        /* imprime_cjt (habilidadesB); 
         * verifica se a base 'i' eh compativel com a missao */
        achou = contido_cjt (s->missoes[mId].habilidades, habilidadesB);
        destroi_cjt (habilidadesB);
        j++;
    }

    for (l = 0; l <= LINES; l++)
    {
        for (c = 0; c <= COLS; c++)
        {
            ch = mvwinch (*janela, l, c);
            mvwaddch (janelaSalva, l, c, ch);
            mvwprintw (*janela, l, c, " ");
        }
    }
    if (achou)
    {
        mvwprintw (*janela, LINES / 2, (COLS / 2) - 13, "MISSAO %d CUMPRIDA POR BASE %c", mId, (char)(menorPos + 65));

        /* incrementa em 1 a experiencia dos herois que executaram a missao */
        inicia_iterador_cjt (s->bases[menorPos].presentes);
        while (incrementa_iterador_cjt (s->bases[menorPos].presentes, &hId))
            ++s->herois[hId].experiencia;

        ++s->nMissoesResolvidas;
        s->missoes[mId].resolvida = 1;
    }
    else
    {
        mvwprintw (*janela, LINES / 2, (COLS / 2) - 11, "MISSAO %d IMPOSSIVEL", mId);
        insere_lef (e, cria_evento (s->relogio + aleat (5, 10), 2, mId, 0));
        ++s->missoes[mId].nAgendamentos;
    }
    
    wrefresh (*janela);
    napms(1200);
    delwin (*janela);
    *janela = janelaSalva;
    wrefresh (*janela);
}

void fimSimulacao (struct mundo *s, struct evento_t **eventoTemp, struct lef_t **e, WINDOW *j)
{
    int i, somaAgendamentos = 0;

    i = 4;
    werase (j);
    box (j, 0, 0);
    mvwprintw (j, (LINES / 2) + 2, (COLS / 2) - 10, "%d/%d MISSOES CUMPRIDAS", s->nMissoesResolvidas, s->nMissoes);
    mvwprintw (j, (LINES / 2) + 4, (COLS / 2) - 6, "(%.2f%%), MEDIA", (float)(100*s->nMissoesResolvidas / s->nMissoes));
    while (i--)
    {
        mvwprintw (j, LINES / 2, (COLS / 2) - 1, "FIM");
        wrefresh (j);
        napms (500);
        
        mvwprintw (j, LINES / 2, (COLS / 2) - 1, "   ");
        wrefresh (j);
        napms (500);
    }

    /* abaixo toda a memoria alocada eh liberada */
    for (i = 0; i < s->nHerois; i++)
        s->herois[i].habilidades = destroi_cjt (s->herois[i].habilidades);

    for (i = 0; i < s->nBases; i++)
    {
        s->bases[i].presentes = destroi_cjt (s->bases[i].presentes);
        fila_destroi (&s->bases[i].filaEspera);
    }

    for (i = 0; i < s->nMissoes; i++)
        s->missoes[i].habilidades = destroi_cjt (s->missoes[i].habilidades);

    /* aterrando os ponteiros de eventoTemp e eventos */
    *eventoTemp = destroi_evento (*eventoTemp);
    *e = destroi_lef (*e);

    /* loop soma todos os agendamentos que levaram a missoes cumpridas */
    for (i = 0; i < s->nMissoes; i++)
        if (s->missoes[i].resolvida)
            somaAgendamentos += s->missoes[i].nAgendamentos;
}

int main ()
{
    WINDOW *janela;
    struct evento_t *eventoTemp;
    struct lef_t *eventos;
    struct mundo simulacao;

    /* inicializa a tela */
    initscr ();
    noecho ();
    cbreak ();
    curs_set (0);
    refresh();

    /* inicializa a janela */
    janela = newwin (LINES, COLS, 0, 0);
    box (janela, 0, 0);
    wrefresh (janela);

    /* inicializa semente randomica */
    srand (0); 

    eventos = cria_lef ();
    inicializaMundo (&simulacao, eventos, janela);

    /* verifica falha na alocacao da lef */
    eventoTemp = retira_lef (eventos);
    if (!eventoTemp)
        return -1;
    
    while (simulacao.relogio < T_FIM_DO_MUNDO)
    {
        while (eventoTemp->tempo == simulacao.relogio)
        {
            switch (eventoTemp->tipo)
            {
                case 1:
                    viajaEchega (&simulacao, eventoTemp, eventos, janela);
                    break;
                case 2:
                    missao (&simulacao, eventoTemp, eventos, &janela);
                    break;
                case 3:
                    primeiroHeroiChega (&simulacao, eventoTemp, eventos, janela);
                    break;
                case 4:
                    heroiDesiste (&simulacao, eventoTemp, eventos);
                    break;
                case 5:
                    avisaEentra (&simulacao, eventoTemp, eventos, janela);
                    break;
                case 7:
                    heroiSai (&simulacao, eventoTemp, eventos);
                    break;
                default:
                    return -1; 
            }
            destroi_evento (eventoTemp);

            eventoTemp = retira_lef (eventos);
            if (!eventoTemp)
                return -1;
        }

        ++simulacao.relogio;
    }

    fimSimulacao (&simulacao, &eventoTemp, &eventos, janela);
    delwin (janela);
    endwin (); 

    return 0;    
}
