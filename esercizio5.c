#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 11

struct urna_t
{
    pthread_mutex_t mutex;
    pthread_cond_t cond_risultato;

    int voti_positivi, risultato, voti_negativi;

} urna;

void init_urna(struct urna_t *u)
{
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&u->mutex, &mattr);
    pthread_cond_init(&u->cond_risultato, &cattr);

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    u->voti_positivi = u->voti_negativi = 0;
    u->risultato = 3; //importante che non sia 0 o 1

    srand(555);
}

void vota(int v)
{
    pthread_mutex_lock(&urna.mutex);

    if (v == 1)
    {
        urna.voti_positivi++;
    }
    else {
        urna.voti_negativi++;
    }

    pthread_mutex_unlock(&urna.mutex);
}

int risultato()
{
    pthread_mutex_lock(&urna.mutex);

    while (urna.voti_positivi < (N / 2) + 1 && urna.voti_negativi < (N / 2) + 1)
    {
        pthread_cond_wait(&urna.cond_risultato, &urna.mutex);
    }

    pthread_cond_broadcast(&urna.cond_risultato);

    if (urna.voti_positivi >= ((N / 2) + 1))
    {
        urna.risultato = 1;
    }
    else
    {
        urna.risultato = 0;
    }

    pthread_mutex_unlock(&urna.mutex);
    return urna.risultato;
}

void *threadd(void *arg)
{
    int voto = rand() % 2;
    vota(voto);
    printf("[%lu] voto: %d\n", pthread_self(), voto);
    if (voto == risultato())
        printf("[%lu] Ho vinto!\n", pthread_self());
    else
        printf("[%lu] Ho perso!\n", pthread_self());
    pthread_exit(0);
}

int main()
{
    pthread_attr_t attr;
    pthread_t thread;

    init_urna(&urna);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    for (int i = 0; i < N; i++)
    {
        pthread_create(&thread, &attr, threadd, NULL);
    }

    pthread_attr_destroy(&attr);
    sleep(10);

    return 0;
}