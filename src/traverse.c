#include "traverse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include "file_info.h"
#include "sort.h"
#include "display.h"

static void traverse_dir(const char *dir_path, const Options *opts, int print_header,
                         int *has_printed_something, int *exit_status)
{
    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "ls: %s: %s\n", dir_path, strerror(errno));
        *exit_status = 1;
        return;
    }

    if (print_header) {
        if (*has_printed_something) {
            putchar('\n');
        }
        printf("%s:\n", dir_path);
        *has_printed_something = 1;
    }

    FileInfoList *entry_list = file_info_list_new();
    if (entry_list == NULL) {
        closedir(dir);
        *exit_status = 1;
        return;
    }

    struct dirent *de;
    while (1) {
        errno = 0;
        de = readdir(dir);
        if (de == NULL) {
            break;
        }

        /* Filter hidden files */
        if (!opts->show_all && !opts->almost_all) {
            if (de->d_name[0] == '.') {
                continue;
            }
        } else if (opts->almost_all && !opts->show_all) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
                continue;
            }
        }

        FileInfo *entry = file_info_create(dir_path, de->d_name, opts, 0);
        if (entry != NULL) {
            file_info_list_add(entry_list, entry);
        }
    }

    if (errno != 0) {
        fprintf(stderr, "ls: %s: %s\n", dir_path, strerror(errno));
        *exit_status = 1;
    }

    closedir(dir);

    /* Sort entries */
    sort_file_info_list(entry_list, opts);

    /* Display entries */
    display_file_list(entry_list, opts, 1);
    *has_printed_something = 1;

    /* Recursive traversal (-R) */
    if (opts->recursive) {
        for (size_t i = 0; i < entry_list->count; ++i) {
            FileInfo *entry = entry_list->items[i];
            /* Recurse into directories, but NOT symbolic links or . / .. */
            if (entry->is_dir && !entry->is_symlink &&
                strcmp(entry->name, ".") != 0 && strcmp(entry->name, "..") != 0) {
                traverse_dir(entry->path, opts, 1, has_printed_something, exit_status);
            }
        }
    }

    file_info_list_free(entry_list);
}

int traverse_operands(int argc, char *argv[], int optind_val, const Options *opts)
{
    int exit_status = 0;
    int has_printed_something = 0;

    /* Case 1: No operands given -> default to current directory "." */
    if (argc == optind_val) {
        if (opts->directory_as_file) {
            FileInfoList *single_list = file_info_list_new();
            if (single_list == NULL) {
                return 1;
            }
            FileInfo *dot_info = file_info_create(NULL, ".", opts, 1);
            if (dot_info != NULL) {
                file_info_list_add(single_list, dot_info);
                display_file_list(single_list, opts, 0);
            }
            file_info_list_free(single_list);
            return 0;
        }

        traverse_dir(".", opts, 0, &has_printed_something, &exit_status);
        return exit_status;
    }

    /* Case 2: Operands given -> separate into non-directory and directory operands */
    FileInfoList *non_dir_list = file_info_list_new();
    FileInfoList *dir_list = file_info_list_new();

    if (non_dir_list == NULL || dir_list == NULL) {
        file_info_list_free(non_dir_list);
        file_info_list_free(dir_list);
        return 1;
    }

    for (int i = optind_val; i < argc; ++i) {
        const char *arg = argv[i];
        FileInfo *info = file_info_create(NULL, arg, opts, 1);
        if (info == NULL) {
            exit_status = 1;
            continue;
        }

        if (!info->stat_ok) {
            fprintf(stderr, "ls: %s: %s\n", arg, strerror(info->stat_errno));
            exit_status = 1;
            file_info_free(info);
            continue;
        }

        if (opts->directory_as_file) {
            file_info_list_add(non_dir_list, info);
        } else if (info->is_dir) {
            file_info_list_add(dir_list, info);
        } else {
            file_info_list_add(non_dir_list, info);
        }
    }

    /* Sort non-directory and directory operands separately */
    sort_file_info_list(non_dir_list, opts);
    sort_file_info_list(dir_list, opts);

    /* Output non-directory operands first */
    if (non_dir_list->count > 0) {
        display_file_list(non_dir_list, opts, 0);
        has_printed_something = 1;
    }

    /* Output directory operands */
    int print_header = (non_dir_list->count > 0 || dir_list->count > 1 || opts->recursive);

    for (size_t i = 0; i < dir_list->count; ++i) {
        FileInfo *dir_info = dir_list->items[i];
        traverse_dir(dir_info->path, opts, print_header, &has_printed_something, &exit_status);
    }

    file_info_list_free(non_dir_list);
    file_info_list_free(dir_list);

    return exit_status;
}
