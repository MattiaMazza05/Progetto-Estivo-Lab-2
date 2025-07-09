#include "types.h"
#include "rescuers.h"
#include "emergency.h"
#include "log.h"

#define FILENAME "emergency_typers.conf"
#define BUFF_DIM 256

emergency_type_t *emergenze_totali = NULL; // creo un array generale per le emergenze allocando di volta in volta memoria in modo dinamico
int tot_emergenze = 0;                     // questo contatore mi serve per l'allocazione di memoria

int parse_emergency_types(void)
{
    int fd;
    SCALL(fd, open(FILENAME, O_RDONLY), "⚠️ Non sono riuscito ad aprire il file di configurazione ⚠️");
    scrivi_log("PARSE_EMERGENCY_TYPES", "FILE_PARSING", "File aperto");

    char buffer[BUFF_DIM];
    ssize_t bytesRead;

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

                    emergency_type_t *tmp = realloc(emergenze_totali, sizeof(emergency_type_t) * (tot_emergenze + 1)); // metto il valore di ritorno della relloc in una variabile temporanera per controllare se ci sono errori durante l'allocazione di memoria
                    if (tmp == NULL)
                    {
                        free(emergenze_totali); // libera la memoria già allocata
                        PERROR_EXIT("⚠️ Errore durante l'assegnazione di memoria ⚠️")
                    }
                    emergenze_totali = tmp;

                    emergenze_totali[tot_emergenze].priority = priorita;
                    emergenze_totali[tot_emergenze].emergency_desc = malloc(strlen(nome) + 1); // alloco memoria dinamica al nome per evitare che tutte le occorrenze abbiano come nome l'ultimo letto
                    strcpy(emergenze_totali[tot_emergenze].emergency_desc, nome);

                    rescuer_request_t *socc_per_emer = NULL; // creo un altro array dinamico soccorritori per emergenza
                    int num_soccorritori = 0;
                    iC = 0;
                    while (buffer[i] != '\n' && i < bytesRead)
                    {
                        rescuer_request_t *tmp2 = realloc(socc_per_emer, sizeof(rescuer_request_t) * (num_soccorritori + 1));
                        if (tmp2 == NULL)
                        {
                            free(socc_per_emer);
                            PERROR_EXIT("⚠️ Errore durante l'assegnazione di memoria dinamica socc_per_emer ⚠️");
                        }
                        socc_per_emer = tmp2;
                        iC = 0;
                        if (buffer[i] == ' ')
                        { // aggiungo questo controllo perchè il primo nome di ogni mezzo era ' Nome' questo non mi faceva corrispondere la struct al nome
                            i++;
                        }
                        while (buffer[i] != ':' && i < bytesRead)
                        {
                            nM[iC++] = buffer[i++];
                        }
                        nM[iC] = '\0';
                        i++;
                        iC = 0;
                        while (buffer[i] != ',' && i < bytesRead)
                        {
                            nS[iC++] = buffer[i++];
                        }
                        nS[iC] = '\0';
                        i++;
                        iC = 0;
                        while (buffer[i] != ';' && i < bytesRead)
                        {
                            nSec[iC++] = buffer[i++];
                        }
                        nSec[iC] = '\0';
                        i++;
                        for (int k = 0; k < tot_mezzi; k++)
                        {
                            if (strcmp(nM, mezzi_totali[k].rescuer->rescuer_type_name) == 0)
                            {
                                socc_per_emer[num_soccorritori].type = mezzi_totali[k].rescuer;
                                break;
                            }
                        }
                        socc_per_emer[num_soccorritori].required_count = atoi(nS);
                        socc_per_emer[num_soccorritori].time_to_manage = atoi(nSec);
                        num_soccorritori++;
                    }

                    emergenze_totali[tot_emergenze].rescuers = socc_per_emer;
                    emergenze_totali[tot_emergenze].rescuers_req_number = num_soccorritori;
                    char log_msg[256];
                    snprintf(log_msg, sizeof(log_msg), "Nome: %s | Priorità: %d | Soccorritori richiesti: %d",
                             emergenze_totali[tot_emergenze].emergency_desc,
                             emergenze_totali[tot_emergenze].priority,
                             emergenze_totali[tot_emergenze].rescuers_req_number);

                    scrivi_log("parse_emergency_types.c", "FILE_PARSING", log_msg);

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
    scrivi_log("PARSE_EMERGENCY_TYPES", "FILE_PARSING", "File chiuso");
    return 0;
}

void free_emergenze(void){
    for (int i = 0; i < tot_emergenze; i++) {
        free(emergenze_totali[i].emergency_desc);
        free(emergenze_totali[i].rescuers);
    }
    free(emergenze_totali);
}