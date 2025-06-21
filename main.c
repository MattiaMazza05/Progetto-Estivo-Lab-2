#include "types.h"

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

    //printf("%d\n", tot_emergenze);

    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);
    if (mq == -1){PERROR_EXIT("⚠️ Errore nell'apertura della message queue ⚠️");}

    printf("\n--- Lettura messaggi dalla coda ---\n");
    while (1)
    {
        ssize_t bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
        if (bytes_read >= 0)
        {
            buffer[bytes_read] = '\0';
            printf("Messaggio ricevuto: %s\n", buffer);
            if (strcmp(buffer, MSG_STOP) == 0)
            {
                printf("--- Fine messaggi ---\n");
                break;
            }
        }
        else {PERROR_BREAK("⚠️ Errore lettura dalla message queue ⚠️");}
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME); // opzionale: elimina la coda
    return 0;
}