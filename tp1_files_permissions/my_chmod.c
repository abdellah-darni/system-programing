// my_chmod.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <file_path> <u|g|o|a> <r|w|x>\n", argv[0]);
        fprintf(stderr, "Toggles the specified permission for the given user class.\n");
        exit(EXIT_FAILURE);
    }

    const char *file_path = argv[1];
    
    char target = tolower(argv[2][0]);
    char perm   = tolower(argv[3][0]);

    struct stat sb;
    mode_t mask = 0;

    if (stat(file_path, &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    switch (target) {
        case 'u':
            if (perm == 'r') mask = S_IRUSR;
            else if (perm == 'w') mask = S_IWUSR;
            else if (perm == 'x') mask = S_IXUSR;
            break;
        case 'g':
            if (perm == 'r') mask = S_IRGRP;
            else if (perm == 'w') mask = S_IWGRP;
            else if (perm == 'x') mask = S_IXGRP;
            break;
        case 'o':
            if (perm == 'r') mask = S_IROTH;
            else if (perm == 'w') mask = S_IWOTH;
            else if (perm == 'x') mask = S_IXOTH;
            break;
        case 'a':
            if (perm == 'r') mask = S_IRUSR | S_IRGRP | S_IROTH;
            else if (perm == 'w') mask = S_IWUSR | S_IWGRP | S_IWOTH;
            else if (perm == 'x') mask = S_IXUSR | S_IXGRP | S_IXOTH;
            break;
    }

    if (mask == 0) {
        fprintf(stderr, "Error: Invalid target ('%c') or permission ('%c').\n", target, perm);
        exit(EXIT_FAILURE);
    }

    mode_t new_mode = sb.st_mode ^ mask;

    if (chmod(file_path, new_mode) == -1) {
        perror("chmod");
        exit(EXIT_FAILURE);
    }

    printf("Success: Toggled '%c' permission for '%c' on %s\n", perm, target, file_path);

    exit(EXIT_SUCCESS);
}