/*
-M processi mittenti
-R processi riceventi
-N buste
-1 gestore G

mittente:
send(messaggio)
-> 1. richiede al gestore G una busta vuota
-> 2. inserisce nella busta il messaggio
-> 3. accodamento della busta nella mailbox

ricevente:
messaggio = receive()
-> 1. estrazione busta dalla mailbox
-> 2. estrazione del messaggio dalla busta
-> 3. rilascio busta vuota al gestore

*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// numero buste della mailbox
#define N 3

// scrittori
#define M 8

// lettori
#define R 5

typedef int T;

typedef struct
{
    T messaggio;
} busta_t;

typedef struct
{
    busta_t coda[N];
    int libere, primo, ultimo;
    int via_libera[N];
} mailbox_t;

struct manager_t
{

    pthread_mutex_t mutex;
    pthread_cond_t cond_M, cond_R;

    mailbox_t mailbox;

} manager;

void initialize_manager(struct manager_t *m)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&m->mutex, &mattr);
    pthread_cond_init(&m->cond_M, &cattr);
    pthread_cond_init(&m->cond_R, &cattr);

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    m->mailbox.libere = N;
    m->mailbox.primo = 0;
    m->mailbox.ultimo = 0;

    for(int i=0; i<N; i++){
        m->mailbox.via_libera[i] = 0;
    }
}

void richiesta(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->mailbox.libere == 0)
    {
        pthread_cond_wait(&m->cond_M, &m->mutex);
    }

    m->mailbox.libere--;

    pthread_mutex_unlock(&m->mutex);
}

void accodamento(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->mailbox.coda[m->mailbox.ultimo].messaggio = 1;
    
    printf("[");
    for(int i=0;i<N;i++){
        printf("%d,", m->mailbox.coda[i].messaggio);
    }
    printf("]\n");
    
    m->mailbox.via_libera[m->mailbox.ultimo] = 1;
    m->mailbox.ultimo = ( m->mailbox.ultimo + 1 ) % N;

    pthread_cond_signal(&m->cond_R);

    pthread_mutex_unlock(&m->mutex);
}

void estrazione(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while(m->mailbox.libere == N && m->mailbox.via_libera[m->mailbox.ultimo] == 0){
        pthread_cond_wait(&m->cond_R, &m->mutex);
    }

    m->mailbox.coda[m->mailbox.primo].messaggio = 0;

    printf("[");
    for(int i=0;i<N;i++){
        printf("%d,", m->mailbox.coda[i].messaggio);
    }
    printf("]\n");

    m->mailbox.via_libera[m->mailbox.primo] = 0;
    m->mailbox.primo = ( m->mailbox.primo + 1 ) % N;

    pthread_mutex_unlock(&m->mutex);
}

void rilascio(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->mailbox.libere++;

    pthread_cond_signal(&m->cond_M);    

    pthread_mutex_unlock(&m->mutex);
}

void *mittente(void *arg){
    richiesta(&manager);
    accodamento(&manager);
}

void *ricevente(void *arg){
    estrazione(&manager);
    rilascio(&manager);
}

/*
void osserva(struct manager_t * m){
    printf()
}
void *observer(void *arg){
    osserva(&manager);
}
*/
int main() {
    pthread_attr_t attr;
    pthread_t thread;

    initialize_manager(&manager);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for(int i=0;i<M;i++) pthread_create(&thread, &attr, mittente, NULL);
    for(int i=0;i<R;i++) pthread_create(&thread, &attr, ricevente, NULL);

    pthread_attr_destroy(&attr);

    sleep(10);

    return 0;
}
