#include "types.h"
#include "emergency.h"
#include "env.h"

#define QUEUE_NAME "/emergenze671860"
#define MAX_SIZE 1024
#define MSG_STOP "fileEnd"

int client(void)
{
    mqd_t mq;
    char buffer[MAX_SIZE];
    srand(time(NULL));

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = tot_emergenze;
    attr.mq_msgsize = MAX_SIZE;
    attr.mq_curmsgs = 0; // mi definisco gli attibuti necessari per la coda

    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0644, &attr);
    if (mq == -1)
    {
        perror("⚠️Errore nell'apertura della message queue ⚠️");
        exit(1);
    }

    int minX = 10, maxX = width, minY = 10, maxY = height;
    int stepX = (maxX - minX) / 10 + 1;
    int stepY = (maxY - minY) / 10 + 1; // con queste due variabili mi calcolo quanti possibili valori può assumere la x e la y

    for (int i = 0; i < tot_emergenze; i++)
    {
        int coord_x = minX + (rand() % stepX) * 10;
        int coord_y = minY + (rand() % stepY) * 10; // adesso mi genero dei numeri casuali per asseganre delle coordinate alle emergenze
        time_t delay_in_secs = time(NULL);

        snprintf(buffer, MAX_SIZE, "%s %d %d %ld", emergenze_totali[i].emergency_desc, coord_x, coord_y, delay_in_secs);

        if (mq_send(mq, buffer, strlen(buffer) + 1, 0) == -1)
        {
            perror("🛑 Messaggio non inviato nella mq 🛑");
            exit(1);
        }

        printf("\x1b[32m✅ Messaggio inviato:\x1b[0m ");
        printf("\x1b[36m[Emergenza]\x1b[0m \x1b[35m%s\x1b[0m ", emergenze_totali[i].emergency_desc);
        printf("\x1b[36m[Coord]\x1b[0m \x1b[33m(%d,%d)\x1b[0m ", coord_x, coord_y);
        printf("\x1b[36m[Timestamp]\x1b[0m \x1b[34m%ld\x1b[0m\n", delay_in_secs);
    }
    
    if(mq_close(mq) == -1){
        perror("⚠️Errore chiusura coda⚠️");
        exit(1);
    }

     if (mq_unlink(QUEUE_NAME) == -1) {
        perror("⚠️Errore rimozione coda⚠️");
        exit(1);
    }
    return 0;
}