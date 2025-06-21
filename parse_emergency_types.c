#include "types.h"
#include "rescuers.h"
#include "emergency.h"

#define FILENAME "emergency_typers.conf"
#define BUFF_DIM 256

emergency_type_t *emergenze_totali =
    NULL;              // creo un array generale per le emergenze allocando di volta in volta memoria in modo dinamico
int tot_emergenze = 0; // questo contatore mi serve per l'allocazione di memoria

int parse_emergency_types(void)
{
    int fd;
    SCALL(fd, open(FILENAME, O_RDONLY),
          "⚠️ Non sono riuscito ad aprire il file di configurazione ⚠️");

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
                        perror("⚠️ Errore durante l'assegnazione di memoria ⚠️");
                        free(emergenze_totali); // libera la memoria già allocata
                        exit(EXIT_FAILURE);
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
                            perror("⚠️ Errore durante l'assegnazione di memoria dinamica socc_per_emer ⚠️");
                            free(socc_per_emer);
                            exit(EXIT_FAILURE);
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

    printf("\n\x1b[32m========== VERIFICA EMERGENZE ==========\x1b[0m\n\n");
    printf("\x1b[36m%-3s %-15s %-8s %-8s %-15s %-8s %-8s %-10s\x1b[0m\n",
           "ID", "Nome", "Priorità", "Rich#", "Tipo", "Quantità", "Tempo", "Velocità");

    for (int e = 0; e < tot_emergenze; e++)
    {
        int req_num = emergenze_totali[e].rescuers_req_number;
        for (int r = 0; r < req_num; r++)
        {
            printf("%-3d %-15s %-8d %-8d ",
                   e + 1,
                   emergenze_totali[e].emergency_desc,
                   emergenze_totali[e].priority,
                   req_num);
            if (emergenze_totali[e].rescuers[r].type)
            {
                printf("%-15s %-8d %-8d %-10d\n",
                       emergenze_totali[e].rescuers[r].type->rescuer_type_name,
                       emergenze_totali[e].rescuers[r].required_count,
                       emergenze_totali[e].rescuers[r].time_to_manage,
                       emergenze_totali[e].rescuers[r].type->speed);
            }
            else
            {
                printf("\x1b[31m%-15s\x1b[0m %-8d %-8d %-10s\n",
                       "NON ASSEGNATO",
                       emergenze_totali[e].rescuers[r].required_count,
                       emergenze_totali[e].rescuers[r].time_to_manage,
                       "-");
            }
        }
    }
    printf("\n\x1b[32m========== FINE VERIFICA EMERGENZE ==========\x1b[0m\n");
    return 0;
}
