#include "types.h"
#include "rescuers.h"
#include "emergency.h"
#include "env.h"

int parse_emergency_types(void);
int parse_rescuers(void);
int parse_env(void);

#define MAX_SIZE 1024
#define MSG_STOP "fileEnd"

emergency_t *emergenze_valide = NULL;
int tot_emer_valide = 0;

mtx_t mutex_soccorritori;

int gestione_emer(void *arg)
{
    int emer_id = *(int *)arg;
    emergency_t *emergenza = &emergenze_valide[emer_id - 1];

    // calcolo il tempo massimo in base alla priorità
    time_t t_limite = 0;
    switch (emergenza->type.priority)
    {
    case 1:
        t_limite = PRIORITY_1;
        break;
    case 2:
        t_limite = PRIORITY_2;
        break;
    default:
        t_limite = 100;
        break;
    }

    mtx_lock(&mutex_soccorritori);

    for (int i = 0; i < tot_emergenze; i++)
    {
        if (strcmp(emergenza->type.emergency_desc, emergenze_totali[i].emergency_desc) == 0)
        {
            bool tutti_disponibili = true;
            int tempo_totale = 0;
            int tipi_mezzi_controllati = 0;
            int mezzi_totali_assegnati = 0;

            for (int j = 0; j < emergenze_totali[i].rescuers_req_number; j++)
            {
                int mezzi_necessari = emergenze_totali[i].rescuers[j].required_count;
                int mezzi_disponibili = 0;
                int tempo_tipo_mezzo = 0;
                bool tipo_calcolato = false;

                for (int k = 0; k < tot_mezzi; k++)
                {
                    if (strcmp(emergenze_totali[i].rescuers[j].type->rescuer_type_name,
                               mezzi_totali[k].rescuer->rescuer_type_name) == 0)
                    {
                        if (mezzi_totali[k].status == IDLE)
                        {
                            mezzi_disponibili++;

                            if (!tipo_calcolato)
                            {
                                DISTANZA_MANHATTAN(mezzi_totali[k].x, mezzi_totali[k].y,
                                                   emergenza->x, emergenza->y,
                                                   mezzi_totali[k].rescuer->speed);

                                tempo_tipo_mezzo = vel;
                                tipo_calcolato = true;
                            }
                        }
                    }
                }

                tipi_mezzi_controllati++;

                if (tempo_tipo_mezzo > tempo_totale)
                    tempo_totale = tempo_tipo_mezzo;

                if (tempo_totale > t_limite)
                {
                    emergenza->status = TIMEOUT;
                    tutti_disponibili = false;
                    break;
                }

                if (mezzi_disponibili < mezzi_necessari)
                {
                    tutti_disponibili = false;
                    break;
                }
                else
                {
                    int mezzi_assegnati_tipo = 0;
                    for (int k = 0; k < tot_mezzi && mezzi_assegnati_tipo < mezzi_necessari; k++)
                    {
                        if (strcmp(emergenze_totali[i].rescuers[j].type->rescuer_type_name,
                                   mezzi_totali[k].rescuer->rescuer_type_name) == 0)
                        {
                            if (mezzi_totali[k].status == IDLE)
                            {
                                mezzi_totali[k].status = EN_ROUTE_TO_SCENE;
                                mezzi_assegnati_tipo++;
                                mezzi_totali_assegnati++;
                            }
                        }
                    }
                }
            }
            sleep(tempo_totale);

            if (tutti_disponibili && tempo_totale <= t_limite)
            {
                emergenza->status = IN_PROGRESS;

                sleep(tempo_totale);

                int tempo_totale_gestire = 0;
                int tempo_gestire_soccorritore = 0;

                CAMBIO_STATO_SOCCORRITORE_CON_TEMPO(tempo_gestire_soccorritore, tempo_totale_gestire, EN_ROUTE_TO_SCENE, ON_SCENE);

                sleep(tempo_totale_gestire);

                CAMBIO_STATO_SOCCORRITORE(ON_SCENE, RETURNING_TO_BASE);

                sleep(tempo_totale);

                CAMBIO_STATO_SOCCORRITORE(RETURNING_TO_BASE, IDLE);

                emergenza->status = COMPLETED;
            }
            else
            {
                if (!tutti_disponibili)
                {
                    emergenza->status = WAITING;
                }
                else if (tempo_totale > t_limite)
                {
                    emergenza->status = TIMEOUT;
                }
            }

            break;
        }
    }

    mtx_unlock(&mutex_soccorritori);
    return 0;
}

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

    emergency_request_t richiesta_emer;
    char nEmer[128];
    char coordX[16];
    char coordY[16];
    char delay_in_sec[21];

    // dichiarazione array dinamico thread e inizializzazione mutex
    thrd_t *emergenze = NULL;
    int *id_emer = NULL;
    int thread_c = 0; // contatore per i thread
    mtx_init(&mutex_soccorritori, mtx_plain);

    mq = mq_open(n_queue, O_CREAT | O_RDONLY, 0644, &attr);
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
            if (strcmp(buffer, MSG_STOP) == 0)
            {
                for (int i = 0; i < thread_c; i++)
                {
                    thrd_join(emergenze[i], NULL);
                    printf("Thread %d terminato\n", id_emer[i]);
                }
                printf("--- Comando exit ricevuto - Chiusura server ---\n");
                break;
            }
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
                        emergenze_valide[tot_emer_valide].status = WAITING;
                        emergenze_valide[tot_emer_valide].x = richiesta_emer.x;
                        emergenze_valide[tot_emer_valide].y = richiesta_emer.y;
                        emergenze_valide[tot_emer_valide].time = richiesta_emer.timestamp;
                        emergenze_valide[tot_emer_valide].type = emergenze_totali[i];
                        emergenze_valide[tot_emer_valide].rescuer_count = emergenze_totali[i].rescuers_req_number;

                        emergenze_valide[tot_emer_valide].rescuers_dt = malloc(emergenze_valide[tot_emer_valide].rescuer_count * sizeof(rescuer_digital_twin_t));
                        for (int j = 0; j < emergenze_totali[i].rescuers_req_number; j++)
                        {
                            for (int k = 0; k < tot_mezzi; k++)
                            {
                                if (strcmp(emergenze_totali[i].rescuers[j].type->rescuer_type_name, mezzi_totali[k].rescuer->rescuer_type_name) == 0)
                                {
                                    emergenze_valide[tot_emer_valide].rescuers_dt[j] = mezzi_totali[k];
                                }
                            }
                        }

                        tot_emer_valide++;

                        emergenze = realloc(emergenze, sizeof(thrd_t) * (thread_c + 1));
                        id_emer = realloc(id_emer, sizeof(int) * (thread_c + 1));

                        if (emergenze == NULL || id_emer == NULL)
                        {
                            PERROR_EXIT("⚠️ Errore allocazione memoria thread ⚠️");
                        }

                        id_emer[thread_c] = tot_emer_valide; // in questo modo l'id del thread è il numero di emergenza

                        printf("🚨 Emergenza ricevuta: %s (%d,%d) - Creazione Thread %d\n",
                               richiesta_emer.emergency_name,
                               richiesta_emer.x, richiesta_emer.y,
                               id_emer[thread_c]);

                        if (thrd_create(&emergenze[thread_c], gestione_emer, &id_emer[thread_c]) != thrd_success)
                        {
                            printf("❌ Errore creazione thread per emergenza %d\n", id_emer[thread_c]);
                        }
                        else
                        {
                            printf("✅ Thread %d avviato per gestione emergenza\n", id_emer[thread_c]);
                            thread_c++;
                        }
                    }
                    // qua inserire l'evento scartato nel file di log
                }
            }
        }
        else
        {
            PERROR_BREAK("⚠️ Errore lettura dalla message queue ⚠️");
        }
    }

    // printf("\n=== RIEPILOGO EMERGENZE VALIDATE (%d) ===\n\n", tot_emer_valide);
    // if (tot_emer_valide == 0)
    // {
    //     printf("Nessuna emergenza valida registrata\n\n");
    // }
    // else
    // {
    //     for (int i = 0; i < tot_emer_valide; i++)
    //     {
    //         // Converti timestamp in data/ora leggibile
    //         char time_str[30];
    //         struct tm *timeinfo = localtime(&emergenze_valide[i].time);
    //         strftime(time_str, sizeof(time_str), "%d-%m-%Y %H:%M:%S", timeinfo);

    //         // Ottieni descrizione dello stato
    //         char *status_desc;
    //         switch (emergenze_valide[i].status)
    //         {
    //         case WAITING:
    //             status_desc = "In attesa";
    //             break;
    //         case ASSIGNED:
    //             status_desc = "Assegnata";
    //             break;
    //         case IN_PROGRESS:
    //             status_desc = "In corso";
    //             break;
    //         case COMPLETED:
    //             status_desc = "Completata";
    //             break;
    //         case CANCELLED:
    //             status_desc = "Annullata";
    //             break;
    //         default:
    //             status_desc = "Sconosciuto";
    //             break;
    //         }

    //         printf("\n--- EMERGENZA #%d ---\n", i + 1);
    //         printf("Tipo: %s\n", emergenze_valide[i].type.emergency_desc);
    //         printf("Stato: %s\n", status_desc);
    //         printf("Priorità: %d\n", emergenze_valide[i].type.priority);
    //         printf("Coordinate: (%d, %d)\n", emergenze_valide[i].x, emergenze_valide[i].y);
    //         printf("Timestamp: %s\n", time_str);
    //         printf("Soccorritori: %d\n", emergenze_valide[i].rescuer_count);

    //         if (emergenze_valide[i].rescuers_dt != NULL)
    //         {
    //             printf("MEZZI DI SOCCORSO:\n");
    //             for (int j = 0; j < emergenze_valide[i].rescuer_count; j++)
    //             {
    //                 printf("  %d. %s\n", j + 1, emergenze_valide[i].rescuers_dt[j].rescuer->rescuer_type_name);
    //             }
    //         }
    //         else
    //         {
    //             printf("Nessun soccorritore\n");
    //         }
    //         printf("---------------------------\n\n");
    //     }
    // }
    // printf("=== FINE REPORT ===\n\n");

    for (int i = 0; i < tot_emer_valide; i++)
    {
        free(emergenze_valide[i].rescuers_dt);
    }
    free(emergenze_valide);

    mq_close(mq);
    mq_unlink(n_queue);
    return 0;
}