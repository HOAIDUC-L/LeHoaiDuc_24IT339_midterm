#include "file_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>

FileInfoList *file_info_list_new(void)
{
    FileInfoList *list = malloc(sizeof(FileInfoList));
    if (list == NULL) {
        return NULL;
    }
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    return list;
}

int file_info_list_add(FileInfoList *list, FileInfo *item)
{
    if (list == NULL || item == NULL) {
        return -1;
    }

    if (list->count >= list->capacity) {
        size_t new_cap = (list->capacity == 0) ? 16 : list->capacity * 2;
        FileInfo **new_items = realloc(list->items, new_cap * sizeof(FileInfo *));
        if (new_items == NULL) {
            return -1;
        }
        list->items = new_items;
        list->capacity = new_cap;
    }

    list->items[list->count++] = item;
    return 0;
}

void file_info_free(FileInfo *info)
{
    if (info == NULL) {
        return;
    }
    if (info->name != NULL) {
        free(info->name);
    }
    if (info->path != NULL) {
        free(info->path);
    }
    if (info->owner != NULL) {
        free(info->owner);
    }
    if (info->group != NULL) {
        free(info->group);
    }
    if (info->link_target != NULL) {
        free(info->link_target);
    }
    free(info);
}

void file_info_list_free(FileInfoList *list)
{
    if (list == NULL) {
        return;
    }
    for (size_t i = 0; i < list->count; ++i) {
        file_info_free(list->items[i]);
    }
    free(list->items);
    free(list);
}

static char *read_symlink_target(const char *path, ssize_t size_hint)
{
    size_t buf_size = (size_hint > 0) ? (size_t)size_hint + 1 : 128;
    char *buf = NULL;

    while (1) {
        char *new_buf = realloc(buf, buf_size);
        if (new_buf == NULL) {
            free(buf);
            return NULL;
        }
        buf = new_buf;

        ssize_t n = readlink(path, buf, buf_size - 1);
        if (n < 0) {
            free(buf);
            return NULL;
        }
        if ((size_t)n < buf_size - 1) {
            buf[n] = '\0';
            return buf;
        }
        buf_size *= 2;
    }
}

static char *format_numeric_id(unsigned int id)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%u", id);
    return strdup(buf);
}

FileInfo *file_info_create(const char *parent_dir, const char *name, const Options *opts, int is_operand)
{
    if (name == NULL) {
        return NULL;
    }

    FileInfo *info = calloc(1, sizeof(FileInfo));
    if (info == NULL) {
        return NULL;
    }

    info->name = strdup(name);
    if (info->name == NULL) {
        free(info);
        return NULL;
    }

    /* Build full path */
    if (parent_dir == NULL || parent_dir[0] == '\0') {
        info->path = strdup(name);
    } else {
        size_t plen = strlen(parent_dir);
        size_t nlen = strlen(name);
        int need_slash = (plen > 0 && parent_dir[plen - 1] != '/');
        size_t tot_len = plen + (need_slash ? 1 : 0) + nlen + 1;

        info->path = malloc(tot_len);
        if (info->path != NULL) {
            if (need_slash) {
                snprintf(info->path, tot_len, "%s/%s", parent_dir, name);
            } else {
                snprintf(info->path, tot_len, "%s%s", parent_dir, name);
            }
        }
    }

    if (info->path == NULL) {
        file_info_free(info);
        return NULL;
    }

    /* Determine stat behavior */
    int stat_res = -1;
    if (is_operand) {
        /*
         * NetBSD manual:
         * -d: directories listed as plain files, symlinks in argument list not indirected through.
         * If not -F, -d or -l, follow symbolic links listed on command line.
         */
        if (opts->directory_as_file || opts->long_format || opts->classify) {
            stat_res = lstat(info->path, &info->st);
        } else {
            /* Try stat first to follow symlinks to directories */
            stat_res = stat(info->path, &info->st);
            if (stat_res != 0) {
                /* If stat failed (e.g. broken symlink), try lstat */
                stat_res = lstat(info->path, &info->st);
            }
        }
    } else {
        /* Directory entries are always inspected with lstat */
        stat_res = lstat(info->path, &info->st);
    }

    if (stat_res == 0) {
        info->stat_ok = 1;

        if (S_ISLNK(info->st.st_mode)) {
            info->is_symlink = 1;
            info->link_target = read_symlink_target(info->path, info->st.st_size);

            struct stat target_st;
            if (stat(info->path, &target_st) != 0) {
                info->is_broken_link = 1;
            }
        }

        if (S_ISDIR(info->st.st_mode)) {
            info->is_dir = 1;
        }

        /* Resolve owner and group */
        if (opts->long_format) {
            if (opts->numeric_ids) {
                info->owner = format_numeric_id((unsigned int)info->st.st_uid);
                info->group = format_numeric_id((unsigned int)info->st.st_gid);
            } else {
                struct passwd *pw = getpwuid(info->st.st_uid);
                if (pw != NULL && pw->pw_name != NULL) {
                    info->owner = strdup(pw->pw_name);
                } else {
                    info->owner = format_numeric_id((unsigned int)info->st.st_uid);
                }

                struct group *gr = getgrgid(info->st.st_gid);
                if (gr != NULL && gr->gr_name != NULL) {
                    info->group = strdup(gr->gr_name);
                } else {
                    info->group = format_numeric_id((unsigned int)info->st.st_gid);
                }
            }
        }
    } else {
        info->stat_ok = 0;
        info->stat_errno = errno;
    }

    return info;
}

time_t file_info_get_time(const FileInfo *info, const Options *opts)
{
    if (info == NULL || !info->stat_ok) {
        return 0;
    }

    if (opts->time_field == TIME_STATUS_CHANGE) {
        return info->st.st_ctime;
    } else if (opts->time_field == TIME_ACCESS) {
        return info->st.st_atime;
    }
    return info->st.st_mtime;
}
