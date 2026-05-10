#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <string.h>

void print_file_info(const char *filename, const struct stat *sb)
{
    char mode_str[11];
    struct passwd *pw;
    struct group *gr;

    switch (sb->st_mode & S_IFMT) {
        case S_IFBLK:  mode_str[0] = 'b'; break;
        case S_IFCHR:  mode_str[0] = 'c'; break;
        case S_IFDIR:  mode_str[0] = 'd'; break;
        case S_IFIFO:  mode_str[0] = 'p'; break;
        case S_IFLNK:  mode_str[0] = 'l'; break;
        case S_IFREG:  mode_str[0] = '-'; break;
        case S_IFSOCK: mode_str[0] = 's'; break;
        default:       mode_str[0] = '?'; break;
    }

    mode_str[1] = (sb->st_mode & S_IRUSR) ? 'r' : '-';
    mode_str[2] = (sb->st_mode & S_IWUSR) ? 'w' : '-';
    mode_str[3] = (sb->st_mode & S_IXUSR) ? 'x' : '-';
    mode_str[4] = (sb->st_mode & S_IRGRP) ? 'r' : '-';
    mode_str[5] = (sb->st_mode & S_IWGRP) ? 'w' : '-';
    mode_str[6] = (sb->st_mode & S_IXGRP) ? 'x' : '-';
    mode_str[7] = (sb->st_mode & S_IROTH) ? 'r' : '-';
    mode_str[8] = (sb->st_mode & S_IWOTH) ? 'w' : '-';
    mode_str[9] = (sb->st_mode & S_IXOTH) ? 'x' : '-';
    mode_str[10] = '\0';

    pw = getpwuid(sb->st_uid);
    gr = getgrgid(sb->st_gid);

    printf("%s %2lu %-8s %-8s %8jd %s\n",
           mode_str,
           (unsigned long) sb->st_nlink,
           (pw != NULL) ? pw->pw_name : "unknown",
           (gr != NULL) ? gr->gr_name : "unknown",
           (intmax_t) sb->st_size,
           filename);
}

void list_dir(const char *dir_path, int recursive)
{
    DIR *dir;
    struct dirent *entry;
    struct stat sb;
    char full_path[1024];

    char **subdirs = NULL;
    int subdir_count = 0;
    int subdir_capacity = 10;

    if (recursive) {
        subdirs = malloc(subdir_capacity * sizeof(char*));
        printf("\n%s:\n", dir_path);
    }

    if ((dir = opendir(dir_path)) == NULL) {
        perror(dir_path);
        if (recursive) free(subdirs);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (lstat(full_path, &sb) == -1) {
            perror("lstat");
            continue;
        }

        print_file_info(entry->d_name, &sb);

        if (recursive && S_ISDIR(sb.st_mode)) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                
                if (subdir_count >= subdir_capacity) {
                    subdir_capacity *= 2;
                    subdirs = realloc(subdirs, subdir_capacity * sizeof(char*));
                }
                
                subdirs[subdir_count] = strdup(full_path);
                subdir_count++;
            }
        }
    }

    closedir(dir);

    if (recursive) {
        for (int i = 0; i < subdir_count; i++) {
            list_dir(subdirs[i], recursive);
            free(subdirs[i]);
        }
        free(subdirs);
    }
}

int main(int argc, char *argv[])
{
    int recursive = 0;
    const char *dir_path = ".";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0) {
            recursive = 1;
        } else {
            dir_path = argv[i];
        }
    }

    list_dir(dir_path, recursive);
    exit(EXIT_SUCCESS);
}
