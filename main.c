#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "ass/cpu/register.h"

int main() {
    Reg r;
    r.rax = 0x1234abcd5678fffe;
    printf("%x", r.eax);
    return 0;
}
