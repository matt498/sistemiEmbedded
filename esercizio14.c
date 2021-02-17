#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_PROCESSI 5

struct manager_t
{
    pthread_mutex_t mutex;

    pthread_cond_t cond_A[NUM_PROCESSI];
    int stato_A[NUM_PROCESSI];
    int active_A;

    pthread_cond_t cond_B;
    int turno;
    int active_B;

    pthread_cond_t cond_C;
    int active_C;

    pthread_cond_t cond_D;
    int active_D;

    pthread_cond_t cond_fine;
    int attesa_fine;
    int usciti;

    int via_libera;

} manager;

void init_manager(struct manager_t *m)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&m->mutex, &mattr);
    pthread_cond_init(&m->cond_fine, &cattr);
    pthread_cond_init(&m->cond_B, &cattr);
    pthread_cond_init(&m->cond_C, &cattr);
    pthread_cond_init(&m->cond_D, &cattr);

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        pthread_cond_init(&m->cond_A[i], &cattr);
        m->stato_A[i] = 0;
    }
    m->usciti = m->attesa_fine = m->via_libera = 0;
    m->active_A = m->active_B = m->turno = m->active_C = m->active_D = 0;

    srand(time(NULL));
}

void start_A(struct manager_t *m, int index)
{
    pthread_mutex_lock(&m->mutex);

    while (m->active_A == 1)
    {
        m->stato_A[index] = 1;
        pthread_cond_wait(&m->cond_A[index], &m->mutex);
    }
    printf("[%d] Prendo A\n", index);
    m->active_A = 1;

    pthread_mutex_unlock(&m->mutex);
}

void end_A(struct manager_t *m, int index)
{
    pthread_mutex_lock(&m->mutex);

    m->active_A = 0;
    m->stato_A[index] = 0;
    printf("[%d] Rilascio A\n", index);
    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        if (m->stato_A[i] == 1)
        {
            pthread_cond_signal(&m->cond_A[i]);
            break;
        }
    }

    pthread_mutex_unlock(&m->mutex);
}

void A(struct manager_t *m, int index)
{
    start_A(m, index);
    end_A(m, index);
}

void start_B(struct manager_t *m, int index)
{
    pthread_mutex_lock(&m->mutex);

    while (m->active_B == 1 || index != m->turno)
    {
        pthread_cond_wait(&m->cond_B, &m->mutex);
    }
    printf("[%d] Prendo B\n", index);
    m->active_B = 1;

    pthread_mutex_unlock(&m->mutex);
}

void end_B(struct manager_t *m, int index)
{
    pthread_mutex_lock(&m->mutex);
    printf("[%d] Rilascio B\n", index);
    m->active_B = 0;
    m->turno = (m->turno + 1) % NUM_PROCESSI;
    pthread_cond_broadcast(&m->cond_B);

    pthread_mutex_unlock(&m->mutex);
}

void B(struct manager_t *m, int index)
{
    start_B(m, index);
    end_B(m, index);
}

void start_C(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->active_D > 0)
    {
        pthread_cond_wait(&m->cond_C, &m->mutex);
    }
    printf("Prendo C\n");
    m->active_C++;

    pthread_mutex_unlock(&m->mutex);
}

void end_C(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->active_C--;
    printf("Rilascio C\n");
    if (m->active_C == 0)
    {
        pthread_cond_broadcast(&m->cond_D);
    }

    pthread_mutex_unlock(&m->mutex);
}

void C(struct manager_t *m)
{
    start_C(m);
    end_C(m);
}

void start_D(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->active_C > 0)
    {
        pthread_cond_wait(&m->cond_D, &m->mutex);
    }
    printf("Prendo D\n");
    m->active_D++;

    pthread_mutex_unlock(&m->mutex);
}

void end_D(struct manager_t *m)
{
    pthread_mutex_lock(&m->mutex);

    m->active_D--;
    printf("Rilascio D\n");
    if (m->active_D == 0)
    {
        pthread_cond_broadcast(&m->cond_C);
    }

    pthread_mutex_unlock(&m->mutex);
}

void D(struct manager_t *m)
{
    start_D(m);
    end_D(m);
}

void fine_ciclo(struct manager_t *m, int index)
{
    pthread_mutex_lock(&m->mutex);

    if (m->attesa_fine == NUM_PROCESSI-1)
    {
        m->via_libera = 1;
        pthread_cond_broadcast(&m->cond_fine);
        m->attesa_fine = 0;
    }

    while (!m->via_libera)
    {
        m->attesa_fine++;
        pthread_cond_wait(&m->cond_fine, &m->mutex);
    }

    m->usciti++;

    if (m->usciti == NUM_PROCESSI)
    {
        m->usciti = 0;
        m->via_libera = 0;
    }


    pthread_mutex_unlock(&m->mutex);
}

void *processo(void *arg)
{
    while (1)
    {
        //
        A(&manager, *(int *)arg);
        //
        B(&manager, *(int *)arg);
        //
        int j = rand() % 10;
        if (j < 5)
        {
            C(&manager);
        }
        else
        {
            D(&manager);
        }
        //
        fine_ciclo(&manager, *(int *)arg);
        sleep(1);
    }
}

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
        taskid[i] = i;
        pthread_create(&t, &a, processo, (void *)(&taskid[i]));
    }

    pthread_attr_destroy(&a);
    sleep(30);

    return 0;
}