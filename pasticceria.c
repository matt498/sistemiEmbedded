#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define N 3
struct pasticceria_t
{

    pthread_mutex_t mutex;
    pthread_cond_t cond_cuoco, cond_commesso, cond_vendita;

    int torte_invendute;
    int vendita_torta;

} pasticceria;

void init_pasticceria(struct pasticceria_t *p)
{

    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_condattr_init(&cattr);

    pthread_mutex_init(&p->mutex, &mattr);
    pthread_cond_init(&p->cond_cuoco, &cattr);
    pthread_cond_init(&p->cond_commesso, &cattr);
    pthread_cond_init(&p->cond_vendita, &cattr);

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);

    p->torte_invendute = p->vendita_torta = 0;

    srand(555);
}

void cuoco_inizio_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    while(p->torte_invendute >= N){
        pthread_cond_wait(&p->cond_cuoco, &p->mutex);
    }

    pthread_mutex_unlock(&p->mutex);
}

void cuoco_fine_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);
    p->torte_invendute++;
    pthread_cond_signal(&p->cond_commesso);
    pthread_mutex_unlock(&p->mutex);
}

void commesso_prendo_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    while(p->torte_invendute == 0){
        pthread_cond_wait(&p->cond_commesso, &p->mutex);
    }

    pthread_mutex_unlock(&p->mutex);
}

void commesso_vendo_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    while(p->vendita_torta == 0){
        pthread_cond_wait(&p->cond_vendita, &p->mutex);
    }

    p->vendita_torta=0;
    p->torte_invendute--;
    pthread_cond_signal(&p->cond_cuoco);

    pthread_mutex_unlock(&p->mutex);
}

void cliente_acquisto(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    p->vendita_torta = 1;
    pthread_cond_signal(&p->cond_vendita);

    pthread_mutex_unlock(&p->mutex);
}

void *cuoco(void *arg)
{
    while(1){
        sleep(1);
        cuoco_inizio_torta(&pasticceria);
        printf("Preparo torta\n");
        sleep(1);
        cuoco_fine_torta(&pasticceria);
        sleep(1);
    }
}

void *commesso(void *arg)
{
    while(1){
        sleep(1);
        commesso_prendo_torta(&pasticceria);
        printf("Preparo torta\n");
        sleep(1);
        commesso_vendo_torta(&pasticceria);
        sleep(1);
    }
}

void *un_cliente(void *arg)
{
    while(1){
        sleep(1);
        printf("vado in pasticceria per comprare una torta sopraffina\n");
        cliente_acquisto(&pasticceria);
        sleep(1);
        printf("torno a casa a mangiare la torta\n");
        sleep(1);
    }
}

int main() {
    pthread_attr_t attr;
    pthread_t thread;

    init_pasticceria(&pasticceria);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&thread, &attr, cuoco, NULL);
    pthread_create(&thread, &attr, commesso, NULL);
    pthread_create(&thread, &attr, un_cliente, NULL);

    pthread_attr_destroy(&attr);

    sleep(10);

    return 0;
}