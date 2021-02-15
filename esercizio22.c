#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define DIM_BUFFER 5
#define NUM_MITTENTI 15

typedef int msg; //userò l'indice del processo come messaggio

struct mailbox_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond, cond_riceventi;

    msg array[DIM_BUFFER];
    int testa_array;
    int coda_array, coda1, coda2, coda3;
    int stato_di_lettura1[DIM_BUFFER];
    int stato_di_lettura2[DIM_BUFFER];
    int stato_di_lettura3[DIM_BUFFER]; //se msg i-esimo vale 2 allora potrà essere cancellato
    int spazio_occupato;

} mailbox;

void init_mailbox(struct mailbox_t *m)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&m->mutex, &mattr);
    pthread_cond_init(&m->cond, &cattr);
    pthread_cond_init(&m->cond_riceventi, &cattr);

    for (int i = 0; i < DIM_BUFFER; i++)
    {
        m->stato_di_lettura1[i] = 0;
        m->stato_di_lettura2[i] = 0;
        m->stato_di_lettura3[i] = 0;
    }

    m->spazio_occupato = m->testa_array = m->coda_array = m->coda1 = m->coda2 = m->coda3 = 0;
}

void send(struct mailbox_t *m, int index)
{
    pthread_mutex_lock(&m->mutex);

    while (m->spazio_occupato == DIM_BUFFER)
    {
        pthread_cond_wait(&m->cond, &m->mutex);
    }
    msg mess = index;
    printf("[%d] Invio in posizione %d il messaggio %d\n", index, m->testa_array, index);
    m->spazio_occupato++;
    m->array[m->testa_array] = mess;
    m->testa_array = (m->testa_array + 1) % DIM_BUFFER;
    pthread_cond_signal(&m->cond_riceventi);

    pthread_mutex_unlock(&m->mutex);
}

void receive1(struct mailbox_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->spazio_occupato == 0)
    {
        pthread_cond_wait(&m->cond_riceventi, &m->mutex);
    }

    msg mess = m->array[m->coda1];
    printf("R1: leggo in posizione %d il messaggio %d\n", m->coda_array, mess);
    if (m->stato_di_lettura1[m->coda1] == 0)
    {
        m->stato_di_lettura1[m->coda1] = 1;
        m->coda1 = (m->coda1 + 1) % DIM_BUFFER;
    }

    if (m->stato_di_lettura1[m->coda_array] == 1 && m->stato_di_lettura2[m->coda_array] == 1 && m->stato_di_lettura3[m->coda_array] == 1)
    {
        m->spazio_occupato--;
        m->stato_di_lettura1[m->coda_array] = 0;
        m->stato_di_lettura2[m->coda_array] = 0;
        m->stato_di_lettura3[m->coda_array] = 0;
        printf("R1: Cancello msg in posizione %d\n", m->coda_array);
        m->array[m->coda_array] = 0;
        m->coda_array = (m->coda_array + 1) % DIM_BUFFER;
    }

    pthread_cond_signal(&m->cond);

    pthread_mutex_unlock(&m->mutex);
}

void receive2(struct mailbox_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->spazio_occupato == 0)
    {
        pthread_cond_wait(&m->cond_riceventi, &m->mutex);
    }

    msg mess = m->array[m->coda2];
    printf("R2: leggo in posizione %d il messaggio %d\n", m->coda_array, mess);
    m->stato_di_lettura2[m->coda2] = 1;
    m->coda2 = (m->coda2 + 1) % DIM_BUFFER;

    if (m->stato_di_lettura1[m->coda_array] == 1 && m->stato_di_lettura2[m->coda_array] == 1 && m->stato_di_lettura3[m->coda_array] == 1)
    {
        m->spazio_occupato--;
        m->stato_di_lettura1[m->coda_array] = 0;
        m->stato_di_lettura2[m->coda_array] = 0;
        m->stato_di_lettura3[m->coda_array] = 0;
        printf("R2: Cancello msg in posizione %d\n", m->coda_array);
        m->array[m->coda_array] = 0;
        m->coda_array = (m->coda_array + 1) % DIM_BUFFER;
    }

    pthread_cond_signal(&m->cond);

    pthread_mutex_unlock(&m->mutex);
}

void receive3(struct mailbox_t *m)
{
    pthread_mutex_lock(&m->mutex);

    while (m->spazio_occupato == 0)
    {
        pthread_cond_wait(&m->cond_riceventi, &m->mutex);
    }

    msg mess = m->array[m->coda3];
    printf("R3: leggo in posizione %d il messaggio %d\n", m->coda_array, mess);
    m->stato_di_lettura3[m->coda3] = 1;
    m->coda3 = (m->coda3 + 1) % DIM_BUFFER;

    if (m->stato_di_lettura1[m->coda_array] == 1 && m->stato_di_lettura2[m->coda_array] == 1 && m->stato_di_lettura3[m->coda_array] == 1)
    {
        m->spazio_occupato--;
        m->stato_di_lettura1[m->coda_array] = 0;
        m->stato_di_lettura2[m->coda_array] = 0;
        m->stato_di_lettura3[m->coda_array] = 0;
        printf("R3: Cancello msg in posizione %d\n", m->coda_array);
        m->array[m->coda_array] = 0;
        m->coda_array = (m->coda_array + 1) % DIM_BUFFER;
    }

    pthread_cond_signal(&m->cond);

    pthread_mutex_unlock(&m->mutex);
}

void *mittente(void *arg)
{
    send(&mailbox, *(int *)arg);
    sleep(1);
}

void *ricevente1(void *arg)
{
    while (1)
    {
        receive1(&mailbox);
        sleep(1);
    }
}

void *ricevente2(void *arg)
{
    while (1)
    {
        receive2(&mailbox);
        sleep(1);
    }
}
void *ricevente3(void *arg)
{
    while (1)
    {
        receive3(&mailbox);
        sleep(1);
    }
}

int main()
{
    init_mailbox(&mailbox);

    pthread_attr_t a;
    pthread_t t;
    int taskid[NUM_MITTENTI];

    pthread_attr_init(&a);
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < NUM_MITTENTI; i++)
    {
        taskid[i] = i + 1;
        pthread_create(&t, &a, mittente, (void *)(&taskid[i]));
    }

    sleep(1);

    pthread_create(&t, &a, ricevente2, NULL);
    pthread_create(&t, &a, ricevente1, NULL);
    pthread_create(&t, &a, ricevente3, NULL);

    pthread_attr_destroy(&a);
    sleep(30);

    return 0;
}