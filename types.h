#ifndef TYPES // mi assicuro che non sia già stata creata la struttura.
#define TYPES
#include "macro.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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


#endif