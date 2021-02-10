//TRABUCCO MATTIA - matricola 160323
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define N 5               //numero massimo di torte prodotte e stoccabili nella pasticceria
#define NUMERO_CLIENTI 10 //numero di thread clienti

struct pasticceria_t
{
    pthread_mutex_t mutex;
    pthread_cond_t condv_cuoco;                                         //variable contition per cuoco
    pthread_cond_t condv_commesso;                                      //variable contition per commesso (fase di incarto)
    pthread_cond_t condv_commesso_vendita;                              //variable contition per commesso (fase di vendita)
    pthread_cond_t condv_cliente;                                       //variable contition per i clienti
    int torte_prodotte;                                                 //conteggio delle torte prodotte dal cuoco
    int torte_incartate;                                                //conteggio delle torte incartate dal commesso
    int coda_cuoco, coda_commesso, coda_commesso_vendita, coda_cliente; //conteggio delle code

} pasticceria;

void myInit(struct pasticceria_t *p)
{
    pthread_mutexattr_t m_attr;
    pthread_condattr_t c_attr;

    pthread_mutexattr_init(&m_attr);
    pthread_condattr_init(&c_attr);

    pthread_mutex_init(&p->mutex, &m_attr);

    pthread_cond_init(&p->condv_cuoco, &c_attr);
    pthread_cond_init(&p->condv_commesso, &c_attr);
    pthread_cond_init(&p->condv_commesso_vendita, &c_attr);
    pthread_cond_init(&p->condv_cliente, &c_attr);

    pthread_condattr_destroy(&c_attr);
    pthread_mutexattr_destroy(&m_attr);

    /* ------ ERRORE: nello scritto manca "p->" (foglio 1 di 4) ------ */
    p->torte_prodotte = p->torte_incartate = p->coda_cuoco = p->coda_commesso = p->coda_commesso_vendita = p->coda_cliente = 0;
}

void cuoco_inizio_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    while (p->torte_prodotte >= N) //mi blocco se ci sono troppe torte invendute
    {
        p->coda_cuoco++;
        pthread_cond_wait(&p->condv_cuoco, &p->mutex);
        p->coda_cuoco--;
    }

    pthread_mutex_unlock(&p->mutex);
}

void cuoco_fine_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    p->torte_prodotte++; //ho effettivamente prodotto una torta

    if (p->coda_commesso > 0) //se c'e' il commesso in coda (fase di incarto) lo sveglio
    {
        pthread_cond_signal(&p->condv_commesso);
    }

    pthread_mutex_unlock(&p->mutex);
}

void commesso_prendo_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    while (p->torte_prodotte == 0) //mi blocco se non ci sono torte prodotte
    {
        p->coda_commesso++;
        pthread_cond_wait(&p->condv_commesso, &p->mutex); 
        p->coda_commesso--;
    }

    pthread_mutex_unlock(&p->mutex);
}

void commesso_vendo_torta(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    while (p->coda_cliente == 0) //mi blocco se non ci sono clienti che vogliono comprare
    {
        p->coda_commesso_vendita++;
        pthread_cond_wait(&p->condv_commesso_vendita, &p->mutex); 
        p->coda_commesso_vendita--;
    }

    p->torte_prodotte--;  //prendo una torta prodotta dalla vetrina
    p->torte_incartate++; //incarto la torta per il cliente (sono a conoscenza che la torta incartata non è direttamente assegnata al cliente, ma non ho trovato riscontri nelle specifiche che facessero pensare a tale necessità.)

    pthread_cond_signal(&p->condv_cliente); //non serve controllare se ci siano clienti in coda perche' ho passato il while. Serve per sincronizzarmi con il cliente.

    if (p->coda_cuoco > 0) //se c'è il cuoco in coda lo sveglio
    {
        pthread_cond_signal(&p->condv_cuoco);
    }

    pthread_mutex_unlock(&p->mutex);
}

void cliente_acquisto(struct pasticceria_t *p)
{
    pthread_mutex_lock(&p->mutex);

    if (p->coda_commesso_vendita > 0) //se c'è il commesso_vendita in coda (fase di vendita) lo sveglio. Serve per sincronizzarmi con il commesso.
    {
        pthread_cond_signal(&p->condv_commesso_vendita);
    }

    while (p->torte_incartate == 0) //mi blocco se non ci sono torte incartate
    {
        p->coda_cliente++;
        pthread_cond_wait(&p->condv_cliente, &p->mutex); //non era richiesto un particolare ordine di accomodamento, ma allora potrebbe verificarsi starvation (un nuovo thread cliente potrebbe "passare davanti" a quelli in coda).
        p->coda_cliente--;
    }

    p->torte_incartate--; //prendo la torta incartata

    pthread_mutex_unlock(&p->mutex);
}

void *cuoco(void *arg)
{
    while (1)
    {
        cuoco_inizio_torta(&pasticceria); //1 torta; può essere bloccante

        sleep(1); //simulo <preparo torta>
        printf("Cuoco esegue\n");

        cuoco_fine_torta(&pasticceria); //1 torta; non bloccante
        printf("Cuoco ha preparato una torta\n");
    }
}
void *commesso(void *arg)
{
    while (1)
    {
        commesso_prendo_torta(&pasticceria); //1 torta; bloccante

        sleep(1); //simulto <incarto la torta in una confezione colorata>
        printf("Commesso esegue\n");

        commesso_vendo_torta(&pasticceria); //1 torta; bloccante
        printf("Commesso ha incartato e venduto una torta\n");
    }
}
void *un_cliente(void *arg)
{
    while (1)
    {
        sleep(1); //simulo <vado in pasticceria per comprare una torta sopraffina>

        cliente_acquisto(&pasticceria); //1 torta; bloccante
        printf("Cliente%ld ho comprato una torta\n", (long)arg);

        sleep(1); //simulo <torno a casa a mangiare la torta>
    }
}

int main(void)
{
    pthread_attr_t a;
    pthread_t p;

    //inizializzo
    myInit(&pasticceria);

    pthread_attr_init(&a);

    //per non scrivere le join:
    pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

    pthread_create(&p, &a, cuoco, (void *)"A"); //creazione thread cuoco
    printf("[MAIN] Creato thread cuoco\n");
    pthread_create(&p, &a, commesso, (void *)"B"); //creazione thread commesso
    printf("[MAIN] Creato thread commesso\n");

    long i;
    for (i = 0; i < NUMERO_CLIENTI; i++) //creazione di NUMERO_CLIENTI thread clienti
    {
        pthread_create(&p, &a, un_cliente, (void *)i);
        printf("[MAIN] Creato thread cliente%ld\n", i);
    }

    pthread_attr_destroy(&a);

    //aspetto prima di terminare
    sleep(15);

    return 0;
}