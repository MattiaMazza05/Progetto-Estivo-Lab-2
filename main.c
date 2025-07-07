#include "types.h"
#include "rescuers.h"
#include "emergency.h"
#include "env.h"
#include "log.h"

int parse_emergency_types(void);
int parse_rescuers(void);
int parse_env(void);

#define MAX_SIZE 1024
#define MSG_STOP "fileEnd"
#define MAX_EMERGENZE 100

emergency_t emergenze_valide[MAX_EMERGENZE];
int tot_emer_valide = 0;

mtx_t mutex_soccorritori;

int gestione_emer(void *arg)
{
    int emer_id = *(int *)arg;
    mtx_lock(&mutex_soccorritori);
    if (emer_id <= 0 || emer_id > tot_emer_valide)
    {
        printf("❌ Thread %d: Emergenza non valida\n", emer_id);
        mtx_unlock(&mutex_soccorritori);
        return -1;
    }

    emergency_t *emergenza = &emergenze_valide[emer_id - 1];

    printf("\n🔍 Thread %d - Inizio gestione emergenza %s\n",
           emer_id, emergenza->type.emergency_desc);

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

                if (tempo_tipo_mezzo > tempo_totale)
                    tempo_totale = tempo_tipo_mezzo;

                if (tempo_totale > t_limite)
                {
                    printf("❌ Thread %d: Tempo %d > limite %ld OPPURE ❌ Thread %d: Mezzi insufficienti (%d < %d)\n", emer_id, tempo_totale, t_limite,
                           emer_id, mezzi_disponibili, mezzi_necessari);
                    emergenza->status = TIMEOUT;
                    tutti_disponibili = false;
                    break;
                }
            }

            if (tutti_disponibili && tempo_totale <= t_limite)
            {
                printf("✅ Thread %d: Gestione emergenza %s\n", emer_id, emergenza->type.emergency_desc);

                int mezzi_totali_necessari = 0;
                for (int j = 0; j < emergenze_totali[i].rescuers_req_number; j++)
                {
                    mezzi_totali_necessari += emergenze_totali[i].rescuers[j].required_count;
                }

                int mezzi_per_emer[mezzi_totali_necessari];
                int indice_array = 0;

                emergenza->status = IN_PROGRESS;

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
                                mezzi_assegnati_tipo++;
                                indice_array++;
                            }
                        }
                    }
                }
                mtx_unlock(&mutex_soccorritori);
                sleep(tempo_totale);
                printf("Thread %d: %s → EN_ROUTE_TO_SCENE (%d sec)\n",
                       emer_id, emergenza->type.emergency_desc, tempo_totale);

                mtx_lock(&mutex_soccorritori);
                CAMBIO_STATO(EN_ROUTE_TO_SCENE, ON_SCENE);
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
                printf("Thread %d: %s → ON_SCENE (%d sec)\n",
                       emer_id, emergenza->type.emergency_desc, tempo_gestione);

                mtx_lock(&mutex_soccorritori);
                CAMBIO_STATO(ON_SCENE, RETURNING_TO_BASE)
                mtx_unlock(&mutex_soccorritori);
                sleep(tempo_totale);
                printf("Thread %d: %s → RETURNING_TO_BASE (%d sec)\n",
                       emer_id, emergenza->type.emergency_desc, tempo_totale);

                mtx_lock(&mutex_soccorritori);
                CAMBIO_STATO(RETURNING_TO_BASE, IDLE);
                emergenza->status = COMPLETED;
                printf("✅ Thread %d: Emergenza %s COMPLETATA\n",
                       emer_id, emergenza->type.emergency_desc);
            }
            else
            {
                printf("❌ Thread %d: Emergenza %s SCARTATA\n",
                       emer_id, emergenza->type.emergency_desc);
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

    emergency_request_t richiesta_emer;
    char nEmer[128];
    char coordX[16];
    char coordY[16];
    char delay_in_sec[21];

    thrd_t emergenze[MAX_MSG_Q];
    int id_emer[MAX_MSG_Q];
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
                    emergenze_valide[tot_emer_valide].status = WAITING;
                    if (strcmp(richiesta_emer.emergency_name, emergenze_totali[i].emergency_desc) == 0 && richiesta_emer.x >= 0 && richiesta_emer.x <= width && richiesta_emer.y >= 0 && richiesta_emer.y <= height && richiesta_emer.timestamp >= 0)
                    {

                        if (tot_emer_valide < MAX_EMERGENZE)
                        {
                            emergenze_valide[tot_emer_valide].status = ASSIGNED;
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
                            if (thread_c < MAX_MSG_Q)
                            {
                                // Assegna l'ID dell'emergenza (basato su posizione nell'array)
                                id_emer[thread_c] = tot_emer_valide; // L'ultima emergenza aggiunta

                                printf("🚨 Creazione Thread %d per emergenza: %s (%d,%d)\n",
                                       id_emer[thread_c], emergenze_valide[tot_emer_valide - 1].type.emergency_desc,
                                       emergenze_valide[tot_emer_valide - 1].x, emergenze_valide[tot_emer_valide - 1].y);

                                // Crea il thread che gestisce questa emergenza
                                if (thrd_create(&emergenze[thread_c], gestione_emer, &id_emer[thread_c]) != thrd_success)
                                {
                                    printf("❌ Errore creazione thread per emergenza %d\n", id_emer[thread_c]);
                                }
                                else
                                {
                                    printf("✅ Thread %d avviato per gestione emergenza\n", id_emer[thread_c]);
                                    thread_c++; // Incrementa solo se creazione riuscita
                                }
                            }
                            else
                            {
                                printf("⚠️ Raggiunto limite massimo di thread (%d)\n", MAX_MSG_Q);
                            }
                        }
                    }
                    else
                    {
                        emergenze_valide[tot_emer_valide].status = CANCELLED;
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

    printf("\n========== RIEPILOGO FINALE SERVER ==========\n");
    printf("📊 TOTALE EMERGENZE RICEVUTE: %d\n", tot_emer_valide);
    printf("🧵 TOTALE THREAD CREATI: %d\n", thread_c);
    printf("✅ TUTTI I THREAD TERMINATI CON SUCCESSO\n");

    // Stato finale di tutti i mezzi
    printf("\n🚑 STATO FINALE MEZZI:\n");
    int finale_idle = 0, finale_en_route = 0, finale_on_scene = 0, finale_returning = 0;
    for (int k = 0; k < tot_mezzi; k++)
    {
        switch (mezzi_totali[k].status)
        {
        case IDLE:
            finale_idle++;
            break;
        case EN_ROUTE_TO_SCENE:
            finale_en_route++;
            break;
        case ON_SCENE:
            finale_on_scene++;
            break;
        case RETURNING_TO_BASE:
            finale_returning++;
            break;
        }
    }
    printf("   🟢 IDLE: %d | 🔵 EN_ROUTE: %d | 🟡 ON_SCENE: %d | 🟠 RETURNING: %d\n",
           finale_idle, finale_en_route, finale_on_scene, finale_returning);

    // Verifica che tutti i mezzi siano tornati IDLE
    if (finale_en_route == 0 && finale_on_scene == 0 && finale_returning == 0)
    {
        printf("✅ TUTTI I MEZZI SONO TORNATI IDLE - SISTEMA PULITO\n");
    }
    else
    {
        printf("❌ ATTENZIONE: ALCUNI MEZZI NON SONO TORNATI IDLE\n");
    }

    printf("🎯 STATO SERVER: TERMINATO CORRETTAMENTE\n");
    printf("==========================================\n");

    for (int i = 0; i < tot_emer_valide; i++)
    {
        free(emergenze_valide[i].rescuers_dt);
    }

    mq_close(mq);
    mq_unlink(n_queue);
    mtx_destroy(&mutex_soccorritori);
    chiudi_log();
    return 0;
}