#ifndef DISPLAY_H
#define DISPLAY_H

#include <sys/types.h>
#include <stdint.h>
#include "file_info.h"
#include "options.h"

/*
 * Formats file mode bits into a 10-character string plus null terminator.
 * Buffer must have size of at least 11 bytes.
 * Handles file types, rwx permissions, setuid (s/S), setgid (s/S), and sticky bit (t/T).
 */
void display_format_mode(mode_t mode, char *buf);

/*
 * Returns the classification indicator character for -F:
 * '/' for directory
 * '*' for executable
 * '@' for symbolic link
 * '%' for whiteout
 * '=' for socket
 * '|' for FIFO
 * '\0' for none
 */
char display_get_classify_char(mode_t mode);

/*
 * Formats a byte size into human-readable format according to BSD humanize_number(3)
 * specifications used by NetBSD ls:
 * - 0 to 999 bytes: "123B"
 * - 1.0K to 9.9K: "1.2K" (one decimal place)
 * - 10K to 999K: "12K"
 * - 1.0M to 9.9M: "2.5M"
 * - 10M to 999M: "50M"
 * and so on with G, T, P, E.
 */
int display_humanize_number(char *buf, size_t len, int64_t bytes);

/*
 * Formats timestamp following the NetBSD 6-month rule:
 * - If within 6 months: "Mmm dd HH:MM" (e.g. "Oct  7 20:30")
 * - If older or future: "Mmm dd  YYYY" (e.g. "Oct  7  2024")
 * Fixed width of 12 characters. Buffer must be at least 13 bytes.
 */
void display_format_time(time_t ftime, char *buf, size_t len);

/*
 * Prints a filename or target path to stdout.
 * If opts->non_printable_as_q is set (-q), replaces non-printable characters with '?'.
 * Otherwise (-w), prints raw characters.
 */
void display_print_name(const char *name, const Options *opts);

/*
 * Formats and prints a list of FileInfo objects to stdout:
 * - Manages formats: single column (-1), columns down (-C), columns across (-x), stream (-m), long (-l/-n)
 * - Manages column widths for alignment in -l, -i, -s, -C, and -x
 * - Prints "total <blocks/size>" header if is_dir_contents and (-l or (-s on terminal))
 * - Handles device major/minor numbers in place of size for special device files
 * - Displays symlink targets preceded by " -> " in long format
 * - Appends classify indicators (-F)
 */
void display_file_list(const FileInfoList *list, const Options *opts, int is_dir_contents);

#endif /* DISPLAY_H */
