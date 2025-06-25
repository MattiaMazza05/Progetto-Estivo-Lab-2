#include "types.h"
#include "rescuers.h"
#include "emergency.h"
#include "env.h"

int parse_emergency_types(void);
int parse_rescuers(void);
int parse_env(void);

#define QUEUE_NAME "/emergenze671860"
#define MAX_SIZE 1024
#define MSG_STOP "fileEnd"

int main()
{
    parse_rescuers();
    parse_emergency_types();
    parse_env();
    mqd_t mq;
    char buffer[MAX_SIZE];
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG_Q;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0; // mi definisco gli attibuti necessari per la coda

    // printf("%d\n", tot_emergenze);

    emergency_t *emergenze_valide = NULL;
    int tot_emer_valide = 0;
    emergency_request_t richiesta_emer;
    char nEmer[128];
    char coordX[16];
    char coordY[16];
    char delay_in_sec[21];

    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == -1)
    {
        PERROR_EXIT("⚠️ Errore nell'apertura della message queue ⚠️");
    }

    printf("\n--- Lettura messaggi dalla coda ---\n");
    while (1)
    {
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        if (bytes_read >= 0)
        {
            buffer[bytes_read] = '\0';
            int i = 0;
            int iC = 0;
            while (buffer[i] != '\0')
            {
                ESTRAI_EMER_MAIN(nEmer);
                strcpy(richiesta_emer.emergency_name, nEmer);
                ESTRAI_EMER_MAIN(coordX);
                richiesta_emer.x = atoi(coordX);
                ESTRAI_EMER_MAIN(coordY);
                richiesta_emer.y = atoi(coordY);
                ESTRAI_EMER_MAIN(delay_in_sec);
                long int delay = strtol(delay_in_sec, NULL, 10);
                richiesta_emer.timestamp = delay;

                for (int i = 0; i < tot_emergenze; i++)
                {
                    if (strcmp(richiesta_emer.emergency_name, emergenze_totali[i].emergency_desc) == 0 && richiesta_emer.x >= 0 && richiesta_emer.x <= width && richiesta_emer.y >= 0 && richiesta_emer.y <= height && richiesta_emer.timestamp >= 0)
                    {

                        emergency_t *tmp = realloc(emergenze_valide, sizeof(emergency_t) * (tot_emer_valide + 1));
                        if (tmp == NULL)
                        {
                            perror("⚠️ Errore durante l'assegnazione di memoria ⚠️");
                            free(emergenze_valide);
                            exit(EXIT_FAILURE);
                        }
                        emergenze_valide = tmp;
                        emergenze_valide[tot_emer_valide].x = richiesta_emer.x;
                        emergenze_valide[tot_emer_valide].y = richiesta_emer.y;
                        emergenze_valide[tot_emer_valide].time = richiesta_emer.timestamp;
                        for (int i = 0; i < tot_emergenze; i++)
                        {
                            if (strcmp(emergenze_totali[i].emergency_desc, richiesta_emer.emergency_name) == 0)
                            {
                                emergenze_valide[tot_emer_valide].type = emergenze_totali[i];
                                emergenze_valide[tot_emer_valide].rescuer_count = emergenze_totali[i].rescuers_req_number;
                                
                            }
                        }

                        tot_emer_valide++;
                    }
                    // qua inserire l'evento scartato nel file di log
                }
            }
            if (strcmp(buffer, MSG_STOP) == 0)
            {
                printf("--- Fine messaggi ---\n");
                break;
            }
        }
        else
        {
            PERROR_BREAK("⚠️ Errore lettura dalla message queue ⚠️");
        }
    }

    // Stampa dettagliata di tutte le emergenze valide
    printf("\n\x1b[35m=========== RIEPILOGO EMERGENZE PROCESSATE (%d) ===========\x1b[0m\n", tot_emer_valide);

    for (int i = 0; i < tot_emer_valide; i++)
    {
        printf("\n\x1b[33mEMERGENZA #%d\x1b[0m\n", i + 1);
        printf("  \x1b[36mTipo:\x1b[0m %s\n", emergenze_valide[i].type.emergency_desc);
        printf("  \x1b[36mPriorità:\x1b[0m %d\n", emergenze_valide[i].type.priority);
        printf("  \x1b[36mCoordinate:\x1b[0m (%d, %d)\n", emergenze_valide[i].x, emergenze_valide[i].y);
        printf("  \x1b[36mTimestamp:\x1b[0m %ld\n", emergenze_valide[i].time);
        printf("  \x1b[36mSoccorritori richiesti:\x1b[0m %d\n", emergenze_valide[i].rescuer_count);

        // Stampa dettagli dei soccorritori
        printf("  \x1b[36mSoccorritori:\x1b[0m\n");

        printf("  \x1b[32m--------------------------------------\x1b[0m\n");
    }

    printf("\n\x1b[35m=========================================================\x1b[0m\n");

    mq_close(mq);
    mq_unlink(QUEUE_NAME); // opzionale: elimina la coda
    return 0;
}