#include "display.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/sysmacros.h>

void display_format_mode(mode_t mode, char *buf)
{
    if (buf == NULL) {
        return;
    }

    /* 1. File entry type */
    if (S_ISDIR(mode)) {
        buf[0] = 'd';
    } else if (S_ISCHR(mode)) {
        buf[0] = 'c';
    } else if (S_ISBLK(mode)) {
        buf[0] = 'b';
    } else if (S_ISLNK(mode)) {
        buf[0] = 'l';
    } else if (S_ISSOCK(mode)) {
        buf[0] = 's';
    } else if (S_ISFIFO(mode)) {
        buf[0] = 'p';
#ifdef S_IFWHT
    } else if ((mode & S_IFMT) == S_IFWHT) {
        buf[0] = 'w';
#endif
    } else {
        buf[0] = '-';
    }

    /* 2. Owner permissions */
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID) {
        buf[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        buf[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    /* 3. Group permissions */
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID) {
        buf[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        buf[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    /* 4. Other permissions */
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX) {
        buf[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        buf[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

    buf[10] = '\0';
}

char display_get_classify_char(mode_t mode)
{
    if (S_ISDIR(mode)) {
        return '/';
    } else if (S_ISLNK(mode)) {
        return '@';
    } else if (S_ISSOCK(mode)) {
        return '=';
    } else if (S_ISFIFO(mode)) {
        return '|';
#ifdef S_IFWHT
    } else if ((mode & S_IFMT) == S_IFWHT) {
        return '%';
#endif
    } else if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
        return '*';
    }
    return '\0';
}

int display_humanize_number(char *buf, size_t len, int64_t bytes)
{
    static const char *prefixes[] = {"B", "K", "M", "G", "T", "P", "E"};

    if (buf == NULL || len == 0) {
        return -1;
    }

    if (bytes < 0) {
        return snprintf(buf, len, "%jdB", (intmax_t)bytes);
    }

    if (bytes < 1000) {
        return snprintf(buf, len, "%jdB", (intmax_t)bytes);
    }

    int scale = 0;
    double val = (double)bytes;
    while (val >= 999.5 && scale < 6) {
        val /= 1024.0;
        scale++;
    }

    if (val < 9.95 && scale > 0) {
        return snprintf(buf, len, "%.1f%s", val, prefixes[scale]);
    } else {
        int64_t rounded = (int64_t)(val + 0.5);
        return snprintf(buf, len, "%jd%s", (intmax_t)rounded, prefixes[scale]);
    }
}

void display_format_time(time_t ftime, char *buf, size_t len)
{
    time_t now = time(NULL);
    /* NetBSD 6-month rule: (365 / 2) * 86400 = 15768000 seconds */
    int64_t six_months = 15768000;
    struct tm *tm_info = localtime(&ftime);

    if (tm_info == NULL) {
        snprintf(buf, len, "????????????");
        return;
    }

    if (ftime + six_months > now && ftime - six_months < now) {
        strftime(buf, len, "%b %e %H:%M", tm_info);
    } else {
        strftime(buf, len, "%b %e  %Y", tm_info);
    }
}

void display_print_name(const char *name, const Options *opts)
{
    if (name == NULL) {
        return;
    }

    if (opts->non_printable_as_q) {
        for (const char *p = name; *p != '\0'; ++p) {
            unsigned char c = (unsigned char)*p;
            if (c < 32 || c == 127) {
                putchar('?');
            } else {
                putchar(c);
            }
        }
    } else {
        fputs(name, stdout);
    }
}

static int get_uint64_digits(uint64_t val)
{
    int digits = 1;
    while (val >= 10) {
        digits++;
        val /= 10;
    }
    return digits;
}

void display_file_list(const FileInfoList *list, const Options *opts, int is_dir_contents)
{
    if (list == NULL) {
        return;
    }

    if (list->count == 0) {
        if (is_dir_contents && (opts->long_format || (opts->show_blocks && isatty(STDOUT_FILENO)))) {
            if (opts->human_readable) {
                printf("total 0B\n");
            } else {
                printf("total 0\n");
            }
        }
        return;
    }

    /* 1. Calculate column widths and total blocks */
    int inode_width = 0;
    int block_width = 0;
    int link_width = 0;
    int owner_width = 0;
    int group_width = 0;
    int size_width = 0;
    uint64_t total_blocks_512 = 0;

    for (size_t i = 0; i < list->count; ++i) {
        const FileInfo *info = list->items[i];
        if (!info->stat_ok) {
            continue;
        }

        total_blocks_512 += (uint64_t)info->st.st_blocks;

        if (opts->inode) {
            int d = get_uint64_digits((uint64_t)info->st.st_ino);
            if (d > inode_width) {
                inode_width = d;
            }
        }

        if (opts->show_blocks) {
            char blk_buf[32];
            if (opts->human_readable) {
                display_humanize_number(blk_buf, sizeof(blk_buf), (int64_t)info->st.st_blocks * 512);
            } else {
                long bsz = opts->block_size > 0 ? opts->block_size : 512;
                uint64_t bcount = ((uint64_t)info->st.st_blocks * 512 + bsz - 1) / bsz;
                snprintf(blk_buf, sizeof(blk_buf), "%llu", (unsigned long long)bcount);
            }
            int len = (int)strlen(blk_buf);
            if (len > block_width) {
                block_width = len;
            }
        }

        if (opts->long_format) {
            int d = get_uint64_digits((uint64_t)info->st.st_nlink);
            if (d > link_width) {
                link_width = d;
            }

            int olen = info->owner ? (int)strlen(info->owner) : 0;
            if (olen > owner_width) {
                owner_width = olen;
            }

            int glen = info->group ? (int)strlen(info->group) : 0;
            if (glen > group_width) {
                group_width = glen;
            }

            char sz_buf[32];
            if (S_ISCHR(info->st.st_mode) || S_ISBLK(info->st.st_mode)) {
                snprintf(sz_buf, sizeof(sz_buf), "%u, %u",
                         major(info->st.st_rdev), minor(info->st.st_rdev));
            } else if (opts->human_readable) {
                display_humanize_number(sz_buf, sizeof(sz_buf), (int64_t)info->st.st_size);
            } else {
                snprintf(sz_buf, sizeof(sz_buf), "%llu", (unsigned long long)info->st.st_size);
            }
            int slen = (int)strlen(sz_buf);
            if (slen > size_width) {
                size_width = slen;
            }
        }
    }

    /* 2. Print total line for directory contents when appropriate */
    if (is_dir_contents) {
        if (opts->long_format || (opts->show_blocks && isatty(STDOUT_FILENO))) {
            if (opts->human_readable) {
                char tot_buf[32];
                display_humanize_number(tot_buf, sizeof(tot_buf), (int64_t)total_blocks_512 * 512);
                printf("total %s\n", tot_buf);
            } else {
                long bsz = opts->block_size > 0 ? opts->block_size : 512;
                uint64_t tot_units = (total_blocks_512 * 512 + bsz - 1) / bsz;
                printf("total %llu\n", (unsigned long long)tot_units);
            }
        }
    }

    /* 3. Print each file entry */
    for (size_t i = 0; i < list->count; ++i) {
        const FileInfo *info = list->items[i];

        if (!info->stat_ok) {
            display_print_name(info->name, opts);
            putchar('\n');
            continue;
        }

        if (opts->inode) {
            printf("%*ju ", inode_width, (uintmax_t)info->st.st_ino);
        }

        if (opts->show_blocks) {
            char blk_buf[32];
            if (opts->human_readable) {
                display_humanize_number(blk_buf, sizeof(blk_buf), (int64_t)info->st.st_blocks * 512);
            } else {
                long bsz = opts->block_size > 0 ? opts->block_size : 512;
                uint64_t bcount = ((uint64_t)info->st.st_blocks * 512 + bsz - 1) / bsz;
                snprintf(blk_buf, sizeof(blk_buf), "%llu", (unsigned long long)bcount);
            }
            printf("%*s ", block_width, blk_buf);
        }

        if (opts->long_format) {
            char mode_str[16];
            display_format_mode(info->st.st_mode, mode_str);

            char time_str[32];
            time_t ftime = file_info_get_time(info, opts);
            display_format_time(ftime, time_str, sizeof(time_str));

            printf("%s %*lu %-*s  %-*s  ",
                   mode_str,
                   link_width, (unsigned long)info->st.st_nlink,
                   owner_width, info->owner ? info->owner : "",
                   group_width, info->group ? info->group : "");

            if (S_ISCHR(info->st.st_mode) || S_ISBLK(info->st.st_mode)) {
                char dev_buf[32];
                snprintf(dev_buf, sizeof(dev_buf), "%u, %u",
                         major(info->st.st_rdev), minor(info->st.st_rdev));
                printf("%*s ", size_width, dev_buf);
            } else if (opts->human_readable) {
                char sz_buf[32];
                display_humanize_number(sz_buf, sizeof(sz_buf), (int64_t)info->st.st_size);
                printf("%*s ", size_width, sz_buf);
            } else {
                printf("%*llu ", size_width, (unsigned long long)info->st.st_size);
            }

            printf("%s ", time_str);
        }

        display_print_name(info->name, opts);

        if (opts->classify) {
            char c = display_get_classify_char(info->st.st_mode);
            if (c != '\0') {
                putchar(c);
            }
        }

        if (opts->long_format && info->is_symlink && info->link_target != NULL) {
            printf(" -> ");
            display_print_name(info->link_target, opts);
        }

        putchar('\n');
    }
}
