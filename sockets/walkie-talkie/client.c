#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define NAME_SIZE 50

void chatc(int sock_fd, const char *my_name) {
    char buffer[BUFFER_SIZE];
    char peer_name[NAME_SIZE];
    int n;

    memset(peer_name, 0, NAME_SIZE);

    send(sock_fd, my_name, strlen(my_name), 0);

    if (recv(sock_fd, peer_name, NAME_SIZE, 0) <= 0) {
        printf("Failed to receive server pseudonym.\n");
        return;
    }

    printf("\n--- Connection Established with [%s] ---\n", peer_name);
    printf("Type 'exit' to end the conversation.\n\n");

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        printf("[%s] (You): ", my_name);
        fgets(buffer, BUFFER_SIZE, stdin);
        
        send(sock_fd, buffer, strlen(buffer), 0);
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("[You ended the chat]\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);
        n = recv(sock_fd, buffer, sizeof(buffer), 0);
        if (n <= 0) {
            printf("\n[%s dropped the connection]\n", peer_name);
            break;
        }
        
        printf("[%s]: %s", peer_name, buffer);
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("[%s ended the chat]\n", peer_name);
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <pseudonym>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *my_name = argv[3];

    int client_fd;
    struct sockaddr_in server_addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to server failed");
        exit(EXIT_FAILURE);
    }

    chatc(client_fd, my_name);

    close(client_fd);

    return EXIT_SUCCESS;
}
