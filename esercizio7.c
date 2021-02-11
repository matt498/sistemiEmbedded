#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

//N: numero barbieri e poltrone
#define N 3

//D: posti divana in sala d'aspetto
#define D 4

#define SHAVING_ITERATIONS 200
#define PAYING_ITERATIONS 100
#define NUM_CLIENTI 10

struct negozio_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond_taglio, cond_pagamento, cond_attesa, synchro;

    int poltrone, posti_divano, pagamento;
} negozio;

void init_negozio(struct negozio_t *n)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&n->mutex, &mattr);
    pthread_cond_init(&n->cond_taglio, &cattr);
    pthread_cond_init(&n->cond_pagamento, &cattr);
    pthread_cond_init(&n->cond_attesa, &cattr);
    pthread_cond_init(&n->synchro, &cattr);

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    n->poltrone = 0;
    n->posti_divano = 0;
    n->pagamento = 0;
}

void entro_in_sala(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    n->posti_divano++;

    //fino a che c'è posto occupo posti, dopo l'occupazione dei 4 posti devo asepttare fuori
    while (n->posti_divano > D)
    {
        pthread_cond_wait(&n->cond_attesa, &n->mutex);
    }

    printf("Entro in sala\n");

    pthread_mutex_unlock(&n->mutex);
}

void taglio(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    //mi blocco se poltrone occupate
    n->poltrone++;
    //chiamo barbiere
    pthread_cond_signal(&n->synchro);

    while (n->poltrone > N)
    {
        pthread_cond_wait(&n->cond_taglio, &n->mutex);
    }

    printf("Voglio un taglio\n");

    pthread_mutex_unlock(&n->mutex);
}

void pagare()
{
    printf("Voglio pagare\n");
    for (int i = 0; i < PAYING_ITERATIONS; i++)
        ;
}

void pagamento(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    while (n->pagamento == 0)
    {
        pthread_cond_wait(&n->cond_pagamento, &n->mutex);
    }
    pagare();
    n->pagamento = 0;

    pthread_mutex_unlock(&n->mutex);
}

void simula_taglio()
{
    printf("Taglio...\n");
    for (int i = 0; i < SHAVING_ITERATIONS; i++)
        ;
}

void accomoda_persona(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    while(n->poltrone == 0){
        pthread_cond_wait(&n->synchro, &n->mutex);
    }

    printf("Accomodo persona\n");
    n->poltrone--;
    pthread_cond_signal(&n->cond_taglio);
    n->posti_divano--;
    pthread_cond_signal(&n->cond_attesa);

    pthread_mutex_unlock(&n->mutex);
}

void accetta_pagamento(struct negozio_t *n)
{
    pthread_mutex_lock(&n->mutex);

    printf("Accetto pagaento\n");
    n->pagamento = 1;
    pthread_cond_signal(&n->cond_pagamento);
    
    pthread_mutex_unlock(&n->mutex);
}

void *barbiere(void *arg)
{
    while (1)
    {

        accomoda_persona(&negozio);
        simula_taglio();
        sleep(2);
        accetta_pagamento(&negozio);
    }
}

void *cliente(void *arg)
{
    entro_in_sala(&negozio);
    taglio(&negozio);
    pagamento(&negozio);
}

int main(void)
{
    pthread_attr_t a;
    pthread_t p;

    //inizializzo
    init_negozio(&negozio);

    pthread_attr_init(&a);

    //per non scrivere le join:
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, barbiere, (void*)"A"); //creazione thread barbiere
    printf("[MAIN] Creato thread barbiere\n");

    long i;
    for (i = 0; i < NUM_CLIENTI; i++) //creazione di NUM_CLIENTI thread clienti
    {
        pthread_create(&p, &a, cliente, (void*)i);
        printf("[MAIN] Creato thread cliente%ld\n", i);
    }

    pthread_attr_destroy(&a);

    //aspetto prima di terminare
    sleep(30);

    return 0;
}