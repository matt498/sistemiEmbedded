#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#define N  8



struct gestore_t {

    pthread_mutex_t mutex;

    pthread_cond_t cond_A, cond_B, cond_Q, cond_2;

    bool A_occupata;
    bool B_occupata;

    int coda_A, coda_B, coda_Q, coda_2;
    
    int scelta[N];
    
} gestore;


void init_gestore(struct gestore_t* g)
{
    
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);

    pthread_cond_init(&g->cond_A, &c_attr);
    pthread_cond_init(&g->cond_B, &c_attr);
    pthread_cond_init(&g->cond_Q, &c_attr);
    pthread_cond_init(&g->cond_2, &c_attr);
   
    g->A_occupata = false;
    g->B_occupata = false;

    // scelta = 1 è A, scelta = 2 é B9
    for (int i = 0; i < N; i++) {
        g->scelta[i] = 0;
    }
    g->coda_A = 0;
    g->coda_B = 0;
    g->coda_Q = 0;
    g->coda_2 = 0;
   
}

void priorita(struct gestore_t* g) {

    if(g->coda_Q > 0)
        pthread_cond_signal(&g->cond_Q);
    else if(g->coda_A > 0)
        pthread_cond_signal(&g->cond_A);
    else if (g->coda_B > 0)
        pthread_cond_signal(&g->cond_B);
    else if(g->coda_2 > 0)
        pthread_cond_signal(&g->cond_2);
}



void RicA(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    while (g->A_occupata) {
        g->coda_A++;
        pthread_cond_wait(&g->cond_A, &g->mutex);
        g->coda_A--;
    }
        

    g->A_occupata = true;
    printf("Il processo %d ha preso la Risorsa A\n", (int)arg);
    
    pthread_mutex_unlock(&g->mutex);

}

void RilascioA(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    // Rilascio la risorsa A
    printf("Il processo %d ha rilasciato la Risorsa A\n", (int)arg);
    g->A_occupata = false;
    priorita(g);

    pthread_mutex_unlock(&g->mutex);

}

void RicB(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    while (g->B_occupata) {
        g->coda_B++;
        pthread_cond_wait(&g->cond_B, &g->mutex);
        g->coda_B--;
    }
       

    g->B_occupata = true;
    printf("Il processo %d ha preso la Risorsa B\n", (int)arg);

    pthread_mutex_unlock(&g->mutex);

}

void RilascioB(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    // Rilascio la risorsa B
    printf("Il processo %d ha rilasciato la Risorsa B\n", (int)arg);
    g->B_occupata = false;
    priorita(g);

    pthread_mutex_unlock(&g->mutex);

}

void RicQ(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    
    while (g->A_occupata && g->B_occupata) {
        g->coda_Q++;
        pthread_cond_wait(&g->cond_Q, &g->mutex);
        g->coda_Q--;
    }
        

    //Prendo la prima disponibile

    if (!g->A_occupata) {
        printf("Il processo %d ha preso la Risorsa A\n", (int)arg);
        g->A_occupata = true;
        g->scelta[(int)arg] = 1;
    }
    else {
        printf("Il processo %d ha preso la Risorsa B\n", (int)arg);
        g->B_occupata = true;
        g->scelta[(int)arg] = 2;
    }

    pthread_mutex_unlock(&g->mutex);

}



void RilascioQ(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    if (g->scelta[(int)arg] == 1) {
        printf("Il processo %d ha rilasciato la Risorsa A\n", (int)arg);
        g->A_occupata = false;
    }
    else if(g->scelta[(int)arg] == 2){
        printf("Il processo %d ha rilasciato la Risorsa B\n", (int)arg);
        g->B_occupata = false;     
    }
    // resetto la variabile scelta
    g->scelta[(int)arg] = 0;

    priorita(g);


    pthread_mutex_unlock(&g->mutex);

}


void Ric2(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    printf("Il processo %d e' iniziato\n", (int)arg);
    
    while (g->A_occupata || g->B_occupata) {
        g->coda_2++;
        printf("Il processo %d e' bloccato\n", (int)arg);
        pthread_cond_wait(&g->cond_2, &g->mutex);
        g->coda_2--;

    }
       
    g->A_occupata = true;
    g->B_occupata = true;
    printf("Il processo %d ha preso entrambe le risorse\n", (int)arg);

    

    pthread_mutex_unlock(&g->mutex);

}

void Rilascio2(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    // Rilascio le risorse
    printf("Il processo %d ha rilasciato le risorse\n", (int)arg);
    g->A_occupata = false;
    g->B_occupata = false;
    priorita(g);


    pthread_mutex_unlock(&g->mutex);

}


/* Thread */



void* processo_1(void* arg)
{
   
    for (;;) {

        RicA(&gestore, arg);
        sleep(1);
        RilascioA(&gestore, arg); 
        sleep(10);
        
    }
    return 0;
}

void* processo_2(void* arg)
{

    for (;;) {

        RicB(&gestore, arg);
        sleep(1);
        RilascioB(&gestore, arg);
        sleep(10);

    }
    return 0;
}

void* processo_3(void* arg)
{

    for (;;) {

        RicQ(&gestore, arg);
        sleep(1);
        RilascioQ(&gestore, arg);
        sleep(15);

    }
    return 0;
}

void* processo_4(void* arg)
{

    for (;;) {

        Ric2(&gestore, arg);
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
    pthread_create(&p, &a, processo_1, (void*)(int)1);
    //pthread_create(&p, &a, processo_2, (void*)(int)2);
    pthread_create(&p, &a, processo_2, (void*)(int)3);
    pthread_create(&p, &a, processo_3, (void*)(int)4);
    pthread_create(&p, &a, processo_3, (void*)(int)5);
    //pthread_create(&p, &a, processo_4, (void*)(int)6);
    pthread_create(&p, &a, processo_4, (void*)(int)7);
    
  

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(30);

    return 0;
}
