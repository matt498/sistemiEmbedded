#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_PROCESSI 2

#define SALDO 200

struct manager_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    //int blocked[NUM_PROCESSI];
    //int previsione_saldo[NUM_PROCESSI];
    int saldo;
} manager;

void init_manager(struct manager_t *m)
{
    //dichiaro attributi
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    //inizializzo attributi
    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&m->mutex, &mattr);
    pthread_cond_init(&m->cond, &cattr);

    /*
    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        m->blocked[i] = 0;
        m->previsione_saldo[i] = 0;
    }
    */

    m->saldo = SALDO;
    srand(time(NULL));
}

void prelievo(struct manager_t *m, int p)
{
    pthread_mutex_lock(&m->mutex);

    while ((m->saldo - p) < 0)
    {
        pthread_cond_wait(&m->cond, &m->mutex);
    }

    m->saldo = m->saldo - p;
    printf("Saldo: %d\n", m->saldo);
    pthread_mutex_unlock(&m->mutex);
}

void deposito(struct manager_t *m, int d)
{
    pthread_mutex_lock(&m->mutex);

    m->saldo = m->saldo + d;
    pthread_cond_broadcast(&m->cond);

    pthread_mutex_unlock(&m->mutex);
}

void *P(void *arg)
{
    while (1)
    {
        int j = rand() % 10;
        if (j < 7)
        {
            prelievo(&manager, 100);
            printf("Prelevo 100\n");
            sleep(1);
        }
        else
        {
            deposito(&manager, 300);
            printf("Deposito 300\n");
            sleep(1);
        }
    }
}

int main()
{
    init_manager(&manager);

    pthread_attr_t a;
    pthread_t t;
    //int taskid[NUM_PROCESSI];

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        //taskid[i] = i;
        pthread_create(&t, &a, P, NULL);
    }

    pthread_attr_destroy(&a);
    sleep(6);

    return 0;
}
