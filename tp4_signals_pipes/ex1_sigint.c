#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

const char data_to_save[] = "Donnees critiques: Processus interrompu par l'utilisateur (Ctrl-C).\n";

void handle_sigint(int sig) {

    int fd = open("sauvegarde.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    
    if (fd != -1) {
        write(fd, data_to_save, strlen(data_to_save));
        close(fd);
    }
    
    const char *msg = "\n[SIGINT] Sauvegarde effectuee dans 'sauvegarde.txt'. Fin du programme.\n";
    write(STDOUT_FILENO, msg, strlen(msg));
    
    exit(EXIT_SUCCESS);
}

int main(void) {
    struct sigaction sa;

    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }

    printf("Programme en cours d'execution (PID: %d).\n", getpid());
    printf("Appuyez sur Ctrl-C pour declencher la sauvegarde...\n");

    while (1) {
        sleep(1);
    }

    return EXIT_SUCCESS;
}