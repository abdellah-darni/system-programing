#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024
#define NAME_SIZE 50

void chats(int sock_fd, const char *my_name) {
    char buffer[BUFFER_SIZE];
    char peer_name[NAME_SIZE];
    int n;

    memset(peer_name, 0, NAME_SIZE);

    if (recv(sock_fd, peer_name, NAME_SIZE, 0) <= 0) {
        printf("Failed to receive client pseudonym.\n");
        return;
    }
    
    send(sock_fd, my_name, strlen(my_name), 0);

    printf("\n--- Connection Established with [%s] ---\n", peer_name);
    printf("Waiting for [%s] to speak first...\n\n", peer_name);

    while (1) {
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

        memset(buffer, 0, BUFFER_SIZE);
        printf("[%s] (You): ", my_name);
        fgets(buffer, BUFFER_SIZE, stdin);
        
        send(sock_fd, buffer, strlen(buffer), 0);
        if (strncmp(buffer, "exit", 4) == 0) {
            printf("[You ended the chat]\n");
            break;
        }
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
    socklen_t addr_size;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == 0) {
        printf("Server started successfully!\n");
        printf("Listening on IP: 0.0.0.0 (All interfaces)\n");
        printf("Listening on Port: %d\n", port);
        printf("Server Pseudonym: %s\n", my_name);
        printf("Waiting for incoming connections...\n");
    } else {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    addr_size = sizeof(client_addr);
    client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
    if (client_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    chats(client_socket, my_name);

    close(client_socket);
    close(server_fd);
    
    return EXIT_SUCCESS;
}
