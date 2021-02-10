#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

struct gestore_t {
    pthread_mutex_t mutex;

    pthread_cond_t priv_A, priv_B, priv_reset;
    int c_A, c_B, c_reset; // conta instanze attive
    int b_A, b_B, b_reset; // conta bloccati

    
} gestore;


void init_gestore(struct gestore_t* g)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);
    pthread_cond_init(&g->priv_A, &c_attr);
    pthread_cond_init(&g->priv_B, &c_attr);
    pthread_cond_init(&g->priv_reset, &c_attr);
    

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);

    g->c_A = 0;
    g->c_B = 0;
    g->c_reset = 0;
    g->b_A = 0;
    g->b_B = 0;
    g->b_reset = 0;

  
}

void StartProcA(struct gestore_t* g)
{

    pthread_mutex_lock(&g->mutex);
    while ((g->c_reset || g->b_reset) || g->c_A>0) {

        g->b_A++;
        pthread_cond_wait(&g->priv_A, &g->mutex);
        g->b_A--;

    }

    g->c_A++;

    pthread_mutex_unlock(&g->mutex);

}

void EndProcA(struct gestore_t* g)
{
    pthread_mutex_lock(&g->mutex);

    g->c_A--;
    if ((g->b_reset && g->c_A==0) && g->c_B==0)
        pthread_cond_signal(&g->priv_reset);

    pthread_mutex_unlock(&g->mutex);
}

void StartProcB(struct gestore_t* g)
{
    pthread_mutex_lock(&g->mutex);
    while ((g->c_reset || g->b_reset) || g->c_B>0) {

        g->b_B++;
        pthread_cond_wait(&g->priv_B, &g->mutex);
        g->b_B--;

    }

    g->c_B++;

    pthread_mutex_unlock(&g->mutex);
}

void EndProcB(struct gestore_t* g)
{
    pthread_mutex_lock(&g->mutex);

    g->c_B--;
    if ((g->b_reset && g->c_A==0) && g->c_B==0)
        pthread_cond_signal(&g->priv_reset);

    pthread_mutex_unlock(&g->mutex);
}

void StartReset(struct gestore_t* g)
{

    pthread_mutex_lock(&g->mutex);
    while (g->c_A>0 || g->c_B>0) {

        g->b_reset++;
        pthread_cond_wait(&g->priv_reset, &g->mutex);
        g->b_reset--;

    }

    g->c_reset++;

    pthread_mutex_unlock(&g->mutex);

}

void EndReset(struct gestore_t* g)
{
    pthread_mutex_lock(&g->mutex);

    g->c_reset--;

    if (g->b_A>0)
        pthread_cond_signal(&g->priv_A);

    if (g->b_B>0)
        pthread_cond_signal(&g->priv_B);

    pthread_mutex_unlock(&g->mutex);
}



void pausetta(void)
{
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = (rand() % 10 + 1) * 1000000;
    nanosleep(&t, NULL);
}




/* le funzioni della risorsa R fittizia */

#define BUSY 1000000
#define CYCLE 50

void myprint(char* s)
{
    int i, j;
    fprintf(stderr, "[");
    for (j = 0; j < CYCLE; j++) {
        fprintf(stderr, s);
        for (i = 0; i < BUSY; i++);
    }
    fprintf(stderr, "]");
}
void ProcA(void)
{
    myprint("-");
}

void ProcB(void)
{
    myprint("+");
}

void Reset(void)
{
    myprint(".");
}


void* PA(void* arg)
{
    for (;;) {
        fprintf(stderr, "A");
        StartProcA(&gestore);
        ProcA();
        EndProcA(&gestore);
        fprintf(stderr, "a");
    }
    return 0;
}

void* PB(void* arg)
{
    for (;;) {
        fprintf(stderr, "B");
        StartProcB(&gestore);
        ProcB();
        EndProcB(&gestore);
        fprintf(stderr, "b");
    }
    return 0;
}

void* PR(void* arg)
{
    for (;;) {
        fprintf(stderr, "R");
        StartReset(&gestore);
        Reset();
        EndReset(&gestore);
        fprintf(stderr, "r");
        pausetta();
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

    /* inizializzo i numeri casuali, usati nella funzione pausetta */
    srand(555);

    pthread_attr_init(&a);

    /* non ho voglia di scrivere 10000 volte join! */
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, PA, NULL);
    pthread_create(&p, &a, PA, NULL);
    pthread_create(&p, &a, PA, NULL);
    pthread_create(&p, &a, PB, NULL);
    pthread_create(&p, &a, PB, NULL);
    pthread_create(&p, &a, PB, NULL);
    pthread_create(&p, &a, PR, NULL);
  

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(5);

    return 0;
}
