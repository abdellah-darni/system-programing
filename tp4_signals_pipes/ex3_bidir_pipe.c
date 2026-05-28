#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(void) {
    int p1[2];
    int p2[2];
    pid_t pid;
    
    int data_to_send[5] = {10, 20, 30, 40, 50};
    
    if (pipe(p1) == -1 || pipe(p2) == -1) {
        perror("Pipe creation failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        close(p1[1]); 
        close(p2[0]); 

        int received_val, doubled_val;

        for (int i = 0; i < 5; i++) {
            if (read(p1[0], &received_val, sizeof(int)) > 0) {
                doubled_val = received_val * 2;
                printf("[Fils]   Reçu: %d | Renvoi du double: %d\n", received_val, doubled_val);
                
                write(p2[1], &doubled_val, sizeof(int));
            }
        }

        close(p1[0]);
        close(p2[1]);
        exit(EXIT_SUCCESS);
    } 
    else {
        close(p1[0]); 
        close(p2[1]); 

        int result;

        for (int i = 0; i < 5; i++) {
            printf("[Pere]   Envoi de la valeur: %d\n", data_to_send[i]);
            write(p1[1], &data_to_send[i], sizeof(int));
            
            if (read(p2[0], &result, sizeof(int)) > 0) {
                printf("[Pere]   Double reçu: %d\n\n", result);
            }
        }

        close(p1[1]);
        close(p2[0]);
        wait(NULL);
    }

    return EXIT_SUCCESS;
}