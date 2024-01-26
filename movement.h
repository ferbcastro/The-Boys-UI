/* 
* inicio declaracoes 
*/

struct coordenadas
{
    int col;
    int lin;
};

struct celula
{
    struct coordenadas pai;
    int vista;
    double f;
    double g;
};

typedef struct
{
    double first;
    struct coordenadas second;
} par;

typedef struct
{
    int max;
    int card;
    par *v;
} conjuntoP;

typedef struct elementoP
{
    struct coordenadas pos;
    struct elementoP *prox; 
} elementoP;

typedef struct
{
    elementoP *primeiro;    
} pilhaP;

/* 
* fim declaracoes 
*/

/* inicio funcoes conjunto */

conjuntoP *cria_cjtP (int max);

par make_par (double f, int lin, int col);

conjuntoP *destroi_cjtP (conjuntoP *c);

int push_cjt (conjuntoP *c, par p);

par pop_cjt (conjuntoP *c);

int vazio_cjtP (conjuntoP *c);

/* fim funcoes conjunto */

/* inicio funcoes pilha */

pilhaP *cria_pilhaP ();

int push_pilha (pilhaP *pilha, struct coordenadas posAux);

int pop_pilha (pilhaP *pilha, struct coordenadas *posAux);

pilhaP *destroi_pilhaP (pilhaP *pilha);

/* fim funcoes pilha */

/* inicio funcoes principais */

double calculaDistDouble (struct coordenadas a, struct coordenadas b);

void movimentaHeroi (struct coordenadas posDest, struct coordenadas posOrigem, WINDOW *janela, int h, int velH);

/* fim funcoes principais */