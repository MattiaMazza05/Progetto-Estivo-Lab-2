#include "types.h"

int parse_emergency_types(void);
int parse_rescuers(void);

int main() {
    parse_rescuers();
    parse_emergency_types();
    return 0;
}