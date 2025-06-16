#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macro.h"
#include "types.h"

#define FILENAME "emergency_typers.conf"
#define BUFF_DIM 128

int main()
{
    int fd;
    SCALL(fd, open(FILENAME, O_RDONLY),
          "⚠️ Non sono riuscito ad aprire il file di configurazione ⚠️");

    char buffer[BUFF_DIM];
    ssize_t bytesRead;

    emergency_type_t *emergenze_totali =
        NULL; // creo un array generale per le emergenze allocando di volta in
              // volta memoria in modo dinamico
    int tot_emergenze =
        0; // questo contatore mi serve per l'allocazione di memoria

    SCALLREAD(
        bytesRead, read(fd, buffer, BUFF_DIM),
        {
            int i = 0;
            char nome[BUFF_DIM];
            char pr[16]; // char di appoggio per estrarre la priorità
            int priorita;
            char nM[BUFF_DIM]; // nM (nome Mezzo)
            char nS[16];       // nS (numero Soccorritori)
            char nSec[16];     // nSec (numero Secondi)
            while (i < bytesRead)
            {
                if (buffer[i] == '[')
                {
                    i++;
                    int iC = 0;
                    ESTRAIDATO(nome)
                    nome[iC] = '\0';
                    iC = 0;
                    FINOAPROXDATO
                    ESTRAIDATO(pr)
                    pr[iC] = '\0';
                    priorita = atoi(pr);
                    iC = 0;
                    i++;

                    emergency_type_t *tmp = realloc(emergenze_totali, sizeof(emergency_type_t) * (tot_emergenze +1)); // metto il valore di ritorno della relloc in una variabile temporanera per controllare se ci sono errori durante l'allocazione di memoria
                    if (tmp == NULL)
                    {
                        // Gestione errore: memoria esaurita
                        perror("⚠️ Errore durante l'assegnazione di memoria ⚠️");
                        free(emergenze_totali); // libera la memoria già allocata
                        exit(EXIT_FAILURE);
                    }
                    emergenze_totali = tmp;

                    emergenze_totali[tot_emergenze].priority = priorita;
                    emergenze_totali[tot_emergenze].emergency_desc = malloc(
                        strlen(nome) +
                        1); // alloco memoria dinamica al nome per evitare che tutte le
                            // occorrenze abbiano come nome l'ultimo letto
                    strcpy(emergenze_totali[tot_emergenze].emergency_desc, nome);

                    rescuer_request_t *socc_per_emer =
                        NULL; // creo un altro array dinamico soccorritori per
                              // emergenza
                    int num_soccorritori = 0;
                    iC=0;
                    while (buffer[i] != '\n' && i<bytesRead) {
                        while (buffer[i] != ':' && i<bytesRead) {
                            nM[iC++] = buffer[i++];
                        }
                        nM[iC] = '\0';
                        i++;
                        num_soccorritori++;
                        while (buffer[i]) {
                            
                        }
                    }
                    nM[iC] = '\0';
                    printf("%s\n", nM);
                    tot_emergenze++;
                }
                else
                {
                    i++;
                }
            }
        },
        "⚠️ Errore durante la lettura del file ⚠️");

    SCALL(fd, close(fd), "⚠️ Errore durante la chiusura del file ⚠️");

    for (int i = 0; i < tot_emergenze; i++)
    {
        printf("%d, %s, %d, %d\n", i, emergenze_totali[i].emergency_desc,
               emergenze_totali[i].priority, tot_emergenze);
    }
    return 0;
}
