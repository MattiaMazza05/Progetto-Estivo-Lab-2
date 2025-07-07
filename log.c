#include "types.h"
#include "macro.h"
int fd_log;
mtx_t log_mutex;

void crea_log(const char *filename)
{
    SCALL(fd_log, open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644), "⚠️ Non sono riuscito ad aprire il file di LOG ⚠️");
    if (mtx_init(&log_mutex, mtx_plain) != thrd_success)
    {
        PERROR_EXIT("Errore durante la creazione del mutex");
    }
}

void scrivi_log(const char *id, const char *event, const char *message)
{
    char buffer[1024];
    time_t now = time(NULL);
    if (!id)
        id = "N/A";

    int riga = snprintf(buffer, sizeof(buffer), "[%ld] [%s] [%s] %s\n", now, id, event, message);

    mtx_lock(&log_mutex);
    SCALL(fd_log, write(fd_log, buffer, riga), "⚠️ Errore durante la scrittura nel file LOG ⚠️");
    mtx_unlock(&log_mutex);
}

void chiudi_log(void)
{
   SCALL(fd_log, close(fd_log), "⚠️ Errore durante la chiusura ⚠️");
    mtx_destroy(&log_mutex);
}