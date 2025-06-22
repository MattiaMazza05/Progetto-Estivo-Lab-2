#include "types.h"
#include "rescuers.h"
#define FILENAME "rescuers.conf"
#define BUFF_DIM 128

rescuer_digital_twin_t *mezzi_totali = NULL; // creo un array generale dove andrò ad inserire tramite allocazione dinamica di memoria tutti i mezzi che leggo dal file .conf
int tot_mezzi = 0;                           // questo contatore mi serve per l'allocazione di memoria

int parse_rescuers(void)
{
    int fd;
    SCALL(fd, open(FILENAME, O_RDONLY), "⚠️ Non sono riuscito ad aprire il file di configurazione ⚠️");

    char buffer[BUFF_DIM];
    ssize_t bytesRead;

    SCALLREAD(bytesRead, read(fd, buffer, BUFF_DIM), {
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
                    perror("⚠️ Errore durante l'assegnazione di memoria ⚠️");
                    free(mezzi_totali);
                    exit(EXIT_FAILURE); 
                }
                mezzi_totali = tmp;
                rescuer->speed = cS;
                rescuer->x = x;
                rescuer->y = y;
                rescuer->rescuer_type_name = malloc(strlen(nome)+1); //alloco memoria dinamica in base al nome, il +1 mi serve per allocare memoria anche per lo \0
                strcpy(rescuer->rescuer_type_name, nome);//ora copio nome dentro la struct
                // printf("Il veicolo è: \x1b[35m%s\x1b[0m ci sono \x1b[35m%d\x1b[0m veicoli totali con una velocità di \x1b[35m%d\x1b[0m caselle al secondo e la posizione della base è: \x1b[35m[%d;%d]\x1b[0m\n", rescuer->rescuer_type_name, nVeicoli, rescuer->speed, rescuer->x, rescuer->y);


                for (int i = 0; i < nVeicoli; i++)
                {
                    mezzi_totali[tot_mezzi + i].id = 1001 + tot_mezzi + i; 
                    mezzi_totali[tot_mezzi +i].x = x;
                    mezzi_totali[tot_mezzi +i].y = y;
                    mezzi_totali[tot_mezzi +i].status = IDLE;
                    mezzi_totali[tot_mezzi +i].rescuer = rescuer;
                }
                
                tot_mezzi += nVeicoli;
            }else i++;
        } }, "⚠️ Errore durante la lettura del file");
    SCALL(fd, close(fd), "⚠️ Errore durante la chiusura del file");

    // printf("\n\x1b[32m========== VERIFICA MEZZI TOTALI ==========\x1b[0m\n\n");
    // printf("\x1b[36m%-4s %-12s %-12s %-12s %-10s %-8s\x1b[0m\n", "ID", "Tipo", "Posizione", "Velocità", "Stato", "Coord");
    // for (int i = 0; i < tot_mezzi; i++)
    // {
    //     printf("%-4d %-12s [%2d,%2d]     %-12d %-10d [%2d,%2d]\n",
    //            mezzi_totali[i].id,
    //            mezzi_totali[i].rescuer->rescuer_type_name,
    //            mezzi_totali[i].x, mezzi_totali[i].y,
    //            mezzi_totali[i].rescuer->speed,
    //            mezzi_totali[i].status,
    //            mezzi_totali[i].rescuer->x, mezzi_totali[i].rescuer->y);
    // }
    // printf("\n\x1b[32m========== FINE VERIFICA MEZZI ==========\x1b[0m\n");
    return 0;
}