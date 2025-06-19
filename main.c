#include "types.h"

int parse_emergency_types(void);
int parse_rescuers(void);
int parse_env(void);

int main() {
    parse_rescuers();
    parse_emergency_types();
    parse_env();
    return 0;
}