#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[])
{
    char *endptr;
    mode_t new_mode;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <octal_mode> <path>\n", argv[0]);
        fprintf(stderr, "Example: %s 0755 my_new_folder\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    long parsed_mode = strtol(argv[1], &endptr, 8);
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Invalid octal mode '%s'\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    new_mode = (mode_t)parsed_mode;

    if (mkdir(argv[2], new_mode) == -1) {
        perror("mkdir");
        exit(EXIT_FAILURE);
    }

    printf("Success: Directory '%s' created with mode %04o.\n", argv[2], new_mode);

    exit(EXIT_SUCCESS);
}