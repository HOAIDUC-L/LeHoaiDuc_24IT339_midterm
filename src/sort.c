#include "sort.h"

#include <stdlib.h>
#include <string.h>

static const Options *s_current_opts = NULL;

static int compare_file_info(const void *p1, const void *p2)
{
    const FileInfo *a = *(const FileInfo * const *)p1;
    const FileInfo *b = *(const FileInfo * const *)p2;

    int cmp = 0;

    if (s_current_opts->sort_key == SORT_BY_SIZE) {
        if (b->st.st_size > a->st.st_size) {
            cmp = 1;
        } else if (b->st.st_size < a->st.st_size) {
            cmp = -1;
        } else {
            cmp = strcmp(a->name, b->name);
        }
    } else if (s_current_opts->sort_key == SORT_BY_TIME) {
        time_t ta = file_info_get_time(a, s_current_opts);
        time_t tb = file_info_get_time(b, s_current_opts);
        if (tb > ta) {
            cmp = 1;
        } else if (tb < ta) {
            cmp = -1;
        } else {
            /* Check nanosecond precision if available for tie-breaking */
            long na = (s_current_opts->time_field == TIME_STATUS_CHANGE) ? a->st.st_ctim.tv_nsec :
                      (s_current_opts->time_field == TIME_ACCESS) ? a->st.st_atim.tv_nsec : a->st.st_mtim.tv_nsec;
            long nb = (s_current_opts->time_field == TIME_STATUS_CHANGE) ? b->st.st_ctim.tv_nsec :
                      (s_current_opts->time_field == TIME_ACCESS) ? b->st.st_atim.tv_nsec : b->st.st_mtim.tv_nsec;
            if (nb > na) {
                cmp = 1;
            } else if (nb < na) {
                cmp = -1;
            } else {
                cmp = strcmp(a->name, b->name);
            }
        }
    } else {
        cmp = strcmp(a->name, b->name);
    }

    if (s_current_opts->reverse_sort) {
        cmp = -cmp;
    }

    return cmp;
}

void sort_file_info_list(FileInfoList *list, const Options *opts)
{
    if (list == NULL || list->count <= 1 || opts == NULL) {
        return;
    }

    /* -f option: output is not sorted */
    if (opts->no_sort) {
        return;
    }

    s_current_opts = opts;
    qsort(list->items, list->count, sizeof(FileInfo *), compare_file_info);
    s_current_opts = NULL;
}
