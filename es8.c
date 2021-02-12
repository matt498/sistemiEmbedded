#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

struct gestore_t {
    pthread_mutex_t mutex;

    pthread_cond_t priv[4];
    int risorsa; // conta instanze attive
    int b[4]; // conta bloccati
    int risorsa_occupata;
    int timer[4];
    int contatori[4];
    
} gestore;


void init_gestore(struct gestore_t* g)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);
    pthread_cond_init(&g->priv[0], &c_attr);
    pthread_cond_init(&g->priv[1], &c_attr);
    pthread_cond_init(&g->priv[2], &c_attr);
    pthread_cond_init(&g->priv[3], &c_attr);

    
    

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);

    g->risorsa = 0;
    g->b[0] = 0;
    g->b[1] = 0;
    g->b[2] = 0;
    g->b[3] = 0;
    g->risorsa_occupata = 0;
    g->timer[0] = 2;
    g->timer[1] = 3;
    g->timer[2] = 4;
    g->timer[3] = 5;

    g->contatori[0] = 0;
    g->contatori[1] = 0;
    g->contatori[2] = 0;
    g->contatori[3] = 0;
  
}

void conta_tick(struct gestore_t* g, void* arg) {

    // aggiorno i contatori se il processo è bloccato

    if (g->b[0])
        g->contatori[(int)arg]++;

    if (g->b[1])
        g->contatori[(int)arg]++;

    if (g->b[2])
        g->contatori[(int)arg]++;

    if (g->b[3])
        g->contatori[(int)arg]++;

    printf("Contatore 0: %d\n", g->contatori[0]);
    printf("Contatore 1: %d\n", g->contatori[1]);
    printf("Contatore 2: %d\n", g->contatori[2]);
    printf("Contatore 3: %d\n", g->contatori[3]);

    // Sveglio un processo se è stato dormiente per più del suo timer

    if (g->b[0] && (g->contatori[0] >= g->timer[0])) {
        pthread_cond_signal(&g->priv[0]);
        g->contatori[0] = 0;
        printf("Processo 0 risvegliato per timer!\n");
    }


    if (g->b[1] && (g->contatori[1] >= g->timer[1])) {
        pthread_cond_signal(&g->priv[1]);
        g->contatori[1] = 0;
        printf("Processo 1 risvegliato per timer!\n");
    }


    if (g->b[2] && (g->contatori[2] >= g->timer[2])) {
        pthread_cond_signal(&g->priv[2]);
        g->contatori[2] = 0;
        printf("Processo 2 risvegliato per timer!\n");
    }


    if (g->b[3] && (g->contatori[3] >= g->timer[3])) {
        pthread_cond_signal(&g->priv[3]);
        g->contatori[3] = 0;
        printf("Processo 3 risvegliato per timer!\n");
    }

}


void StartProc(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    printf("Processo %d cerca di accedere\n", (int)arg);
    while (g->risorsa_occupata) {

        g->b[(int) arg]++;
        printf("Bloccato processo: %d\n", (int)arg);
        
        pthread_cond_wait(&g->priv[(int) arg], &g->mutex);

        printf("Sbloccato processo: %d\n", (int)arg);
        g->b[(int)arg]--;

    }

    printf("Processo %d ha preso la risorsa\n", (int)arg);
    g->risorsa_occupata=1;
    
    pthread_mutex_unlock(&g->mutex);

}

void EndProc(struct gestore_t* g, void* arg)
{
    pthread_mutex_lock(&g->mutex);

    // Sveglio i processi e libero la risorsa

    g->risorsa_occupata = 0;

    pthread_cond_signal(&g->priv[0]);
    pthread_cond_signal(&g->priv[1]);
    pthread_cond_signal(&g->priv[2]);
    pthread_cond_signal(&g->priv[3]);
    
        
    printf("Processo %d rilascia la risorsa\n", (int)arg);
        

    pthread_mutex_unlock(&g->mutex);
}

void Proc(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    g->risorsa = (int)arg;

    for (int i = 0; i < ((int)arg * 3 + 1); i++) {
        printf("%d - ", g->risorsa);
        conta_tick(g, arg);
    }
      
    printf("\n");

    pthread_mutex_unlock(&g->mutex);

}


/* Thread */



void* processo(void* arg)
{
   
    for (;;) {

        StartProc(&gestore, arg);
        sleep(1);
        Proc(&gestore, arg);
        sleep(1);
        EndProc(&gestore, arg);
        sleep(3);
        
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

    pthread_create(&p, &a, processo, (void*)(int)0);
    pthread_create(&p, &a, processo, (void*)(int)1);
    pthread_create(&p, &a, processo, (void*)(int)2);
    pthread_create(&p, &a, processo, (void*)(int)3);
  

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(30);

    return 0;
}
