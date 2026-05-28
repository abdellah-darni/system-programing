#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *dir_path = argv[1];

    if (rmdir(dir_path) == -1) {
        perror("rmdir");
        exit(EXIT_FAILURE);
    }

    printf("Success: Directory '%s' successfully removed.\n", dir_path);

    exit(EXIT_SUCCESS);
}