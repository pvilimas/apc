#include "apc.h"

int main(int argc, char** argv) {

    if (argc > 2) {
        printf("usage: %s       -- start repl mode\n"
               "       %s \"...\" -- evaluate the string\n",
               argv[0], argv[0]);
        exit(1);
    }

    apc_init();

    if (argc == 2) {
        apc_eval(argv[1]);
        apc_exit(E_OK);
    } else {
        apc_start_repl();
    }

    apc_exit(E_INTERNAL_ERROR); // how did we get here
}
