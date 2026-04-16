#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *file_path = argv[1];

    if (unlink(file_path) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    printf("Success: File '%s' successfully unlinked.\n", file_path);

    exit(EXIT_SUCCESS);
}