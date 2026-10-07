#include "options.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int options_init(Options *opts)
{
    if (opts == NULL) {
        return -1;
    }

    opts->show_all = 0;
    /* -A is always set for the super-user */
    opts->almost_all = (geteuid() == 0) ? 1 : 0;
    opts->directory_as_file = 0;
    opts->classify = 0;
    opts->no_sort = 0;
    opts->human_readable = 0;
    opts->inode = 0;
    opts->kilobytes = 0;
    opts->long_format = 0;
    opts->numeric_ids = 0;
    /* -q is default when output is to a terminal; -w is default when not */
    opts->non_printable_as_q = isatty(STDOUT_FILENO) ? 1 : 0;
    opts->recursive = 0;
    opts->reverse_sort = 0;
    opts->show_blocks = 0;
    opts->sort_key = SORT_BY_NAME;
    opts->time_field = TIME_MODIFICATION;
    opts->block_size = 512;

    /* Check BLOCKSIZE environment variable */
    const char *env_blocksize = getenv("BLOCKSIZE");
    if (env_blocksize != NULL && *env_blocksize != '\0') {
        char *endptr = NULL;
        long bsz = strtol(env_blocksize, &endptr, 10);
        if (endptr != env_blocksize && bsz > 0) {
            /* Support optional k/m/g suffixes if user set e.g. 1k */
            if (*endptr == 'k' || *endptr == 'K') {
                bsz *= 1024;
            } else if (*endptr == 'm' || *endptr == 'M') {
                bsz *= 1024 * 1024;
            } else if (*endptr == 'g' || *endptr == 'G') {
                bsz *= 1024 * 1024 * 1024;
            }
            if (bsz > 0) {
                opts->block_size = bsz;
            }
        }
    }

    return 0;
}

int options_parse(Options *opts, int argc, char *argv[], int *optind_out)
{
    int ch;

    /* Reset optind for POSIX getopt parsing */
    optind = 1;

    while ((ch = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (ch) {
        case 'A':
            opts->almost_all = 1;
            break;
        case 'a':
            opts->show_all = 1;
            break;
        case 'c':
            /* -c and -u override each other */
            opts->time_field = TIME_STATUS_CHANGE;
            break;
        case 'd':
            /* -d and -R override each other */
            opts->directory_as_file = 1;
            opts->recursive = 0;
            break;
        case 'F':
            opts->classify = 1;
            break;
        case 'f':
            opts->no_sort = 1;
            break;
        case 'h':
            /* The rightmost of -k and -h overrides previous flag */
            opts->human_readable = 1;
            opts->kilobytes = 0;
            break;
        case 'i':
            opts->inode = 1;
            break;
        case 'k':
            /* The rightmost of -k and -h overrides previous flag */
            opts->kilobytes = 1;
            opts->human_readable = 0;
            opts->block_size = 1024;
            break;
        case 'l':
            /* -l and -n override each other */
            opts->long_format = 1;
            opts->numeric_ids = 0;
            break;
        case 'n':
            /* -l and -n override each other */
            opts->long_format = 1;
            opts->numeric_ids = 1;
            break;
        case 'q':
            /* -w and -q override each other */
            opts->non_printable_as_q = 1;
            break;
        case 'R':
            /* -d and -R override each other */
            opts->recursive = 1;
            opts->directory_as_file = 0;
            break;
        case 'r':
            opts->reverse_sort = 1;
            break;
        case 'S':
            opts->sort_key = SORT_BY_SIZE;
            break;
        case 's':
            opts->show_blocks = 1;
            break;
        case 't':
            opts->sort_key = SORT_BY_TIME;
            break;
        case 'u':
            /* -c and -u override each other */
            opts->time_field = TIME_ACCESS;
            break;
        case 'w':
            /* -w and -q override each other */
            opts->non_printable_as_q = 0;
            break;
        case '?':
        default:
            return -1;
        }
    }

    if (optind_out != NULL) {
        *optind_out = optind;
    }

    return 0;
}
