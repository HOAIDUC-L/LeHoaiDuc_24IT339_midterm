#ifndef OPTIONS_H
#define OPTIONS_H

#include <sys/types.h>

typedef enum {
    SORT_BY_NAME = 0,
    SORT_BY_SIZE,
    SORT_BY_TIME
} SortKey;

typedef enum {
    TIME_MODIFICATION = 0, /* Default: time of last modification (st_mtime) */
    TIME_STATUS_CHANGE,    /* -c: time file status was last changed (st_ctime) */
    TIME_ACCESS            /* -u: time of last access (st_atime) */
} TimeField;

typedef enum {
    FORMAT_SINGLE_COLUMN = 0, /* Default or -1: one entry per line */
    FORMAT_LONG,              /* -l or -n: long listing */
    FORMAT_COLUMN,            /* -C: multi-column sorted down columns */
    FORMAT_COLUMN_ACROSS,     /* -x: multi-column sorted across rows */
    FORMAT_STREAM             /* -m: comma-separated stream */
} DisplayFormat;

typedef struct {
    DisplayFormat format;   /* -1, -C, -x, -m, -l, -n */
    int show_all;           /* -a: include . and .. */
    int almost_all;         /* -A: list all entries except for . and .. */
    int directory_as_file;  /* -d: directories listed as plain files */
    int classify;           /* -F: append type indicator (/ * @ % = |) */
    int no_sort;            /* -f: output is not sorted */
    int human_readable;     /* -h: human-readable sizes (bytes, K, M, G, ...) */
    int inode;              /* -i: print inode number */
    int kilobytes;          /* -k: block counts reported in kilobytes (1024 B) */
    int long_format;        /* -l / -n: list in long format */
    int numeric_ids;        /* -n: numeric owner and group IDs */
    int non_printable_as_q; /* -q: non-printable chars as '?' (default if terminal) */
                            /* -w: raw non-printable chars (default if non-terminal) */
    int recursive;          /* -R: recursively list subdirectories */
    int reverse_sort;       /* -r: reverse sort order */
    int show_blocks;        /* -s: display file system blocks */
    
    SortKey sort_key;       /* SORT_BY_NAME, SORT_BY_SIZE, SORT_BY_TIME */
    TimeField time_field;   /* TIME_MODIFICATION, TIME_STATUS_CHANGE, TIME_ACCESS */

    long block_size;        /* 512, 1024, or parsed from BLOCKSIZE env */
} Options;

/*
 * Initialize options to default values.
 * Returns 0 on success.
 */
int options_init(Options *opts);

/*
 * Parse command-line flags and handle overrides according to manual.
 * Sets *optind_out to index of first non-option operand in argv.
 * Returns 0 on success, -1 on invalid option.
 */
int options_parse(Options *opts, int argc, char *argv[], int *optind_out);

#endif /* OPTIONS_H */
