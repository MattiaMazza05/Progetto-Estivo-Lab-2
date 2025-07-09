#ifndef LOGGER_H
#define LOGGER_H

#include <threads.h>
#include "types.h"
void crea_log(const char *filename);
void scrivi_log(const char *id, const char *event, const char *message);
void chiudi_log(void);

#endif