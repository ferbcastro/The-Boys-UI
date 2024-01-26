#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include "movement.h"

conjuntoP *cria_cjt (int max)
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

conjuntoP *destroi_cjt (conjuntoP *c)
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

    if (!c)
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

int vazio_cjt (conjuntoP *c)
{
    if (!c)
        return 1;
    
    return (c->card == 0);
}

pilhaP *cria_pilha ()
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

pilhaP *destroi_pilha (pilhaP *pilha)
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

int main ()
{
    pilhaP *pilhaTemp;
    struct coordenadas cTemp;
 
    pilhaTemp = cria_pilha ();

    cTemp.lin = cTemp.col = 1;
    push_pilha (pilhaTemp, cTemp);
    cTemp.lin = cTemp.col = 2;
    push_pilha (pilhaTemp, cTemp);
    cTemp.lin = cTemp.col = 3;
    push_pilha (pilhaTemp, cTemp);
    cTemp.lin = cTemp.col = 4;
    push_pilha (pilhaTemp, cTemp);

    int i = 3;
    while (i--)
    {
        pop_pilha (pilhaTemp, &cTemp);
        printf ("%d %d \n", cTemp.lin, cTemp.col);
    }

    destroi_pilha (pilhaTemp);

    return 0;
}