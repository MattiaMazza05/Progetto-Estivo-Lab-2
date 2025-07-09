#include "types.h"
#include "emergency.h"
#include "env.h"

#define MAX_SIZE 1024
#define MSG_STOP "fileEnd"
#define BUFF_DIM 4096

int parse_env(void);

int main(int argc, char *argv[])
{
    parse_env();
    mqd_t mq;
    char buffer[MAX_SIZE];
    switch (argc)
    {
    case 5: // emergenza da terminale
    {
        long int delay = strtol(argv[4], NULL, 10); // questo mi serve per convertire il char in long int
        snprintf(buffer, MAX_SIZE, "%s %d %d %ld", argv[1], atoi(argv[2]), atoi(argv[3]), delay);
        // apro mq
        mq = mq_open(n_queue, O_RDWR, 0644);
        // mando
        if (mq_send(mq, buffer, strlen(buffer) + 1, 0) == -1)
        {
            PERROR_EXIT("🛑 Messaggio non inviato nella mq 🛑");
        }
        if (mq_close(mq) == -1)
        {
            PERROR_EXIT("⚠️ Errore chiusura coda ⚠️");
        } // definita nella macro
        break;
    }
    case 3: // emergenza da file
    {
        int fd;
        SCALL(fd, open(argv[2], O_RDONLY), "⚠️ Non sono riuscito ad aprire il file contentente le emergenze ⚠️");
        char buffer[BUFF_DIM];
        ssize_t bytesRead;
        SCALLREAD(bytesRead, read(fd, buffer, BUFF_DIM), {
            int i = 0;
            int iC;
            char nEmer[128];
            char coordX[16];
            char coordY[16];
            char delay_in_sec[21]; //metto 21 così sono sicuro che può contenere qualsiasi tipo di long int
            while (i < bytesRead) {
                while(buffer[i] != '\n' && i<bytesRead){
                    ESTRAI_EMERGENZA(nEmer);
                    ESTRAI_EMERGENZA(coordX);
                    ESTRAI_EMERGENZA(coordY); 
                    ESTRAI_EMERGENZA(delay_in_sec);

                    long int delay = strtol(delay_in_sec, NULL, 10); 
                    snprintf(buffer, MAX_SIZE, "%s %d %d %ld", nEmer, atoi(coordX), atoi(coordY), delay);

                    mq = mq_open(n_queue, O_RDWR, 0644);

                    if (mq_send(mq, buffer, strlen(buffer) + 1, 0) == -1)
                    {
                        PERROR_EXIT("🛑 Messaggio non inviato nella mq 🛑");
                    }
                    if (mq_close(mq) == -1)
                    {
                        PERROR_EXIT("⚠️ Errore chiusura coda ⚠️");
                    }
                }
            } }, "⚠️ Errore nella chiusura del file contente le emergenze ⚠️");
        break;
    }
    case 2: // exit
        if (strcmp("exit", argv[1]) == 0)
        {
            mq = mq_open(n_queue, O_RDWR, 0644);
            if (mq == -1)
            {
                PERROR_EXIT("⚠️ Errore nell'apertura della message queue ⚠️");
            }
            if (mq_send(mq, MSG_STOP, strlen(MSG_STOP) + 1, 0) == -1)
            {
                perror("🛑 Messaggio di stop non inviato nella mq 🛑");
                if (mq_close(mq) == -1)
                {
                    PERROR_EXIT("⚠️ Errore chiusura coda ⚠️");
                }
            }
            if (mq_close(mq) == -1)
            {
                PERROR_EXIT("⚠️ Errore chiusura coda ⚠️");
            }
            break;
        }
        printf("Il messaggio non è valido, digita ./client per avere le istruzioni\n");
        break;
    }
    return 0;
}