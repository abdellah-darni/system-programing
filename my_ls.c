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

int main(int argc, char *argv[])
{
    const char *dir_path = (argc >= 2) ? argv[1] : ".";
    
    DIR *dir;
    struct dirent *entry;
    struct stat sb;
    char full_path[1024];

    if ((dir = opendir(dir_path)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        if (lstat(full_path, &sb) == -1) {
            perror("lstat");
            continue;
        }

        print_file_info(entry->d_name, &sb);
    }

    closedir(dir);
    exit(EXIT_SUCCESS);
}