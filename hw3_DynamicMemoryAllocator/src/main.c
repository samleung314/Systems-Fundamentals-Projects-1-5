#include <stdio.h>
#include "sfmm.h"
#include "helper.h"

int main(int argc, char const *argv[]) {

    sf_mem_init(); //Called once before any allocation requests

    void *a = sf_malloc(16);
    sf_varprint(a);

    a = sf_realloc(a,1);
    sf_varprint(a);

    sf_mem_fini(); //Called after all allocation requests

    return EXIT_SUCCESS;
}