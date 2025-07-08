#include "types.h"
#include "log.h"

#define FILENAME "env.conf"
#define BUFF_DIM 128

char n_queue[BUFF_DIM];
int height;
int width;

int parse_env(void)
{
    int fd;
    SCALL(fd, open(FILENAME, O_RDONLY), "⚠️ Non sono riuscito ad asprire il file di configurazione ⚠️");
    scrivi_log("PARSE_ENV", "FILE_PARSING", "File aperto");
    char buffer[BUFF_DIM];
    ssize_t bytesRead;

    SCALLREAD(
        bytesRead, read(fd, buffer, BUFF_DIM),
        {
            int i = 0;
            char appoggio[BUFF_DIM];
            int selettore = 0;
            while (i < bytesRead)
            {
                while (buffer[i] != '=' && i < bytesRead)
                {
                    i++;
                }
                if (buffer[i] == '=')
                {
                    i++;
                    int iC = 0;
                    while (buffer[i] != '\n' && i < bytesRead)
                    {
                        appoggio[iC++] = buffer[i++];
                    }
                    appoggio[iC] = '\0';
                    i++;
                }
                else
                    i++;
                switch (selettore)
                {
                case 0:
                    strcpy(n_queue, "/");     
                    strcat(n_queue, appoggio); // in questo modo mi creo direttamente la varibile per dare il nome alla queue
                    break;
                case 1:
                    height = atoi(appoggio);
                    break;
                case 2:
                    width = atoi(appoggio);
                    break;
                default:
                    break;
                }
                selettore++;
            }
            char log_msg[BUFF_DIM];
            snprintf(log_msg, sizeof(log_msg),"Descrizione ambiente Nome: %.64s | Height: %d | Width: %d", n_queue, height, width);
            scrivi_log("PARSE_ENV", "FILE_PARSING", log_msg);
        },
        "⚠️ Errore durante la lettura del file ⚠️");

    SCALL(fd, close(fd), "⚠️ Errore durante la chiusura del file ⚠️");
    scrivi_log("PARSE_ENV", "FILE_PARSING", "File chiuso");
    return 0;
}