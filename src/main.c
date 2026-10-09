#include <stdio.h>
#include <stdlib.h>
#include "options.h"
#include "traverse.h"

int main(int argc, char *argv[])
{
    Options opts;
    if (options_init(&opts) != 0) {
        return EXIT_FAILURE;
    }

    int optind_val = 1;
    if (options_parse(&opts, argc, argv, &optind_val) != 0) {
        fprintf(stderr, "usage: ls [-1AacCdFfhiklmnqRrSstuwx] [file ...]\n");
        return EXIT_FAILURE;
    }

    int status = traverse_operands(argc, argv, optind_val, &opts);
    return (status == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
