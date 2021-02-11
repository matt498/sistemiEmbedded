#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define LIBERA 0
#define OCCUPATA 1
#define NUM_PROCESSI 5
struct manager_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond_A, cond_B, cond_Q, cond_2;

    //int blocked_A, blocked_B, blocked_Q, blocked_2;
    int stato_A, stato_B;

    char sceltaQ;
} manager;

void init_manager(struct manager_t *m)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&m->mutex, &mattr);
    pthread_cond_init(&m->cond_A, &cattr);
    pthread_cond_init(&m->cond_B, &cattr);
    pthread_cond_init(&m->cond_Q, &cattr);
    pthread_cond_init(&m->cond_2, &cattr);

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    //m->blocked_A = m->blocked_B = m->blocked_Q = m->blocked_2 = 0;
    m->stato_A = m->stato_B = 0;

    srand(112);
}

void precedenza(struct manager_t *m)
{
    if (m->stato_A == LIBERA || m->stato_B == LIBERA)
        pthread_cond_signal(&m->cond_Q);
    else if (m->stato_A == LIBERA)
        pthread_cond_signal(&m->cond_A);
    else if (m->stato_B == LIBERA)
        pthread_cond_signal(&m->cond_B);
    else if (m->stato_A == LIBERA && m->stato_B == LIBERA)
        pthread_cond_signal(&m->cond_2);
}

void start_A(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->stato_A == OCCUPATA)
    {
        pthread_cond_wait(&m->cond_A, &m->mutex);
    }

    m->stato_A = OCCUPATA;

    pthread_mutex_unlock(&m->mutex);
}

void end_A(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->stato_A = LIBERA;
    precedenza(m);

    pthread_mutex_unlock(&m->mutex);
}

void start_B(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->stato_B == OCCUPATA)
    {
        pthread_cond_wait(&m->cond_B, &m->mutex);
    }

    m->stato_B = OCCUPATA;

    pthread_mutex_unlock(&m->mutex);
}

void end_B(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->stato_B = LIBERA;
    precedenza(m);

    pthread_mutex_unlock(&m->mutex);
}

void start_Q(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->stato_A == OCCUPATA && m->stato_B == OCCUPATA)
    {
        pthread_cond_wait(&m->cond_Q, &m->mutex);
    }

    if (m->stato_A == LIBERA)
    {
        m->stato_A = OCCUPATA;
        m->sceltaQ = 'A';
    }
    else
    {
        m->stato_B = OCCUPATA;
        m->sceltaQ = 'B';
    }
    pthread_mutex_unlock(&m->mutex);
}

void end_Q(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    if (m->sceltaQ == 'A')
        m->stato_A = LIBERA;
    else
        m->stato_B = LIBERA;

    precedenza(m);

    pthread_mutex_unlock(&m->mutex);
}

void start_2(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->stato_A == OCCUPATA || m->stato_B == OCCUPATA)
    {
        pthread_cond_wait(&m->cond_Q, &m->mutex);
    }

    m->stato_A = OCCUPATA;
    m->stato_B = OCCUPATA;

    pthread_mutex_unlock(&m->mutex);
}

void end_2(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->stato_A = LIBERA;
    m->stato_B = LIBERA;
    precedenza(m);

    pthread_mutex_unlock(&m->mutex);
}

void *processo(void *arg)
{
    while (1)
    {
        int i = rand() % 4;
        if (i == 0)
        {
            start_Q(&manager);
            printf("%c\n", manager.sceltaQ);
            end_Q(&manager);
            printf("libero %c\n", manager.sceltaQ);
            sleep(1);
        }
        else if (i == 1)
        {
            start_A(&manager);
            printf("A\n");
            end_A(&manager);
            printf("libero A\n");
            sleep(1);
        }
        else if (i == 2)
        {
            start_B(&manager);
            printf("B\n");
            end_B(&manager);
            printf("libero B\n");
            sleep(1);
        }
        else if (i == 3)
        {
            start_2(&manager);
            printf("AB\n");
            end_2(&manager);
            printf("libero AB\n");
            sleep(1);
        }
    }
}

int main()
{
    init_manager(&manager);

    pthread_attr_t a;
    pthread_t t;


    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        pthread_create(&t, &a, processo, NULL);
    }

    pthread_attr_destroy(&a);
    sleep(10);

    return 0;
}