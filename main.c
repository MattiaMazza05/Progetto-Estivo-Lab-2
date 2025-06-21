#include "types.h"

int parse_emergency_types(void);
int parse_rescuers(void);
int parse_env(void);
int client(void);

int main() {
    parse_rescuers();
    parse_emergency_types();
    parse_env();
    client();
    return 0;
}