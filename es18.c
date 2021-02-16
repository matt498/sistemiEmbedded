#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>

#define SEQUENZA_NESSUNO 0
#define SEQUENZA_F1      1
#define SEQUENZA_F2      2
#define SEQUENZA_F3      3
#define SEQUENZA_F4      4
#define SEQUENZA_F5      5
#define SEQUENZA_F6      6


/* la struttura condivisa */
struct gestore_t {
	pthread_mutex_t mutex;

	pthread_cond_t cond_f1, cond_f2, cond_f3, cond_f4, cond_f4_durante_3, cond_f5, cond_f6;
	int c_f1, c_f2, c_f3, c_f4, c_f5, c_f6;

	/* stato del sistema */
	int b_f1, b_f2, b_f3, b_f4, b_f5, b_f6;
	int stato;
	
	int b_f4_durante_3;

} gestore;

void init_gestore(struct gestore_t* g)
{
	pthread_mutexattr_t m_attr;
	pthread_condattr_t c_attr;

	pthread_mutexattr_init(&m_attr);
	pthread_condattr_init(&c_attr);

	pthread_mutex_init(&g->mutex, &m_attr);
	pthread_cond_init(&g->cond_f1, &c_attr);
	pthread_cond_init(&g->cond_f2, &c_attr);
	pthread_cond_init(&g->cond_f3, &c_attr);
	pthread_cond_init(&g->cond_f4, &c_attr);
	pthread_cond_init(&g->cond_f4_durante_3, &c_attr);
	pthread_cond_init(&g->cond_f5, &c_attr);
	pthread_cond_init(&g->cond_f6, &c_attr);

	pthread_condattr_destroy(&c_attr);
	pthread_mutexattr_destroy(&m_attr);

	g->c_f1 = g->c_f2 = g->c_f3 = g->c_f4 = g->c_f5 = g->c_f6 = 0;

	g->b_f1 = g->b_f2 = g->b_f3 = g->b_f4 = g->b_f5 = g->b_f6 = 0;

	g->b_f4_durante_3 = 0;

	/* stato del sistema */
	g->stato = SEQUENZA_NESSUNO;
}


//implementazione delle funzioni

void StartF1(struct gestore_t* g)
{
	
	pthread_mutex_lock(&g->mutex);
	while (g->stato != SEQUENZA_NESSUNO) {

		g->b_f1++;
		pthread_cond_wait(&g->cond_f1, &g->mutex);
		g->b_f1--;

	}

	g->c_f1++;

	g->stato = SEQUENZA_F1;

	pthread_mutex_unlock(&g->mutex);

}

void EndF1(struct gestore_t* g)
{
	pthread_mutex_lock(&g->mutex);

	g->c_f1--;
	if (g->c_f1 == 0) {

		g->stato = SEQUENZA_F3;
		pthread_cond_broadcast(&g->cond_f3);
	
	}
		

	pthread_mutex_unlock(&g->mutex);
}

void StartF2(struct gestore_t* g)
{

	pthread_mutex_lock(&g->mutex);
	
	while (g->stato != SEQUENZA_NESSUNO) {

		g->b_f2++;
		pthread_cond_wait(&g->cond_f2, &g->mutex);
		g->b_f2--;

	}

	g->c_f2++;

	g->stato = SEQUENZA_F2;

	pthread_mutex_unlock(&g->mutex);

}

void EndF2(struct gestore_t* g)
{
	pthread_mutex_lock(&g->mutex);

	g->c_f2--;
	if (g->c_f2 == 0) {

		g->stato = SEQUENZA_F3;
		pthread_cond_broadcast(&g->cond_f3);

	}

	pthread_mutex_unlock(&g->mutex);
}


void StartF3(struct gestore_t* g)
{

	pthread_mutex_lock(&g->mutex);

	
	while ((g->stato != SEQUENZA_F3) || (g->stato == SEQUENZA_F3 && g->b_f4_durante_3>0)) {

		g->b_f3++;
		pthread_cond_wait(&g->cond_f3, &g->mutex);
		g->b_f3--;

	}

	g->c_f3++;

	pthread_mutex_unlock(&g->mutex);

}

void EndF3(struct gestore_t* g)
{
	pthread_mutex_lock(&g->mutex);

	g->c_f3--;
	if (g->c_f3 == 0) {

		g->stato = SEQUENZA_F4;
		pthread_cond_broadcast(&g->cond_f4_durante_3);
		pthread_cond_broadcast(&g->cond_f4);
	

	}

	pthread_mutex_unlock(&g->mutex);


}

void StartF4(struct gestore_t* g)
{

	pthread_mutex_lock(&g->mutex);

	while ((g->stato == SEQUENZA_F3)) {

		g->b_f4_durante_3++;
		pthread_cond_wait(&g->cond_f4_durante_3, &g->mutex);
		g->b_f4_durante_3--;

	}

	while ((g->stato != SEQUENZA_F4) && (g->stato != SEQUENZA_F3)) {

		g->b_f4++;
		pthread_cond_wait(&g->cond_f4, &g->mutex);
		g->b_f4--;

	}

	g->c_f4++;

	pthread_mutex_unlock(&g->mutex);

}

