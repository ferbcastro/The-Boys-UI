#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ncurses.h>
#include <float.h>
#include "movement.h"

/* 
* inicio funcoes para lidar com conjunto
*/

conjuntoP *cria_cjtP (int max)
{
    conjuntoP *novoCjt;
    par *vetTemp;

    novoCjt = malloc (sizeof(conjuntoP));
    if (!novoCjt)
        return NULL;

    vetTemp = malloc (max * sizeof(par));
    novoCjt->max = max;
    novoCjt->card = 0;
    novoCjt->v = vetTemp;

    return novoCjt;
}

par make_par (double f, int lin, int col)
{
    par pTemp;
    struct coordenadas cTemp;

    pTemp.first = f;
    cTemp.lin = lin;
    cTemp.col = col;
    pTemp.second = cTemp;

    return pTemp;
}

conjuntoP *destroi_cjtP (conjuntoP *c)
{
    if (!c)
        return NULL;
    
    if (c->v)
        free (c->v);
    free (c);

    return NULL;
}

int push_cjt (conjuntoP *c, par p)
{
    int i, ini, fim, meio;
    double f = p.first;

    if (!c || !c->v)
        return 0;

    ini = 0;
    fim = c->card - 1;
    while (ini <= fim)
    {
        meio = (ini + fim) / 2;
        if (f >= c->v[meio].first)
            ini = meio + 1;
        else 
            fim = meio - 1;
    }

    for (i = c->card - 1; i >= ini; i--)
        c->v[i + 1] = c->v[i];

    c->v[ini] = p;
    ++c->card;

    return 1;
}

par pop_cjt (conjuntoP *c)
{
    int i;
    par pTemp;

    pTemp = c->v[0];
    for (i = 0; i < c->card - 1; i++)
        c->v[i] = c->v[i + 1];
    --c->card;

    return pTemp;
}

int vazio_cjtP (conjuntoP *c)
{
    if (!c)
        return 1;
    
    return (c->card == 0);
}

/* 
* fim funcoes para lidar com conjunto 
*/

/* 
* inicio funcoes para lidar com pilha 
*/

pilhaP *cria_pilhaP ()
{
    pilhaP *ptrTemp;

    ptrTemp = malloc (sizeof (pilhaP));
    if (!ptrTemp)
        return NULL;
    
    ptrTemp->primeiro = NULL;

    return ptrTemp;
}

int push_pilha (pilhaP *pilha, struct coordenadas posAux)
{
    elementoP *ptrTemp;

    if (!pilha)
        return 0;

    ptrTemp = malloc (sizeof (elementoP));
    if (!ptrTemp)
        return 0;
    
    ptrTemp->prox = pilha->primeiro;
    pilha->primeiro = ptrTemp;
    ptrTemp->pos = posAux;

    return 1;
}   

int pop_pilha (pilhaP *pilha, struct coordenadas *posAux)
{
    elementoP *ptrTemp;

    if (!pilha || !pilha->primeiro)
        return 0;

    ptrTemp = pilha->primeiro;
    pilha->primeiro = pilha->primeiro->prox;
    *posAux = ptrTemp->pos;
    free (ptrTemp);

    return 1;    
}

pilhaP *destroi_pilhaP (pilhaP *pilha)
{
    elementoP *ptrTemp;

    if (!pilha)
        return NULL;

    while (pilha->primeiro)
    {
        ptrTemp = pilha->primeiro;
        pilha->primeiro = pilha->primeiro->prox;
        free (ptrTemp);
    }
    
    free (pilha);

    return NULL;
}

/*
* fim funcoes para lidar com pilha 
*/

/* 
* inicio algoritmo A* e funcoes auxiliares 
*/

/* calcula distancia cartesiana entre dois pontos (x, y) */
double calculaDistDouble (struct coordenadas a, struct coordenadas b)
{
    return (double)sqrt ((a.col - b.col) * (a.col - b.col) + (a.lin - b.lin) * (a.lin - b.lin));
}

