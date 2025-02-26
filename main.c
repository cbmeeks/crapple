#include <stdio.h>
#include "crapple.c"

int main(void) {

    crapple_init();
    crapple_test();

    crapple_update();

    crapple_terminate();
    return 0;
}
