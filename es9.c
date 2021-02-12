#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

typedef int T;
#define N  5
#define LETTURA   0
#define SCRITTURA 1 


struct gestore_t {
    pthread_mutex_t mutex;

    pthread_cond_t send[N];
    pthread_cond_t receive;
    
    T messaggio[N];
    int inviato[N]; // vettore che segna se il processo ha inviato nel buffer
    int m;

    // LETTURA stato = 0 , SCRITTURA stato= 1
    int stato; 

} gestore;


void init_gestore(struct gestore_t* g)
{
    int i;

    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);

    for(i=0; i<N ; i++)
        pthread_cond_init(&g->send[i], &c_attr);
    
    pthread_cond_init(&g->receive, &c_attr);
   
    

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);
    
    // inizializzo il messaggio con valori non possibili
    for (i = 0; i < N; i++) {
        g->messaggio[i] = 9;
        g->inviato[i] = 0; // metto che nessun messaggio è stato inviato
    }
     
    // variabile condivisa utilizzata dai processi per messaggio
    g->m = 9;
    g->stato = SCRITTURA;
}




void StartSend(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    while(g->stato == LETTURA && g->inviato[(int) arg] == 1)
        pthread_cond_wait(&g->send[(int)arg], &g->mutex);

    printf("Il processo %d sta per inviare\n", (int)arg);
    
    pthread_mutex_unlock(&g->mutex);

}

void Send(struct gestore_t* g, void* arg) {

    pthread_mutex_lock(&g->mutex);

    printf("Il processo %d invia il messaggio\n", (int)arg);

    g->m = (int)arg;
    g->messaggio[(int)arg] = g->m;
    // segno che il processo arg ha inviato il messaggio
    g->inviato[(int)arg] = 1;
   

    pthread_mutex_unlock(&g->mutex);

}

void EndSend(struct gestore_t* g, void* arg)
{
    pthread_mutex_lock(&g->mutex);

    int messaggio_completo = 1;

    for (int i = 0; i < N; i++) {
        if (g->inviato[i] == 0)
            messaggio_completo = 0;
    }

    if (messaggio_completo) {
        g->stato = LETTURA;
        printf("Tutti i processi hanno inviato quindi passo alla LETTURA\n");
        // Sveglio il lettore
        pthread_cond_signal(&g->receive);
    }
       

    pthread_mutex_unlock(&g->mutex);
}

void StartReceive(struct gestore_t* g)
{

    pthread_mutex_lock(&g->mutex);

    while (g->stato == SCRITTURA)
        pthread_cond_wait(&g->receive, &g->mutex);

    printf("Il processo Receive sta per ricevere i messaggi\n");

    pthread_mutex_unlock(&g->mutex);

}

void Receive(struct gestore_t* g) {

    pthread_mutex_lock(&g->mutex);

    printf("Il processo ha ricevuto i messaggi\n");


   // scrivo il messaggio

    for (int i = 0; i < N; i++) {
        printf("%d  ", g->messaggio[i]);
    }
    printf("\n");

    pthread_mutex_unlock(&g->mutex);

}

void EndReceive(struct gestore_t* g)
{
    pthread_mutex_lock(&g->mutex);

    // resetto messaggio e vettore inviato
    for (int i = 0; i < N; i++) {
        g->messaggio[i] = 9;
        g->inviato[i] = 0; 
    }
    g->m = 9;

    printf("Passo allora stato SCRITTURA\n");
    // cambio lo stato e sveglio i processi send
    g->stato = SCRITTURA;

    pthread_cond_signal(&g->send[0]);
    pthread_cond_signal(&g->send[1]);
    pthread_cond_signal(&g->send[2]);
    pthread_cond_signal(&g->send[3]);
    pthread_cond_signal(&g->send[4]);

    pthread_mutex_unlock(&g->mutex);
}

/* Thread */



void* processo_mittente(void* arg)
{
   
    for (;;) {

        StartSend(&gestore, arg);
        sleep(1);
        Send(&gestore, arg);
        sleep(1);
        EndSend(&gestore, arg);
        sleep(1);
        
    }
    return 0;
}

void* processo_R(void* arg)
{

    for (;;) {

        StartReceive(&gestore);
        sleep(1);
        Receive(&gestore);
        sleep(1);
        EndReceive(&gestore);
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

    pthread_create(&p, &a, processo_mittente, (void*)(int)0);
    pthread_create(&p, &a, processo_mittente, (void*)(int)1);
    pthread_create(&p, &a, processo_mittente, (void*)(int)2);
    pthread_create(&p, &a, processo_mittente, (void*)(int)3);
    pthread_create(&p, &a, processo_mittente, (void*)(int)4);
    pthread_create(&p, &a, processo_R, NULL);
  

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(30);

    return 0;
}
