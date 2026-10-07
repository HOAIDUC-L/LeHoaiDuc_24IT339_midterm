#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "options.h"

typedef struct {
    char *name;         /* Base name for display */
    char *path;         /* Full path to the file */
    struct stat st;     /* Metadata obtained from stat/lstat */
    int stat_ok;        /* 1 if metadata valid, 0 on failure */
    int stat_errno;     /* errno if stat failed */

    char *owner;        /* Owner name or numeric UID string */
    char *group;        /* Group name or numeric GID string */
    char *link_target;  /* Symbolic link target or NULL */

    int is_symlink;     /* 1 if file is a symbolic link */
    int is_dir;         /* 1 if file is a directory */
    int is_broken_link; /* 1 if symbolic link target cannot be accessed */
} FileInfo;

typedef struct {
    FileInfo **items;
    size_t count;
    size_t capacity;
} FileInfoList;

/*
 * Create an empty FileInfoList.
 */
FileInfoList *file_info_list_new(void);

/*
 * Append a FileInfo item to list, dynamically reallocating if needed.
 * Returns 0 on success, -1 on allocation failure.
 */
int file_info_list_add(FileInfoList *list, FileInfo *item);

/*
 * Free all FileInfo items in list and list itself.
 */
void file_info_list_free(FileInfoList *list);

/*
 * Create and populate a FileInfo struct for a given file name in parent_dir.
 * If parent_dir is NULL or empty, name is treated as the path.
 * is_operand indicates if this file is a direct command-line operand.
 * Returns newly allocated FileInfo pointer, or NULL on memory allocation failure.
 */
FileInfo *file_info_create(const char *parent_dir, const char *name, const Options *opts, int is_operand);

/*
 * Free a FileInfo struct and its dynamically allocated fields.
 */
void file_info_free(FileInfo *info);

/*
 * Returns the timestamp selected by opts (-t, -c, -u, or default mtime).
 */
time_t file_info_get_time(const FileInfo *info, const Options *opts);

#endif /* FILE_INFO_H */
