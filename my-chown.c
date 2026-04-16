#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <username> <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *username = argv[1];
    const char *file_path = argv[2];

    struct passwd *pwd = getpwnam(username);
    
    if (pwd == NULL) {
        fprintf(stderr, "Error: User '%s' does not exist.\n", username);
        exit(EXIT_FAILURE);
    }

    if (chown(file_path, pwd->pw_uid, -1) == -1) {
        perror("chown");
        exit(EXIT_FAILURE);
    }

    printf("Success: Owner of '%s' updated to '%s' (UID: %u).\n", 
           file_path, username, pwd->pw_uid);

    exit(EXIT_SUCCESS);
}