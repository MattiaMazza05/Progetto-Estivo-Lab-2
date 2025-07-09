#include "types.h"
#include "rescuers.h"
#include "log.h"
#define FILENAME "rescuers.conf"
#define BUFF_DIM 128

rescuer_digital_twin_t *mezzi_totali = NULL; // creo un array generale dove andrò ad inserire tramite allocazione dinamica di memoria tutti i mezzi che leggo dal file .conf
int tot_mezzi = 0;                           // questo contatore mi serve per l'allocazione di memoria

int parse_rescuers(void)
{
    int fd_conf;
    SCALL(fd_conf, open(FILENAME, O_RDONLY), "⚠️ Non sono riuscito ad aprire il file di configurazione ⚠️");
    scrivi_log("PARSE_RESCUER", "FILE_PARSING", "File aperto");

    char buffer[BUFF_DIM];
    ssize_t bytesRead;

    SCALLREAD(bytesRead, read(fd_conf, buffer, BUFF_DIM), {
        int i = 0;
        char nome[128];
        char nV[16]; // nV (numeroVeicoli) indice che mi serve per riempire i char di appoggio
        char v[16];  // velocità
        char cX[16];
        char cY[16];
        int nVeicoli;
        int cS; // caselle al Secondo
        int x;
        int y;
        while (i < bytesRead)
        {
            if (buffer[i] == '[')
            {
                i++;
                int iC = 0; // iC (indiceChar) è l'indice che mi serve per riempire i char di appoggio
                ESTRAIDATO(nome)
                nome[iC] = '\0';
                iC = 0; // azzero il contatore in modo da poterlo riutilizzare per gli altri dati
                FINOAPROXDATO
                ESTRAIDATO(nV)
                nV[iC] = '\0';
                nVeicoli = atoi(nV);
                iC = 0;
                FINOAPROXDATO
                ESTRAIDATO(v);
                v[iC] = '\0';
                cS = atoi(v); // converto il char in int
                iC = 0;
                FINOAPROXDATO
                while (buffer[i] != ';' && i < bytesRead)
                {
                    cX[iC++] = buffer[i++];
                }
                cX[iC] = '\0';
                x = atoi(cX);
                iC = 0;
                i++; //incremento in modo tale da esculedere la ','
                ESTRAIDATO(cY)
                cY[iC] = '\0';
                y = atoi(cY);
                iC = 0;
                rescuer_type_t *rescuer = malloc(sizeof(rescuer_type_t)); //alloco memoria dinamica per la struct rescuer_type_t che contiene le informazioni del veicolo
                rescuer_digital_twin_t *tmp = realloc(mezzi_totali, sizeof(rescuer_digital_twin_t) * (tot_mezzi + nVeicoli));//eseguo un controllo per assicurarmi che l'allocazione sia andata a buon fine
                if (tmp == NULL) {
                    scrivi_log("PARSE_RESCUER", "FILE_PARSING", "Errore durante assegnazione memoria");
                    free(mezzi_totali);
                    PERROR_EXIT("⚠️ Errore durante l'assegnazione di memoria ⚠️");
                }
                mezzi_totali = tmp;
                rescuer->speed = cS;
                rescuer->x = x;
                rescuer->y = y;
                rescuer->rescuer_type_name = malloc(strlen(nome)+1); //alloco memoria dinamica in base al nome, il +1 mi serve per allocare memoria anche per lo \0
                strcpy(rescuer->rescuer_type_name, nome);//ora copio nome dentro la struct


                for (int i = 0; i < nVeicoli; i++)
                {
                    mezzi_totali[tot_mezzi + i].id = 1001 + tot_mezzi + i; 
                    mezzi_totali[tot_mezzi +i].x = x;
                    mezzi_totali[tot_mezzi +i].y = y;
                    mezzi_totali[tot_mezzi +i].status = IDLE;
                    mezzi_totali[tot_mezzi +i].rescuer = rescuer;

                    char id_rescuer[16];
                    char log_msg[256];
                    snprintf(id_rescuer, sizeof(id_rescuer), "%d", mezzi_totali[tot_mezzi + i].id);
                    snprintf(log_msg, sizeof(log_msg),"Soccorritore: %s | Base: [%d,%d] | Velocità: %d | Stato: %d",
                        mezzi_totali[tot_mezzi + i].rescuer->rescuer_type_name,
                        mezzi_totali[tot_mezzi + i].x,
                        mezzi_totali[tot_mezzi + i].y,
                        mezzi_totali[tot_mezzi + i].rescuer->speed,
                        mezzi_totali[tot_mezzi + i].status
                    );
                    scrivi_log(id_rescuer, "FILE_PARSING", log_msg);
                }
                
                tot_mezzi += nVeicoli;
            }else i++;
        } }, "⚠️ Errore durante la lettura del file");
    SCALL(fd_conf, close(fd_conf), "⚠️ Errore durante la chiusura del file di CONFIGURAZIONE ⚠️");
    scrivi_log("PARSE_RESCUER", "FILE_PARSING", "File chiuso");
    return 0;
}

void free_rescuers(void){
    rescuer_type_t *liberi[tot_mezzi];
    int n_liberi = 0;
    for (int i = 0; i < tot_mezzi; i++) {
        bool liberato = false;
        for (int j = 0; j < n_liberi; j++) {
            if (liberi[j] == mezzi_totali[i].rescuer) {
                liberato = true;
                break;
            }
        }
        if(!liberato){
            free(mezzi_totali[i].rescuer->rescuer_type_name);
            free(mezzi_totali[i].rescuer);
            liberi[n_liberi++] = mezzi_totali[i].rescuer;
        }
    }
    free(mezzi_totali);
}