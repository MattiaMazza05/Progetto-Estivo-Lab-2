#include "gestione_thread.h"

thread_slot_t queue_thread[DIMENSIONE_CODA_THREAD];

int thread_attesa[DIMENSIONE_THREAD_ATTESA];
int coda_testa = 0, coda_fine = 0, contatore_coda = 0;

void inserisci_emer(int emer_id)
{
    if (contatore_coda < DIMENSIONE_THREAD_ATTESA)
    {
        thread_attesa[coda_fine] = emer_id;
        coda_fine = (coda_fine + 1) % DIMENSIONE_THREAD_ATTESA;
        contatore_coda++;
    }
}

int rimuovi_emer(void)
{
    if (contatore_coda == 0)
        return -1;
    int id = thread_attesa[coda_testa];
    coda_testa = (coda_testa + 1) % DIMENSIONE_THREAD_ATTESA;
    contatore_coda--;
    return id;
}

void check_threads(void)
{
    for (int i = 0; i < DIMENSIONE_CODA_THREAD; i++)
    {
        if (queue_thread[i].occupato)
        {
            if (queue_thread[i].occupato && queue_thread[i].terminato)
            {
                thrd_join(queue_thread[i].thread, NULL);
                queue_thread[i].occupato = false;
                queue_thread[i].emer_id = -1;
                queue_thread[i].terminato = false;
            }
        }
    }
}

