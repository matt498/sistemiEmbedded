#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#define UOMINI 0
#define DONNE 1

struct gestore_t {
    pthread_mutex_t mutex;

    pthread_cond_t cond;
    
    
    int c_uomini, c_donne; // conta istanze attive
    int b_uomini, b_donne; // conta bloccati

    int stato;
    
    int posti_occupati;

    
    
} gestore;


void init_gestore(struct gestore_t* g)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);
    pthread_cond_init(&g->cond, &c_attr);
 

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);

    g->c_uomini = 0;
    g->c_donne = 0;
    g->b_uomini = 0;
    g->b_donne = 0;
   
   

    g->stato = UOMINI;
    g->posti_occupati = 0;
  
}

void StartUomo(struct gestore_t* g, void* arg)
{
    pthread_mutex_lock(&g->mutex);

    while ((g->stato == DONNE) || (g->posti_occupati == 5)) {

        g->b_uomini++;
        pthread_cond_wait(&g->cond, &g->mutex);
        g->b_uomini--;
    }
   
    g->c_uomini++;

  
    pthread_mutex_unlock(&g->mutex);

}

void OccupaToilette(struct gestore_t* g, void* arg)
{
    pthread_mutex_lock(&g->mutex);

    g->posti_occupati++;
    printf("La persona %d ha occupato la toilette\n", (int)arg);

    pthread_mutex_unlock(&g->mutex);
}

void EndUomo(struct gestore_t* g, void* arg)
{
    pthread_mutex_lock(&g->mutex);

    g->c_uomini--;
    g->posti_occupati--;
    printf("L'uomo %d ha lasciato la toilette\n", (int)arg);

    if (g->c_uomini == 0 && g->b_donne > 0) {
        pthread_cond_broadcast(&g->cond);
        g->stato = DONNE;
    }
    else if(g->b_uomini>0)
        pthread_cond_broadcast(&g->cond);
        
   

    pthread_mutex_unlock(&g->mutex);
}

void StartDonna(struct gestore_t* g, void* arg)
{
    pthread_mutex_lock(&g->mutex);

    while ((g->stato == UOMINI) || (g->posti_occupati == 5)) {

        g->b_donne++;
        pthread_cond_wait(&g->cond, &g->mutex);
        g->b_donne--;
    }

    g->c_donne++;





    pthread_mutex_unlock(&g->mutex);

}

void EndDonna(struct gestore_t* g, void* arg)
{
    pthread_mutex_lock(&g->mutex);

    g->c_donne--;
    g->posti_occupati--;
    printf("La donna %d ha lasciato la toilette\n", (int)arg);

    if (g->c_donne == 0 && g->b_uomini > 0) {
        pthread_cond_broadcast(&g->cond);
        g->stato = UOMINI;
    }
    else if (g->b_donne > 0)
        pthread_cond_broadcast(&g->cond);


    pthread_mutex_unlock(&g->mutex);

}

void* P_Uomini(void* arg)
{
    for (;;) {

        StartUomo(&gestore,arg);
        OccupaToilette(&gestore, arg);
        EndUomo(&gestore, arg);
        sleep(1);
    }
    return 0;
}

void* P_Donne(void* arg)
{
    for (;;) {
        
        StartDonna(&gestore, arg);
        OccupaToilette(&gestore, arg);
        EndDonna(&gestore, arg);
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

    for(int i=0; i<10; i++)
        pthread_create(&p, &a, P_Uomini, (void*)i);

    for (int j = 10; j < 20; j++)
        pthread_create(&p, &a, P_Donne, (void*)j);
   

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(10);

    return 0;
}