void EndF4(struct gestore_t* g)
{
	pthread_mutex_lock(&g->mutex);

	g->c_f4--;
	if (g->c_f4 == 0) {

		g->stato = SEQUENZA_NESSUNO;
		
		pthread_cond_broadcast(&g->cond_f1);
		pthread_cond_broadcast(&g->cond_f2);
		pthread_cond_broadcast(&g->cond_f5);
		

	}

	pthread_mutex_unlock(&g->mutex);
}


void StartF5(struct gestore_t* g)
{

	pthread_mutex_lock(&g->mutex);

	while (g->stato != SEQUENZA_NESSUNO) {

		g->b_f5++;
		pthread_cond_wait(&g->cond_f5, &g->mutex);
		g->b_f5--;

	}

	g->c_f5++;

	g->stato = SEQUENZA_F5;

	pthread_mutex_unlock(&g->mutex);

}

void EndF5(struct gestore_t* g)
{
	pthread_mutex_lock(&g->mutex);

	g->c_f5--;
	if (g->c_f5 == 0) {

		g->stato = SEQUENZA_F6;
		pthread_cond_broadcast(&g->cond_f6);

	}

	pthread_mutex_unlock(&g->mutex);
}

void StartF6(struct gestore_t* g)
{

	pthread_mutex_lock(&g->mutex);

	while (g->stato != SEQUENZA_F6) {

		g->b_f6++;
		pthread_cond_wait(&g->cond_f6, &g->mutex);
		g->b_f6--;

	}

	g->c_f6++;

	pthread_mutex_unlock(&g->mutex);

}


void EndF6(struct gestore_t* g)
{
	pthread_mutex_lock(&g->mutex);

	g->c_f6--;
	if (g->c_f6 == 0) {

		g->stato = SEQUENZA_NESSUNO;

		pthread_cond_broadcast(&g->cond_f1);
		pthread_cond_broadcast(&g->cond_f2);
		pthread_cond_broadcast(&g->cond_f5);

	}

	pthread_mutex_unlock(&g->mutex);
}


/* alla fine di ogni ciclo ogni thread aspetta un po'.
   Cosa succede se tolgo questa nanosleep?
   di fatto solo i thread di tipo B riescono ad entrare --> starvation!!!!
   (provare per credere)
*/
void pausetta(void)
{
	struct timespec t;
	t.tv_sec = 0;
	t.tv_nsec = (rand() % 10 + 1) * 1000000;
	nanosleep(&t, NULL);
}


/* i thread */


void* F1(void* arg)
{
	for (;;) {
		StartF1(&gestore);
		putchar(*(char*)arg);
		EndF1(&gestore);
		pausetta();
	}
	return 0;
}

void* F2(void* arg)
{
	for (;;) {
		StartF2(&gestore);
		putchar(*(char*)arg);
		EndF2(&gestore);
		pausetta();
	}
	return 0;
}

void* F3(void* arg)
{
	for (;;) {
		StartF3(&gestore);
		putchar(*(char*)arg);
		EndF3(&gestore);
		pausetta();
	}
	return 0;
}

void* F4(void* arg)
{
	for (;;) {
		StartF4(&gestore);
		putchar(*(char*)arg);
		EndF4(&gestore);
		pausetta();
	}
	return 0;
}

void* F5(void* arg)
{
	for (;;) {
		StartF5(&gestore);
		putchar(*(char*)arg);
		EndF5(&gestore);
		pausetta();
	}
	return 0;
}

void* F6(void* arg)
{
	for (;;) {
		StartF6(&gestore);
		putchar(*(char*)arg);
		EndF6(&gestore);
		pausetta();
	}
	return 0;
}


/* la creazione dei thread */



int main()
{
	pthread_attr_t a;
	pthread_t p;

	/* inizializzo il mio sistema */
	init_gestore(&gestore);

	/* inizializzo i numeri casuali, usati nella funzione pausetta */
	srand(555);

	pthread_attr_init(&a);

	/* non ho voglia di scrivere 10000 volte join! */
	pthread_attr_setdetachstate(&a, PTHREAD_CREATE_DETACHED);

	pthread_create(&p, &a, F1, (void*)"1");
	pthread_create(&p, &a, F1, (void*)"1");

	pthread_create(&p, &a, F2, (void*)"2");
	pthread_create(&p, &a, F2, (void*)"2");
	pthread_create(&p, &a, F2, (void*)"2");

	pthread_create(&p, &a, F3, (void*)"3");
	pthread_create(&p, &a, F3, (void*)"3");

	pthread_create(&p, &a, F4, (void*)"4");
	pthread_create(&p, &a, F4, (void*)"4");

	pthread_create(&p, &a, F5, (void*)"5");
	pthread_create(&p, &a, F5, (void*)"5");

	pthread_create(&p, &a, F6, (void*)"6");
	pthread_create(&p, &a, F6, (void*)"6");

	pthread_attr_destroy(&a);

	/* aspetto 10 secondi prima di terminare tutti quanti */
	sleep(10);

	return 0;
}
