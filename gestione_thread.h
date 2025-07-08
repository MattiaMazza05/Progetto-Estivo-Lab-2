#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include "types.h"

extern thread_slot_t queue_thread[DIMENSIONE_CODA_THREAD];
extern int contatore_coda;
void inserisci_emer(int emer_id);
int rimuovi_emer(void);
void check_threads(void);

#endif