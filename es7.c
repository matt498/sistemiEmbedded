#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>


struct gestore_t {

    pthread_mutex_t mutex;

    pthread_cond_t cond_barbiere, cond_divano, cond_cassiere;
    
    int clienti_sul_divano;
    int clienti_su_poltrone;
    bool cassiere_occupato;
    
} gestore;


void init_gestore(struct gestore_t* g)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);
    pthread_cond_init(&g->cond_barbiere, &c_attr);
    pthread_cond_init(&g->cond_divano, &c_attr);
    pthread_cond_init(&g->cond_cassiere, &c_attr);
    

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);


    
    g->clienti_sul_divano = 0;
    g->clienti_su_poltrone = 0;
    g->cassiere_occupato = false;

  
}



void entra(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    printf("Il cliente %d entra\n", (int)arg);

    // controllo se c'è posto sul divano
    while (g->clienti_sul_divano >= 4) {

        pthread_cond_wait(&g->cond_divano, &g->mutex);
       
    }
    
    // Il cliente si siede sul divano

    printf("Il cliente %d si siede sul divano\n", (int)arg);
    g->clienti_sul_divano++;

    pthread_mutex_unlock(&g->mutex);

}

void divano(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    while (g->clienti_su_poltrone >= 3 ) {

        
        pthread_cond_wait(&g->cond_barbiere, &g->mutex);
        
    }

    // tolgo un cliente dal divano 

    printf("Il cliente %d si alza dal divano\n", (int)arg);

    g->clienti_sul_divano--;

    // Viene occupata una poltrona

    g->clienti_su_poltrone++;
        
    pthread_cond_signal(&g->cond_divano);

    pthread_mutex_unlock(&g->mutex);

}

void taglia_barba(struct gestore_t* g, void* arg)
{
    pthread_mutex_lock(&g->mutex);

    // Taglio di capelli

    printf("Il cliente %d inizia il taglio con il barbiere\n", (int)arg);

    //Una poltrona si libera e sveglio i clienti che aspettano sul divano

    g->clienti_su_poltrone--;

    pthread_cond_signal(&g->cond_barbiere);


    pthread_mutex_unlock(&g->mutex);

}

void paga(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    // Mi metto in coda per il pagamento

    while (g->cassiere_occupato) {

        pthread_cond_wait(&g->cond_cassiere, &g->mutex);

    }

    // Il cassiere è libero e il cliente se ne appropria
    g->cassiere_occupato = true;


    // Effettuo il pagamento

    printf("Il cliente %d paga il conto\n", (int)arg);


    pthread_mutex_unlock(&g->mutex);

}

void esci(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    // Libero il cassiere e sveglio tutti i clienti in coda

    g->cassiere_occupato = false;

    printf("Il cliente %d esce dal negozio\n", (int)arg);
    
    pthread_cond_signal(&g->cond_cassiere);

    pthread_mutex_unlock(&g->mutex);

}



void* cliente(void* arg)
{
    for (;;) {

        entra(&gestore, arg);
        divano(&gestore, arg);
        taglia_barba(&gestore, arg);
        paga(&gestore, arg);
        esci(&gestore, arg);
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

    for(int i=0; i<10; i++)
        pthread_create(&p, &a, cliente, (void*)(int)i);
    
    

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(60);

    return 0;
}
