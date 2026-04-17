// my_attr.c

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>

int main(int argc, char *argv[])
{
    struct stat sb;
    struct passwd *pw;
    struct group *gr;
    char mode_str[10];

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (lstat(argv[1], &sb) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    printf("ID of containing device:  [%x,%x]\n",
        major(sb.st_dev),
        minor(sb.st_dev));

    printf("File type:                ");

    switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:  printf("block device\n");            break;
    case S_IFCHR:  printf("character device\n");        break;
    case S_IFDIR:  printf("directory\n");               break;
    case S_IFIFO:  printf("FIFO/pipe\n");               break;
    case S_IFLNK:  printf("symlink\n");                 break;
    case S_IFREG:  printf("regular file\n");            break;
    case S_IFSOCK: printf("socket\n");                  break;
    default:       printf("unknown?\n");                break;
    }

    printf("I-node number:            %ju\n", (uintmax_t) sb.st_ino);

    mode_str[0] = (sb.st_mode & S_IRUSR) ? 'r' : '-';
    mode_str[1] = (sb.st_mode & S_IWUSR) ? 'w' : '-';
    mode_str[2] = (sb.st_mode & S_IXUSR) ? 'x' : '-';
    mode_str[3] = (sb.st_mode & S_IRGRP) ? 'r' : '-';
    mode_str[4] = (sb.st_mode & S_IWGRP) ? 'w' : '-';
    mode_str[5] = (sb.st_mode & S_IXGRP) ? 'x' : '-';
    mode_str[6] = (sb.st_mode & S_IROTH) ? 'r' : '-';
    mode_str[7] = (sb.st_mode & S_IWOTH) ? 'w' : '-';
    mode_str[8] = (sb.st_mode & S_IXOTH) ? 'x' : '-';
    mode_str[9] = '\0';

    printf("Mode:                     %s (%jo octal)\n", 
        mode_str, (uintmax_t) sb.st_mode);

    printf("Link count:               %ju\n", (uintmax_t) sb.st_nlink);

    pw = getpwuid(sb.st_uid);
    gr = getgrgid(sb.st_gid);

    printf("Ownership:                User: %s   Group: %s\n",
        (pw != NULL) ? pw->pw_name : "unknown",
        (gr != NULL) ? gr->gr_name : "unknown");

    printf("Preferred I/O block size: %jd bytes\n",
        (intmax_t) sb.st_blksize);
    printf("File size:                %jd bytes\n",
        (intmax_t) sb.st_size);
    printf("Blocks allocated:         %jd\n",
        (intmax_t) sb.st_blocks);

    printf("Last status change:       %s", ctime(&sb.st_ctime));
    printf("Last file access:         %s", ctime(&sb.st_atime));
    printf("Last file modification:   %s", ctime(&sb.st_mtime));

    exit(EXIT_SUCCESS);
}