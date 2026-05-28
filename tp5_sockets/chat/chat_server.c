#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define NAME_SIZE 50

void full_chat(int sock_fd, const char *my_name) {
    char peer_name[NAME_SIZE];
    memset(peer_name, 0, NAME_SIZE);

    if (recv(sock_fd, peer_name, NAME_SIZE, 0) <= 0) return;
    send(sock_fd, my_name, strlen(my_name), 0);

    printf("\n--- Full-Duplex Chat Established with [%s] ---\n", peer_name);
    printf("You can both type at any time. Type 'exit' to quit.\n\n");

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return;
    }

    if (pid == 0) {
        char send_buf[BUFFER_SIZE];
        while (1) {
            memset(send_buf, 0, BUFFER_SIZE);
            fgets(send_buf, BUFFER_SIZE, stdin);
            
            send(sock_fd, send_buf, strlen(send_buf), 0);
            
            if (strncmp(send_buf, "exit", 4) == 0) {
                printf("[You ended the chat]\n");
                kill(getppid(), SIGTERM);
                break;
            }
        }
        exit(EXIT_SUCCESS);
    } 
    else {
        char recv_buf[BUFFER_SIZE];
        while (1) {
            memset(recv_buf, 0, BUFFER_SIZE);
            int n = recv(sock_fd, recv_buf, BUFFER_SIZE, 0);
            
            if (n <= 0 || strncmp(recv_buf, "exit", 4) == 0) {
                printf("\n[%s disconnected]\n", peer_name);
                kill(pid, SIGTERM);
                break;
            }
            printf("[%s]: %s", peer_name, recv_buf);
        }

        wait(NULL);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <pseudonym>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    const char *my_name = argv[2];
    int server_fd, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);
    
    printf("Server [%s] listening on port %d...\n", my_name, port);
    
    client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
    
    full_chat(client_socket, my_name);

    close(client_socket);
    close(server_fd);
    return EXIT_SUCCESS;
}