void movimentaHeroi (struct coordenadas posDest, struct coordenadas posOrigem, WINDOW *janela, int h, int velH)
{
    int i, j, achou = 0;
    double gNovo, hNovo, fNovo;
    int grid[LINES + 1][COLS + 1];
    struct celula tela[LINES + 1][COLS + 1];
    struct coordenadas cTemp;
    conjuntoP *listaAberta = cria_cjtP ((LINES + 1) * (COLS + 1));
    pilhaP *caminho;
    par pTemp;

    for (i = 0; i <= LINES; i++)
    {
        for (j = 0; j <= COLS; j++)
        {
            grid[i][j] = ((int)(A_CHARTEXT & mvwinch (janela, i, j)) == 32);
            tela[i][j].pai.lin = -1;
            tela[i][j].pai.col = -1;
            tela[i][j].f = FLT_MAX;
            tela[i][j].g = FLT_MAX;
            tela[i][j].vista = 0;
        }
    }

    i = posOrigem.lin;
    j = posOrigem.col;     
    tela[i][j].pai.lin = i;
    tela[i][j].pai.col = j;
    tela[i][j].f = 0;
    tela[i][j].g = 0;

    push_cjt (listaAberta, make_par (0, i, j));
    while (!vazio_cjtP (listaAberta) && !achou)
    {
        pTemp = pop_cjt (listaAberta);
        i = pTemp.second.lin;
        j = pTemp.second.col;
        tela[i][j].vista = 1;

        /* sul */
        if (i + 1 < LINES)
        {
            if (posDest.lin == (i + 1) && posDest.col == j)
            {
                achou = 1;
                tela[i + 1][j].pai.lin = i;
                tela[i + 1][j].pai.col = j;
                continue;
            }
            else if (!tela[i + 1][j].vista && grid[i + 1][j] && grid[i + 1][j + 1])
            {
                cTemp.col = j;
                cTemp.lin = i + 1;
                gNovo = tela[i][j].g + 1;
                hNovo = calculaDistDouble (cTemp, posDest);
                fNovo = gNovo + hNovo;

                if (tela[i + 1][j].f == FLT_MAX || tela[i + 1][j].f > fNovo)
                {
                    push_cjt (listaAberta, make_par (fNovo, i + 1, j));
                    tela[i + 1][j].f = fNovo;
                    tela[i + 1][j].g = gNovo;
                    tela[i + 1][j].pai.lin = i;
                    tela[i + 1][j].pai.col = j;
                }
            }
        }
        /* norte */
        if (i - 1 > 0)
        {
            if (posDest.lin == (i - 1) && posDest.col == j)
            {
                achou = 1;
                tela[i - 1][j].pai.lin = i;
                tela[i - 1][j].pai.col = j;
                continue;
            }
            else if (!tela[i - 1][j].vista && grid[i - 1][j] && grid[i - 1][j + 1])
            {
                cTemp.col = j;
                cTemp.lin = i - 1;
                gNovo = tela[i][j].g + 1;
                hNovo = calculaDistDouble (cTemp, posDest);
                fNovo = gNovo + hNovo;

                if (tela[i - 1][j].f == FLT_MAX || tela[i - 1][j].f > fNovo)
                {
                    push_cjt (listaAberta, make_par (fNovo, i - 1, j));
                    tela[i - 1][j].f = fNovo;
                    tela[i - 1][j].g = gNovo;
                    tela[i - 1][j].pai.lin = i;
                    tela[i - 1][j].pai.col = j;
                }
            }
        }
        /* leste */
        if (j + 1 < COLS)
        {
            if (posDest.lin == i && posDest.col == (j + 1))
            {
                achou = 1;
                tela[i][j + 1].pai.lin = i;
                tela[i][j + 1].pai.col = j;
                continue;
            }
            else if (!tela[i][j + 1].vista && grid[i][j + 1] && grid[i][j + 2])
            {
                cTemp.col = j + 1;
                cTemp.lin = i;
                gNovo = tela[i][j].g + 1;
                hNovo = calculaDistDouble (cTemp, posDest);
                fNovo = gNovo + hNovo;

                if (tela[i][j + 1].f == FLT_MAX || tela[i][j + 1].f > fNovo)
                {
                    push_cjt (listaAberta, make_par (fNovo, i, j + 1));
                    tela[i][j + 1].f = fNovo;
                    tela[i][j + 1].g = gNovo;
                    tela[i][j + 1].pai.lin = i;
                    tela[i][j + 1].pai.col = j;
                }
            }
        }
        /* oeste */
        if (j - 1 > 0)
        {
            if (posDest.lin == i && posDest.col == (j - 1))
            {
                achou = 1;
                tela[i][j - 1].pai.lin = i;
                tela[i][j - 1].pai.col = j;
            }
            else if (!tela[i][j - 1].vista && grid[i][j - 1])
            {
                cTemp.col = j - 1;
                cTemp.lin = i;
                gNovo = tela[i][j].g + 1;
                hNovo = calculaDistDouble (cTemp, posDest);
                fNovo = gNovo + hNovo;

                if (tela[i][j - 1].f == FLT_MAX || tela[i][j - 1].f > fNovo)
                {
                    push_cjt (listaAberta, make_par (fNovo, i, j - 1));
                    tela[i][j - 1].f = fNovo;
                    tela[i][j - 1].g = gNovo;
                    tela[i][j - 1].pai.lin = i;
                    tela[i][j - 1].pai.col = j;
                }
            }
        }
    }

    caminho = cria_pilhaP ();
    /* movimentar heroi */
    if (achou)
    {
        i = posDest.lin;
        j = posDest.col;

        while (tela[i][j].pai.lin != i || tela[i][j].pai.col != j)
        {
            cTemp.lin = i;
            cTemp.col = j;
            push_pilha (caminho, cTemp);

            i = tela[i][j].pai.lin;
            j = tela[i][j].pai.col;
        }

        pop_pilha (caminho, &cTemp);
        while (pop_pilha (caminho, &cTemp))
        {
            mvwprintw (janela, posOrigem.lin, posOrigem.col, "  ");
            mvwprintw (janela, cTemp.lin, cTemp.col, "%d", h);
            wrefresh (janela);
            napms(100 / velH);

            posOrigem = cTemp;
        }
    }

    destroi_cjtP (listaAberta);
    destroi_pilhaP (caminho);

    return;
}

/* 
* fim algoritmo A* e funcoes auxiliares 
*/