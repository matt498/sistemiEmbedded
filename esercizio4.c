#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#define CARTA 0
#define SASSO 1
#define FORBICE 2

char *nomi_mosse[3] = {"carta", "sasso", "forbice"};

struct manager_t
{

    pthread_mutex_t mutex;
    pthread_cond_t cond_arbitro, cond_giocata;

    int sceltaA, sceltaB;
    bool mossaA, mossaB, start;

} morracinese;

void initialize_manager(struct manager_t *m)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&m->mutex, &mattr);
    pthread_cond_init(&m->cond_arbitro, &cattr);
    pthread_cond_init(&m->cond_giocata, &cattr);

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    m->sceltaA = 4;
    m->sceltaB = 5; //4 indica nessuna mossa compiuta
    m->mossaA = m->mossaB = false;
    m->start = false;

    srand(time(NULL));
}

void start_game(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);
    printf("##START GAME##\n");
    m->start = true;

    pthread_cond_signal(&m->cond_giocata);

    while (m->mossaA == false && m->mossaB == false)
    {
        pthread_cond_wait(&m->cond_arbitro, &m->mutex);
    }

    pthread_mutex_unlock(&m->mutex);
}

void vincitore(struct manager_t *m)
{
    int vincitore;
    if(m->sceltaA == CARTA){
        if(m->sceltaB == SASSO) vincitore = 1;
        if(m->sceltaB == FORBICE) vincitore = 2;
    }
    else if(m->sceltaA == SASSO){
        if(m->sceltaB == CARTA) vincitore = 2;
        if(m->sceltaB == FORBICE) vincitore = 1;
    }
    else if(m->sceltaA == FORBICE){
        if(m->sceltaB == CARTA) vincitore = 1;
        if(m->sceltaB == SASSO) vincitore = 2;
    }
    if(vincitore == 1){
        printf("VINCITORE: A\n");
    }
    else{
        printf("VINCITORE: B\n");
    }
}

void end_game(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    vincitore(m);

    m->mossaA = m->mossaB = false;

    printf("##END GAME##\n");
    printf("Premere un tasto per rigiocare\n");
    
    char c;
    scanf("%c", &c);

    pthread_mutex_unlock(&m->mutex);
}

void inizio_giocata(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->start == false)
    {
        pthread_cond_wait(&m->cond_giocata, &m->mutex);
    }

    do
    {
        m->sceltaA = rand() % 3;
        m->sceltaB = rand() % 3;

        if(m->sceltaA == m->sceltaB){
            printf("A:[%s] vs B:[%s] -> PAREGGIO\n", nomi_mosse[m->sceltaA], nomi_mosse[m->sceltaA]);
        }

    } while (m->sceltaA == m->sceltaB);

    m->mossaA = true;
    m->mossaB = true;
    m->start = false;

    printf("A:[%s] vs B:[%s]\n", nomi_mosse[m->sceltaA], nomi_mosse[m->sceltaB]);

    pthread_cond_signal(&m->cond_arbitro);

    pthread_mutex_unlock(&m->mutex);
}

void *arbitro(void *arg)
{
    while(1){
        start_game(&morracinese);
        end_game(&morracinese);
    }
}

void *giocata(void *arg){
    while(1){
        inizio_giocata(&morracinese);
    }
}

int main() {
    pthread_attr_t attr;
    pthread_t thread;

    initialize_manager(&morracinese);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&thread, &attr, arbitro, NULL);
    pthread_create(&thread, &attr, giocata, NULL);

    pthread_attr_destroy(&attr);

    sleep(10);

    return 0;
}