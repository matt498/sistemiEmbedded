#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_PROCESSI 10

struct manager_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond_segn, cond_test;

    int eventoA, blocked;

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

    pthread_mutex_unlock(&m->mutex);
}

void segnalaA(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->eventoA == 1)
    {
        pthread_cond_wait(&m->cond_segn, &m->mutex);
    }

    m->eventoA = 1;
    pthread_cond_broadcast(&m->cond_test);

    pthread_mutex_unlock(&m->mutex);
}

void *P(void *arg)
{
    // testo un'esecuzione alternata
    while (1)
    {
        int i = rand() % 10;
        if (i < 3)
        {
            printf("Segnalo A\n");
            segnalaA(&manager);
        }
        else
        {
            printf("Tento di consumare\n");
            testA(&manager);
            printf("Consumo A\n");
        }
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

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        pthread_create(&t, &a, P, NULL);
    }

    pthread_attr_destroy(&a);
    sleep(30);

    return 0;
}