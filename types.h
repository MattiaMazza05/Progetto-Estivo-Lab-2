#ifndef TYPES // mi assicuro che non sia già stata creata la struttura.
#define TYPES
#include "macro.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <threads.h>
#include <stdbool.h>
#include <stdarg.h>

#define EMERGENCY_NAME_LENGTH  64

#define PRIORITY_0 0
#define PRIORITY_1 30
#define PRIORITY_2 10

#define MAX_MSG_Q 10

#define LOG_FILE "progetto_estivo.log"
#define MAX_LOG_MSG 1024
typedef enum
{
    IDLE,
    EN_ROUTE_TO_SCENE,
    ON_SCENE,
    RETURNING_TO_BASE
} rescuers_status_t;

typedef struct
{
    char *rescuer_type_name;
    int speed;
    int x;
    int y;
} rescuer_type_t;

typedef struct
{
    int id;
    int x;
    int y;
    rescuer_type_t *rescuer;
    rescuers_status_t status;
} rescuer_digital_twin_t;

typedef struct
{
    rescuer_type_t* type;
    int required_count;
    int time_to_manage;
}rescuer_request_t;


typedef struct{
    short priority;
    char* emergency_desc;
    rescuer_request_t* rescuers;
    int rescuers_req_number;
}emergency_type_t;

typedef enum {
    WAITING, ASSIGNED, IN_PROGRESS, PAUSED, COMPLETED, CANCELLED, TIMEOUT
} emergency_status_t;

typedef struct {
    char emergency_name[EMERGENCY_NAME_LENGTH];
    int x;
    int y;
    time_t timestamp;
}emergency_request_t;

typedef struct {
    emergency_type_t type;
    emergency_status_t status;
    int x;
    int y;
    time_t time;
    int rescuer_count;
    rescuer_digital_twin_t *rescuers_dt;
}emergency_t;

#define DIMENSIONE_CODA_THREAD 10
typedef struct{
    thrd_t thread;
    bool occupato;
    int emer_id;
    bool terminato;
} thread_slot_t;

#define DIMENSIONE_THREAD_ATTESA 100 //stessa dimensione della mq

#endif