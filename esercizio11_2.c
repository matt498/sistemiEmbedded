#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sched.h>

#define NUM_PROCESSI 10

struct manager_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond_segn, cond_test;
    pthread_cond_t prio_cond[NUM_PROCESSI];

    int prio_bloccati[NUM_PROCESSI];

    int eventoA, blocked;
    //int pr_bloccati;

} manager;

void init_manager(struct manager_t *m)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&m->mutex, &mattr);
    pthread_cond_init(&m->cond_test, &cattr);
    pthread_cond_init(&m->cond_segn, &cattr);

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        pthread_cond_init(&m->prio_cond[i], &cattr);
        m->prio_bloccati[i] = 0;
    }

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    m->eventoA = m->blocked = 0;

    srand(234);
}

void testA(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->eventoA == 0)
    {
        m->blocked++;
        pthread_cond_wait(&m->cond_test, &m->mutex);
        m->blocked--;
    }

    if (m->blocked == 0) //solo l'ultimo di un gruppo bloccato resetta l'evento IMPORTANTE altrimenti deadlock
        m->eventoA = 0;

    pthread_cond_broadcast(&m->cond_segn);

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        if (m->prio_bloccati[i] == 1){
            printf("Sveglio quello con prio: %d\n", i);
            pthread_cond_signal(&m->prio_cond[i]);
        }
    }

    pthread_mutex_unlock(&m->mutex);
}

void segnalaA(struct manager_t *m, int prio)
{
    pthread_mutex_lock(&m->mutex);

    while (m->eventoA == 1)
    {
        m->prio_bloccati[prio] = 1;
        printf("Evento segnalato, mi blocco\n");
        pthread_cond_wait(&m->cond_segn, &m->mutex);
    }

    if (m->prio_bloccati[prio] == 1)
    {
        while (m->prio_bloccati[prio] == 1)
        {
            printf("Aspetto il mio turno, mia priorità: %d\n", prio);
            pthread_cond_wait(&m->prio_cond[prio], &m->mutex);
        }
        m->prio_bloccati[prio] = 0;
    }

    m->eventoA = 1;
    pthread_cond_broadcast(&m->cond_test);

    pthread_mutex_unlock(&m->mutex);
}

void *PS(void *arg)
{
    // testo un'esecuzione alternata
    while (1)
    {
        printf("[prio: %d] Segnalo A\n", *(int *)arg);
        segnalaA(&manager, *(int *)arg);
        sleep(1);
    }
}

void *PT(void *arg)
{
    while (1)
    {
        printf("Tento di consumare\n");
        testA(&manager);
        printf("Consumo A\n");
        sleep(1);
    }
}

/*
*
*   Per compilare usare: docker run -it -v$(pwd):/home/conan/project --rm conanio/gcc7 /bin/bash
*

*/

int main()
{
    init_manager(&manager);

    pthread_attr_t a;
    pthread_t t;
    int taskid[NUM_PROCESSI];
    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        int j = rand() % 10;
        taskid[i] = i;
        if (j > 4)
        {
            pthread_create(&t, &a, PS, (void *)&taskid[i]);
        }
        else
            pthread_create(&t, &a, PT, NULL);
    }

    pthread_attr_destroy(&a);
    sleep(30);

    return 0;
}