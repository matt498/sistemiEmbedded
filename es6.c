#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define DELAY 100000000
#define N 5

void DELAYS(void) {
    int i;
    for (i = 0; i < DELAY; i++);
}

struct gestore_t {

    pthread_mutex_t mutex;

    pthread_cond_t c[N];

    bool forchetta_usata[N];
    
   
} gestore;


void init_gestore(struct gestore_t* g)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);

    for(int i=0;i<N;i++)
        pthread_cond_init(&g->c[i], &c_attr);
   

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);

    for (int i = 0; i < N; i++)
        g->forchetta_usata[i] = false;
    
}


void* filosofo(void* arg)
{
    int f1, f2;
    f1 = (int)arg;
    f2 = (int)arg + 1;

    if ((int)arg == (N - 1))
        f2 = 0;


    for (;;) {
        
        pthread_mutex_lock(&gestore.mutex);


        //PENSA
        printf("Il filosofo %d pensa\n", (int)arg);
        //printf("%d\n", gestore.forchetta_usata[f1]);
        //DELAYS();
        sleep(1);

        while (gestore.forchetta_usata[f1] || gestore.forchetta_usata[f2]) {

            pthread_cond_wait(&gestore.c[f1], &gestore.mutex);
            pthread_cond_wait(&gestore.c[f2], &gestore.mutex);

        }
        
        // Ha entrambe le forchette quindi può mangiare

        gestore.forchetta_usata[f1] = true;
        gestore.forchetta_usata[f2] = true;
        printf("Il filosofo %d ha preso la forchetta %d e %d\n", (int)arg, f1, f2);

        printf("Il filosofo %d mangia\n", (int)arg);
        //DELAYS();
        sleep(1);

        // Rilascia le forchette

        gestore.forchetta_usata[f1] = false;
        gestore.forchetta_usata[f2] = false;
        printf("Il filosofo %d ha lasciato la forchetta %d e %d\n", (int)arg, f1, f2);

        pthread_cond_signal(&gestore.c[f1]);
        pthread_cond_signal(&gestore.c[f2]);


        pthread_mutex_unlock(&gestore.mutex);
        sleep(2);
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

    for (int i = 0; i < N; i++) {
        pthread_create(&p, &a, filosofo, (void*)i);
        printf("Thread filosofo %d creato\n", i);
    }
        
   

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(20);

    return 0;
}
