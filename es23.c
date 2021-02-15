#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#define N  3



struct gestore_t {

    pthread_mutex_t mutex;

    pthread_cond_t cond_1, cond_2;

    bool R1_occupata;
    bool R2_occupata;

    int coda_1, coda_2;
    
    int scelta[N];
    
} gestore;


void init_gestore(struct gestore_t* g)
{
    
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);

    pthread_cond_init(&g->cond_1, &c_attr);
    pthread_cond_init(&g->cond_2, &c_attr);
   
   
    g->R1_occupata = false;
    g->R2_occupata = false;

    // scelta = 1 è R1, scelta = 2 é R2
    for (int i = 0; i < N; i++) {
        g->scelta[i] = 0;
    }
    
    g->coda_1 = 0;
    g->coda_2 = 0;
    
   
}

void priorita(struct gestore_t* g) {

    if(g->coda_2 > 0)
        pthread_cond_signal(&g->cond_2);
    else if(g->coda_1 > 0)
        pthread_cond_signal(&g->cond_1);
   
}





void Richiesta1(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    
    while (g->R1_occupata && g->R2_occupata) {
        g->coda_1++;
        pthread_cond_wait(&g->cond_1, &g->mutex);
        g->coda_1--;
    }
        

    //Prendo la prima disponibile

    if (!g->R1_occupata) {
        printf("Il processo %d ha preso la Risorsa R1\n", (int)arg);
        g->R1_occupata = true;
        g->scelta[(int) arg] = 1;
    }
    else {
        printf("Il processo %d ha preso la Risorsa R2\n", (int)arg);
        g->R2_occupata = true;
        g->scelta[(int)arg] = 2;
    }

    pthread_mutex_unlock(&g->mutex);

}



void Rilascio1(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    if (g->scelta[(int) arg] == 1) {
        printf("Il processo %d ha rilasciato la Risorsa R1\n", (int)arg);
        g->R1_occupata = false;
    }
    else if(g->scelta[(int)arg] == 2){
        printf("Il processo %d ha rilasciato la Risorsa R2\n", (int)arg);
        g->R2_occupata = false;     
    }
    // resetto la variabile scelta
    g->scelta[(int) arg] = 0;

    priorita(g);


    pthread_mutex_unlock(&g->mutex);

}


void Richiesta2(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    printf("Il processo %d e' iniziato\n", (int)arg);
    
    while (g->R1_occupata || g->R2_occupata) {
        g->coda_2++;
        printf("Il processo %d e' bloccato\n", (int)arg);
        pthread_cond_wait(&g->cond_2, &g->mutex);
        g->coda_2--;

    }
       
    g->R1_occupata = true;
    g->R2_occupata = true;
    printf("Il processo %d ha preso entrambe le risorse\n", (int)arg);


    pthread_mutex_unlock(&g->mutex);

}

void Rilascio2(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    // Rilascio le risorse
    printf("Il processo %d ha rilasciato le risorse\n", (int)arg);
    g->R1_occupata = false;
    g->R2_occupata = false;

    priorita(g);


    pthread_mutex_unlock(&g->mutex);

}


/* Thread */





void* processo_1(void* arg)
{

    for (;;) {

        Richiesta1(&gestore, arg);
        sleep(1);
        Rilascio1(&gestore, arg);
        sleep(15);

    }
    return 0;
}

void* processo_2(void* arg)
{

    for (;;) {

        Richiesta2(&gestore, arg);
        sleep(1);
        Rilascio2(&gestore, arg);
        sleep(1);

    }
    return 0;
}


/* ------------------------------------------------------------------------ */

int main(int argc, char** argv)
{
    pthread_attr_t a;
    pthread_t p;

    /* inizializzo il sistema */
    init_gestore(&gestore);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);


    //pthread_create(&p, &a, processo_1, (void*)(int)0);
    //pthread_create(&p, &a, processo_1, (void*)(int)1);
    //pthread_create(&p, &a, processo_2, (void*)(int)2);
    //pthread_create(&p, &a, processo_2, (void*)(int)3);
    pthread_create(&p, &a, processo_1, (void*)(int)0);
    pthread_create(&p, &a, processo_1, (void*)(int)1);
    pthread_create(&p, &a, processo_1, (void*)(int)2);
  
    pthread_create(&p, &a, processo_2, (void*)(int)3);
    pthread_create(&p, &a, processo_2, (void*)(int)4);
    pthread_create(&p, &a, processo_2, (void*)(int)5);
  

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(30);

    return 0;
}
