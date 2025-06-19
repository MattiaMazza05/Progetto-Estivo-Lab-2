#include "types.h"

#define FILENAME "env.conf"
#define BUFF_DIM 256

int parse_env(void)
{
    int fd;
    SCALL(fd, open(FILENAME, O_RDONLY), "⚠️Non sono riuscito ad asprire il file di configurazione ⚠️");

    char buffer[BUFF_DIM];
    ssize_t bytesRead;

    char n_queue[BUFF_DIM];
    int height;
    int width;
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
                    strcpy(n_queue, appoggio);
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
        },
        "⚠️ Errore durante la lettura del file ⚠️");

    SCALL(fd, close(fd), "⚠️ Errore durante la chiusura del file ⚠️");

    printf("\n\x1b[32m========== VERIFICA DATI ENV ==========\x1b[0m\n\n");
    printf("\x1b[36m%-20s %-10s %-10s\x1b[0m\n", "Nome coda", "Altezza", "Larghezza");
    printf("%-20s %-10d %-10d\n", n_queue, height, width);
    printf("\n\x1b[32m========== FINE VERIFICA ENV ==========\x1b[0m\n");

    return 0;
}