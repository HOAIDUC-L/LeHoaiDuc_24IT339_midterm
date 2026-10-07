#ifndef SORT_H
#define SORT_H

#include "file_info.h"
#include "options.h"

/*
 * Sorts FileInfoList according to criteria specified in Options:
 * - Default: Lexicographical order by filename
 * - -t: Time modified (or -c/-u), latest first; tie-break lexicographically
 * - -S: Size, largest first; tie-break lexicographically
 * - -r: Reverses sorting order
 * - -f: Output is not sorted
 */
void sort_file_info_list(FileInfoList *list, const Options *opts);

#endif /* SORT_H */
