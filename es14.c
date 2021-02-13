#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

typedef int msg;
#define N  10



struct gestore_t {

    pthread_mutex_t mutex;

    pthread_cond_t cond_A[N], cond_B[N], cond_C, cond_D, cond_fine;

    int c_A, c_B, c_C, c_D; // conta istanze attive
    int b_A, b_B, b_C, b_D; // conta_bloccati

    bool bloccati_A[N]; // verifica se è il processo è bloccato su A
   
    bool finito;

    int index;
    int c_finito;
    int conta_usciti;
    int eseguito[N];
    

} gestore;


void init_gestore(struct gestore_t* g)
{
    

    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&g->mutex, &m_attr);

    for (int j = 0; j < N; j++) {

        pthread_cond_init(&g->cond_A[j], &c_attr);
        pthread_cond_init(&g->cond_B[j], &c_attr);

    }
    
    pthread_cond_init(&g->cond_C, &c_attr);
    pthread_cond_init(&g->cond_D, &c_attr);
    pthread_cond_init(&g->cond_fine, &c_attr);
   

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);
    
    g->c_A = g->c_B = g->c_C = g->c_D = 0;
  
    g->b_A = g->b_B = g->b_C = g->b_D = 0;
   
    for (int i = 0; i < N; i++) {

        g->bloccati_A[i] = false;
        g->eseguito[i] = false;
        
        
    }

    g->index = 10;
    g->c_finito = 0;
    g->finito = false;
    g->conta_usciti = 0;

}




void StartA(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    while (g->c_A > 0) {

        g->b_A++;
        g->bloccati_A[(int)arg] = true;

        pthread_cond_wait(&g->cond_A[(int)arg], &g->mutex);

        g->bloccati_A[(int)arg] = false;
        g->b_A--;

    }
    
    g->c_A++;
    
    pthread_mutex_unlock(&g->mutex);

}


void ProcA(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    g->index = (int)arg;
    printf("Il processo %d (%d) scrive A\n", g->index, (int) arg);

    pthread_mutex_unlock(&g->mutex);

}


void EndA(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    g->c_A--;

    if (g->b_A > 0) {

        for (int i = 0; i < N; i++) {

            if (g->bloccati_A[i] == true) {
                pthread_cond_signal(&g->cond_A[i]);
                break;
            }

        }

    }
    pthread_mutex_unlock(&g->mutex);

}

void StartB(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    if ((int)arg != 0) {

        while (g->eseguito[(int) arg -1] == 0) {
            g->b_B++;
            printf("Il processo %d e mi blocco\n", (int)arg);
            pthread_cond_wait(&g->cond_B[(int)arg], &g->mutex);
            g->b_B--;

        }
          
    }
    g->c_B++;

    pthread_mutex_unlock(&g->mutex);

}


void ProcB(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    g->index = (int)arg;
    printf("Il processo %d (%d) scrive B\n", g->index, (int)arg);

    pthread_mutex_unlock(&g->mutex);

}


void EndB(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    g->c_B--;
    g->eseguito[(int)arg] = 1;
    pthread_cond_signal(&g->cond_B[(int) arg + 1]);
  
    pthread_mutex_unlock(&g->mutex);

}

void StartC(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    while (g->c_D > 0) {

        g->b_C++;
        pthread_cond_wait(&g->cond_C, &g->mutex);
        g->b_C--;

    }

    g->c_C++;


    pthread_mutex_unlock(&g->mutex);

}


void ProcC(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    g->index = (int)arg;
    printf("Il processo %d (%d) scrive C\n", g->index, (int)arg);

    pthread_mutex_unlock(&g->mutex);

}


void EndC(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    g->c_C--;

    // Se è l'ultimo possono partire i D

    if (g->c_C == 0) {

        pthread_cond_broadcast(&g->cond_D);

    }

    pthread_mutex_unlock(&g->mutex);

}

void StartD(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    while (g->c_C > 0) {

        g->b_D++;
        pthread_cond_wait(&g->cond_D, &g->mutex);
        g->b_D--;

    }

    g->c_D++;


    pthread_mutex_unlock(&g->mutex);

}


void ProcD(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    g->index = (int)arg;
    printf("Il processo %d (%d) scrive D\n", g->index, (int)arg);


    pthread_mutex_unlock(&g->mutex);

}


void EndD(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    g->c_D--;

    // Se è l'ultimo possono partire i C

    if (g->c_D == 0) {

        pthread_cond_broadcast(&g->cond_C);

    }


    pthread_mutex_unlock(&g->mutex);

}

void Fine_ciclo(struct gestore_t* g, void* arg)
{

    pthread_mutex_lock(&g->mutex);

    
    printf("Sono il processo %d e ho finito. Abbiamo finito in %d\n", (int)arg, g->c_finito);

    if (g->c_finito == (N-1)) {
        g->finito = true;
        printf("Tutti i processi hanno finito e ripartiamo\n");
        pthread_cond_broadcast(&g->cond_fine);
        g->c_finito = 0;

    }
      
    while(!g->finito) {

        g->c_finito++;
        pthread_cond_wait(&g->cond_fine, &g->mutex);
        
    }

    g->conta_usciti++;

    if (g->conta_usciti == N) {
        g->finito = false;
        g->conta_usciti = 0;
    }
      


    pthread_mutex_unlock(&g->mutex);

}


/* Thread */



void* processo_con_C(void* arg)
{
   
    for (;;) {

        StartA(&gestore, arg);
        ProcA(&gestore, arg);
        EndA(&gestore, arg);
        sleep(1);

        StartB(&gestore, arg);
        ProcB(&gestore, arg);
        EndB(&gestore, arg);
        sleep(1);

        StartC(&gestore, arg);
        ProcC(&gestore, arg);
        EndC(&gestore, arg);
        sleep(1);

        Fine_ciclo(&gestore, arg);

        sleep(1);
        
    }
    return 0;
}

void* processo_con_D(void* arg)
{

    for (;;) {

        StartA(&gestore, arg);
        ProcA(&gestore, arg);
        EndA(&gestore, arg);
        sleep(1);

        StartB(&gestore, arg);
        ProcB(&gestore, arg);
        EndB(&gestore, arg);
        sleep(1);

        StartD(&gestore, arg);
        ProcD(&gestore, arg);
        EndD(&gestore, arg);
        sleep(1);

        Fine_ciclo(&gestore, arg);

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

   
    pthread_create(&p, &a, processo_con_C, (void*)(int)0);
    pthread_create(&p, &a, processo_con_C, (void*)(int)1);
    pthread_create(&p, &a, processo_con_C, (void*)(int)2);
    pthread_create(&p, &a, processo_con_C, (void*)(int)3);
    pthread_create(&p, &a, processo_con_C, (void*)(int)4);

    pthread_create(&p, &a, processo_con_D, (void*)(int)5);
    pthread_create(&p, &a, processo_con_D, (void*)(int)6);
    pthread_create(&p, &a, processo_con_D, (void*)(int)7);
    pthread_create(&p, &a, processo_con_D, (void*)(int)8);
    pthread_create(&p, &a, processo_con_D, (void*)(int)9);
   
 

    pthread_attr_destroy(&a);

    /* aspetto 10 secondi prima di terminare tutti quanti */
    sleep(30);

    return 0;
}
