#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

struct gestore_t {

    pthread_mutex_t mutex;

    pthread_cond_t priv;
    int num_0, num_1;
   
} urna;


void init_gestore(struct gestore_t* g)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);
    pthread_cond_init(&g->priv, &c_attr);
    

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);

    g->num_0 = 0;
    g->num_1 = 0;
  
    srand((unsigned int)time(0));
}

void vota(struct gestore_t* g, int v)
{

    pthread_mutex_lock(&g->mutex);
    
    if (v == 1)
        g->num_1++;
    else
        g->num_0++;


    pthread_cond_broadcast(&g->priv);


    pthread_mutex_unlock(&g->mutex);

    

}

int risultato(struct gestore_t* g)
{
    pthread_mutex_lock(&g->mutex);

    while ((g->num_0 + g->num_1) < 3) {
        pthread_cond_wait(&g->priv, &g->mutex);
    }
    
    int ris;

    if (g->num_0 > g->num_1)
        ris = 0;
    else
        ris = 1;

    pthread_mutex_unlock(&g->mutex);

    return ris;
}




/* le funzioni della risorsa R fittizia */



void *thread(void* arg)
{
    int voto = rand() % 2;
    vota(&urna, voto);
    printf("Il thread %d ha votato %d\n", (int)arg, voto);
    sleep(1);
    if (voto == risultato(&urna)) printf("Thread %d : Ho vinto!\n",(int) arg);
    else printf("Thread %d : Ho perso!\n",(int)arg);
    //pthread_exit(0);
    return 0;
}

/* ------------------------------------------------------------------------ */

int main(int argc, char** argv)
{
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il sistema */
    init_gestore(&urna);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, thread, (void*)(int)1);
    pthread_create(&p, &a, thread, (void*)(int)2);
    pthread_create(&p, &a, thread, (void*)(int)3);
    pthread_create(&p, &a, thread, (void*)(int)4);
    pthread_create(&p, &a, thread, (void*)(int)5);

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(10);

    return 0;
}
