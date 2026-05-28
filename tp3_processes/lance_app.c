#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NUM_COPIES 1000

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program1> [program2 ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) {
        printf("--- Launching %d copies of '%s' ---\n", NUM_COPIES, argv[i]);

        for (int j = 0; j < NUM_COPIES; j++) {
            
            pid_t pid = fork();

            if (pid < 0) {
                perror("fork failed");
                break; 
            } 
            else if (pid == 0) {
                execlp(argv[i], argv[i], (char *)NULL);

                perror("execlp failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    printf("All %d fork requests submitted. Waiting for children to finish...\n", (argc - 1) * NUM_COPIES);

    int status;
    while (wait(&status) > 0) {}

    printf("All child processes have cleanly terminated.\n");
    exit(EXIT_SUCCESS);
}
