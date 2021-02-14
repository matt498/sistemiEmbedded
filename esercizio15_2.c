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
    pthread_cond_t cond[NUM_PROCESSI];
    int blocked[NUM_PROCESSI]; //1 quando non puo prelevare
    int FIFO[NUM_PROCESSI]; //array circolare
    int quantita_prelievo[NUM_PROCESSI]; //utile al processo deposito quando deve svegliare.. esempio: saldo=0  richiesta di prelievo=100 deposito=50 --> richiesta non puo ancora essere soddisfatta
    int testa;
    int coda;
    int saldo;
    int bloccati; //in attesa
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

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        m->blocked[i] = 0;
        m->FIFO[i] = 0;
        m->quantita_prelievo[i] = 0;
        pthread_cond_init(&m->cond[i], &cattr);
    }

    m->testa = m->coda = m->bloccati = 0;
    m->saldo = SALDO;
    srand(time(NULL));
}

void sveglia(struct manager_t *m) //usato sia da prelievo che da deposito
{
    if (m->bloccati > 0)
    {
        if (m->quantita_prelievo[m->FIFO[m->coda]] <= m->saldo)
        {
            printf("Sveglio %d\n", m->FIFO[m->coda]);
            m->blocked[m->FIFO[m->coda]] = 0; //sblocco il processo cercando il suo indice nella fifo in coda
            pthread_cond_signal(&m->cond[m->FIFO[m->coda]]); //sveglio
            m->FIFO[m->coda] = 0; //resetto l'elemento nella fifo
            m->coda = (m->coda + 1) % NUM_PROCESSI; //aumento la coda
        }
    }
}

void prelievo(struct manager_t *m, int p, int ind)
{
    pthread_mutex_lock(&m->mutex);

    int entrato = 0;
    int index = ind - 1;

    m->quantita_prelievo[index] = p; //tengo traccia della quantità

    if ((m->saldo - p) < 0 || m->bloccati > 0) //mi blocco se non posso prelevare e se ci sono prelievi pendenti
    {
        printf("[%d] Voglio prelevare 100 e saldo: %d mi blocco\n", index, m->saldo);
        m->bloccati++;
        m->FIFO[m->testa] = index;
        m->testa = (m->testa + 1) % NUM_PROCESSI;
        m->blocked[index] = 1;
        entrato = 1; //aggiusto tutte le mie vaibili... "entrato" mi serve per dopo
    }

    while (m->blocked[index] == 1 && (m->saldo - p) < 0) //blocco della wait
    {
        pthread_cond_wait(&m->cond[index], &m->mutex);
    }

    if (entrato == 1) //se precedentemente mi sono bloccato decremento, se sono entrato senza bloccarmi non decremento
        m->bloccati--;
    m->saldo = m->saldo - p;
    printf("[%d] Prelevo 100, rimangono %d\n", index, m->saldo);

    sveglia(m);

    pthread_mutex_unlock(&m->mutex);
}

void deposito(struct manager_t *m, int d)
{
    pthread_mutex_lock(&m->mutex);

    m->saldo = m->saldo + d;
    printf("Deposito 50\n");

    sveglia(m);

    pthread_mutex_unlock(&m->mutex);
}

void *P(void *arg)
{
    while (1)
    {
        prelievo(&manager, 100, *(int *)arg);
        sleep(1);
    }
}

void *P2(void *arg)
{
    while (1)
    {
        deposito(&manager, 50);
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

    pthread_create(&t, &a, P2, NULL);

    for (int i = 0; i < NUM_PROCESSI; i++)
    {
        taskid[i] = i + 1;
        pthread_create(&t, &a, P, (void *)&taskid[i]);
    }

    pthread_attr_destroy(&a);
    sleep(20);

    return 0;
}
