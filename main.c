#include "types.h"
#include "rescuers.h"
#include "emergency.h"
#include "env.h"
#include "log.h"
#include "gestione_thread.h"

int parse_env(void);
int parse_rescuers(void);
int parse_emergency_types(void);

#define MAX_SIZE 1024
#define MSG_STOP "fileEnd"
#define MAX_EMERGENZE 100

emergency_t emergenze_valide[MAX_EMERGENZE];
int tot_emer_valide = 0;
mtx_t mutex_soccorritori;

int gestione_emer(void *arg)
{
    thread_slot_t *slot = (thread_slot_t *)arg;
    int emer_id = slot->emer_id;
    char log_msg[256];
    char id_str[16];
    char id_socc[16];
    time_t now;

    if (!lock(&mutex_soccorritori))
    {
        snprintf(id_str, sizeof(id_str), "%d", slot->emer_id);
        snprintf(log_msg, sizeof(log_msg), "Impossibile acquisire lock per emergenza %d",
                 slot->emer_id);
        scrivi_log(id_str, "TIMEOUT", log_msg);
        return -1;
    }
    if (emer_id < 0 || emer_id >= tot_emer_valide)
    {
        snprintf(id_str, sizeof(id_str), "%d", slot->emer_id);
        now = time(NULL);
        snprintf(log_msg, sizeof(log_msg), "Emergenza NON VALIDA | Time: %ld", now);
        scrivi_log(id_str, "EMERGENCY_STATUS", log_msg);
        mtx_unlock(&mutex_soccorritori);
        return -1;
    }

    emergency_t *emergenza = &emergenze_valide[emer_id];

    // Calcolo il tempo massimo in base alla priorità
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

    for (int i = 0; i < tot_emergenze; i++)
    {
        if (strcmp(emergenza->type.emergency_desc, emergenze_totali[i].emergency_desc) == 0)
        {
            bool tutti_disponibili = true;
            int tempo_totale = 0;

            for (int j = 0; j < emergenze_totali[i].rescuers_req_number; j++)
            {
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

                if (tempo_tipo_mezzo > tempo_totale)
                    tempo_totale = tempo_tipo_mezzo;

                if (tempo_totale > t_limite)
                {
                    emergenza->status = TIMEOUT;
                    snprintf(id_str, sizeof(id_str), "%d", slot->emer_id);
                    now = time(NULL);
                    snprintf(log_msg, sizeof(log_msg), "Emergenza: %s | Stato %d (TIMEOUT) | Time: %ld",
                             emergenza->type.emergency_desc, emergenza->status, now);
                    scrivi_log(id_str, "EMERGENCY_STATUS", log_msg);
                    tutti_disponibili = false;
                    break;
                }
            }

            if (tutti_disponibili && tempo_totale <= t_limite)
            {
                int mezzi_totali_necessari = 0;
                for (int j = 0; j < emergenze_totali[i].rescuers_req_number; j++)
                {
                    mezzi_totali_necessari += emergenze_totali[i].rescuers[j].required_count;
                }

                int mezzi_per_emer[mezzi_totali_necessari];
                int indice_array = 0;

                emergenza->status = IN_PROGRESS;
                snprintf(id_str, sizeof(id_str), "%d", slot->emer_id);
                now = time(NULL);
                snprintf(log_msg, sizeof(log_msg), "Emergenza: %s | Stato %d | Time: %ld",
                         emergenza->type.emergency_desc, emergenza->status, now);
                scrivi_log(id_str, "EMERGENCY_STATUS", log_msg);

                for (int j = 0; j < emergenze_totali[i].rescuers_req_number; j++)
                {
                    int mezzi_necessari = emergenze_totali[i].rescuers[j].required_count;
                    int mezzi_assegnati_tipo = 0;

                    for (int k = 0; k < tot_mezzi && mezzi_assegnati_tipo < mezzi_necessari; k++)
                    {
                        if (strcmp(emergenze_totali[i].rescuers[j].type->rescuer_type_name,
                                   mezzi_totali[k].rescuer->rescuer_type_name) == 0)
                        {
                            if (mezzi_totali[k].status == IDLE)
                            {
                                mezzi_per_emer[indice_array] = mezzi_totali[k].id;
                                mezzi_totali[k].status = EN_ROUTE_TO_SCENE;
                                now = time(NULL);
                                snprintf(id_socc, sizeof(id_socc), "%d", mezzi_totali[k].id);
                                snprintf(log_msg, sizeof(log_msg), "Soccorritore: %s | Stato %d | Time: %ld",
                                         mezzi_totali[k].rescuer->rescuer_type_name, mezzi_totali[k].status, now);
                                scrivi_log(id_socc, "RESCUER_STATUS", log_msg);
                                now = time(NULL);
                                snprintf(log_msg, sizeof(log_msg), "Assegnato: %s | Emergenza: %s | Time: %ld",
                                         mezzi_totali[k].rescuer->rescuer_type_name, emergenza->type.emergency_desc, now);
                                scrivi_log(id_str, "RESCUER_STATUS", log_msg);
                                mezzi_assegnati_tipo++;
                                indice_array++;
                            }
                        }
                    }
                }
                mtx_unlock(&mutex_soccorritori);
                sleep(tempo_totale);

                if (!lock(&mutex_soccorritori))
                {
                    snprintf(id_str, sizeof(id_str), "%d", slot->emer_id);
                    snprintf(log_msg, sizeof(log_msg), "Impossibile acquisire lock per emergenza %d",
                             slot->emer_id);
                    scrivi_log(id_str, "TIMEOUT", log_msg);
                    return -1;
                }
                now = time(NULL);
                CAMBIO_STATO(EN_ROUTE_TO_SCENE, ON_SCENE, now);
                int tempo_gestione = 0;
                for (int j = 0; j < emergenze_totali[i].rescuers_req_number; j++)
                {
                    if (emergenze_totali[i].rescuers[j].time_to_manage > tempo_gestione)
                    {
                        tempo_gestione = emergenze_totali[i].rescuers[j].time_to_manage;
                    }
                }
                mtx_unlock(&mutex_soccorritori);
                sleep(tempo_gestione);

                if (!lock(&mutex_soccorritori))
                {
                    snprintf(id_str, sizeof(id_str), "%d", slot->emer_id);
                    snprintf(log_msg, sizeof(log_msg), "Impossibile acquisire lock per emergenza %d",
                             slot->emer_id);
                    scrivi_log(id_str, "TIMEOUT", log_msg);
                    return -1;
                }
                now = time(NULL);
                CAMBIO_STATO(ON_SCENE, RETURNING_TO_BASE, now)
                mtx_unlock(&mutex_soccorritori);
                sleep(tempo_totale);

                if (!lock(&mutex_soccorritori))
                {
                    snprintf(id_str, sizeof(id_str), "%d", slot->emer_id);
                    snprintf(log_msg, sizeof(log_msg), "Impossibile acquisire lock per emergenza %d",
                             slot->emer_id);
                    scrivi_log(id_str, "TIMEOUT", log_msg);
                    return -1;
                }
                now = time(NULL);
                CAMBIO_STATO(RETURNING_TO_BASE, IDLE, now);
                emergenza->status = COMPLETED;
                now = time(NULL);
                snprintf(log_msg, sizeof(log_msg), "Emergenza: %s | Stato %d | Time: %ld",
                         emergenza->type.emergency_desc, emergenza->status, now);
                scrivi_log(id_str, "EMERGENCY_STATUS", log_msg);

                slot->terminato = true;
            }
            break;
        }
    }

    mtx_unlock(&mutex_soccorritori);
    return 0;
}

