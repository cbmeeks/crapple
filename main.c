#include <stdio.h>
#include "crapple.c"

int main(void) {
    crapple_test();

    if (crapple_init() != 0) {
        printf("Error");
        return 1;
    };

    crapple_update();

    crapple_terminate();
    return 0;
}