int main()
{
    crea_log("progetto_estivo.log");
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
    scrivi_log("main.c", "MESSAGE_QUEUE", "MQ creata e definiti attributi");
    emergency_request_t richiesta_emer;
    char nEmer[128];
    char coordX[16];
    char coordY[16];
    char delay_in_sec[21];

    for (int i = 0; i < DIMENSIONE_CODA_THREAD; i++)
    {
        queue_thread[i].occupato = false;
        queue_thread[i].emer_id = -1;
    }

    mtx_init(&mutex_soccorritori, mtx_timed);

    mq = mq_open(n_queue, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == -1)
    {
        scrivi_log("main.c", "MESSAGE_QUEUE", "Errore apertura MQ");
        PERROR_EXIT("⚠️ Errore nell'apertura della message queue ⚠️");
    }

    scrivi_log("main.c", "MESSAGE_QUEUE", "Lettura messaggi MQ");
    int id_log = 0;

    while (1)
    {
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        if (bytes_read >= 0)
        {
            buffer[bytes_read] = '\0';
            if (strcmp(buffer, MSG_STOP) == 0)
            {
                for (int i = 0; i < DIMENSIONE_CODA_THREAD; i++)
                {
                    if (queue_thread[i].occupato)
                    {
                        thrd_join(queue_thread[i].thread, NULL);
                    }
                }
                scrivi_log("main.c", "MESSAGE_QUEUE", "Messaggio exit ricevuto chiusura server");
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
                int accettata = -1; //-1 default, 0 accettata, 1 non corrisponde nome, 2 coordinate non valide, 3 timestamp non valido
                int id_e;
                id_e = id_log;
                char log_msg[256];
                char id_str[16];
                snprintf(id_str, sizeof(id_str), "%d", id_e);
                emergenze_valide[tot_emer_valide].status = WAITING;
                time_t now = time(NULL);
                snprintf(log_msg, sizeof(log_msg), "Emergenza: %s | Stato %d | Time: %ld",
                         richiesta_emer.emergency_name, emergenze_valide[tot_emer_valide].status, now);
                scrivi_log(id_str, "EMERGENCY_STATUS", log_msg);
                for (int i = 0; i < tot_emergenze; i++)
                {

                    if (strcmp(richiesta_emer.emergency_name, emergenze_totali[i].emergency_desc) == 0)
                    {
                        if (richiesta_emer.x >= 0 && richiesta_emer.x <= width && richiesta_emer.y >= 0 && richiesta_emer.y <= height)
                        {
                            if (richiesta_emer.timestamp >= 0)
                            {

                                accettata = 0;
                                if (tot_emer_valide < MAX_EMERGENZE)
                                {
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
                                    snprintf(log_msg, sizeof(log_msg), "Emergenza ACCETTTA: %s | Coordinate [%d,%d] | Time: %ld",
                                             emergenze_valide[tot_emer_valide].type.emergency_desc, emergenze_valide[tot_emer_valide].x, emergenze_valide[tot_emer_valide].y, emergenze_valide[tot_emer_valide].time);
                                    scrivi_log(id_str, "MESSAGE_QUEUE", log_msg);
                                    emergenze_valide[tot_emer_valide].status = ASSIGNED;
                                    now = time(NULL);
                                    snprintf(log_msg, sizeof(log_msg), "Emergenza: %s | Stato %d | Time: %ld",
                                             richiesta_emer.emergency_name, emergenze_valide[tot_emer_valide].status, now);
                                    scrivi_log(id_str, "EMERGENCY_STATUS", log_msg);

                                    tot_emer_valide++;
                                    id_log++;

                                    check_threads();

                                    int posizione_trovata = -1;
                                    for (int i = 0; i < DIMENSIONE_CODA_THREAD; i++)
                                    {
                                        if (!queue_thread[i].occupato)
                                        {
                                            posizione_trovata = i;
                                            break;
                                        }
                                    }

                                    if (posizione_trovata != -1)
                                    {
                                        queue_thread[posizione_trovata].emer_id = tot_emer_valide - 1;
                                        queue_thread[posizione_trovata].occupato = true;
                                        if (thrd_create(&queue_thread[posizione_trovata].thread, gestione_emer, &queue_thread[posizione_trovata]) == thrd_success)
                                        {
                                            queue_thread[posizione_trovata].terminato = false;
                                        }
                                    }
                                    else
                                    {
                                        inserisci_emer(tot_emer_valide);
                                    }

                                    check_threads();
                                    while (contatore_coda > 0)
                                    {
                                        int posizione_libera = -1;
                                        for (int i = 0; i < DIMENSIONE_CODA_THREAD; i++)
                                        {
                                            if (!queue_thread[i].occupato)
                                            {
                                                posizione_libera = i;
                                                break;
                                            }
                                        }
                                        if (posizione_libera == -1)
                                            break;
                                        int prox_emer = rimuovi_emer();
                                        if (prox_emer != -1)
                                        {
                                            queue_thread[posizione_libera].emer_id = prox_emer;
                                            queue_thread[posizione_libera].occupato = true;
                                            if (thrd_create(&queue_thread[posizione_libera].thread, gestione_emer, &queue_thread[posizione_libera]) == thrd_success)
                                            {
                                                queue_thread[posizione_trovata].terminato = false;
                                            }
                                        }
                                    }
                                }
                                break;
                            }
                            else
                            {
                                accettata = 3;
                            }
                        }
                        else
                        {
                            accettata = 2;
                        }
                    }
                    else
                    {
                        accettata = 1;
                    }
                }
                if (accettata == 1 || accettata == 2 || accettata == 3)
                {
                    char id_str[16];
                    char log_msg[256];
                    snprintf(id_str, sizeof(id_str), "%d", id_e);
                    switch (accettata)
                    {
                    case 1:
                        snprintf(log_msg, sizeof(log_msg), "Emergenza SCARTATA: %s non so gestire questa emergenza nome non trovato",
                                 richiesta_emer.emergency_name);
                        scrivi_log(id_str, "MESSAGE_QUEUE", log_msg);
                        break;
                    case 2:
                        snprintf(log_msg, sizeof(log_msg), "Emergenza SCARTATA: %s coordinate non valide",
                                 richiesta_emer.emergency_name);
                        scrivi_log(id_str, "MESSAGE_QUEUE", log_msg);
                        break;
                    case 3:
                        snprintf(log_msg, sizeof(log_msg), "Emergenza SCARTATA: %s timestamp non valido",
                                 richiesta_emer.emergency_name);
                        scrivi_log(id_str, "MESSAGE_QUEUE", log_msg);
                        break;
                    }

                    id_log++;
                }
            }
        }
        else
        {
            PERROR_BREAK("⚠️ Errore lettura dalla message queue ⚠️");
        }
    }

    for (int i = 0; i < tot_emer_valide; i++)
    {
        free(emergenze_valide[i].rescuers_dt);
    }


    free_emergenze();
    free_rescuers();
    mq_close(mq);
    mq_unlink(n_queue);
    mtx_destroy(&mutex_soccorritori);
    chiudi_log();
    return 0;
}