#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

typedef struct ProcessNode {
    pid_t pid;
    char *name;
    struct ProcessNode *next;
} ProcessNode;

ProcessNode* add_process(ProcessNode *head, pid_t pid, const char *name) 
{
    ProcessNode *new_node = malloc(sizeof(ProcessNode));
    if (new_node == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    new_node->pid = pid;
    
    new_node->name = strdup(name);
    if (new_node->name == NULL) {
        perror("strdup failed");
        free(new_node);
        exit(EXIT_FAILURE);
    }

    new_node->next = head;
    
    return new_node;
}

void destroy_list(ProcessNode *head) 
{
    ProcessNode *current = head;
    ProcessNode *next_node;

    while (current != NULL) {
        next_node = current->next;
        
        free(current->name);
        free(current);
        
        current = next_node;
    }
}

void print_list(ProcessNode *head) 
{
    ProcessNode *current = head;
    printf("\n--- Process Tracking List ---\n");
    while (current != NULL) {
        printf("PID: %d \t Program: %s\n", current->pid, current->name);
        current = current->next;
    }
    printf("-----------------------------\n\n");
}

int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program1> [program2 ...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    ProcessNode *process_list = NULL;

    for (int i = 1; i < argc; i++) {
        pid_t pid = fork();

        if (pid < 0) {
            perror("fork failed");
            continue;
        } 
        else if (pid == 0) {
            execlp(argv[i], argv[i], (char *)NULL);
            
            fprintf(stderr, "Error: Failed to execute '%s'\n", argv[i]);
            exit(EXIT_FAILURE);
        } 
        else {
            process_list = add_process(process_list, pid, argv[i]);
        }
    }

    print_list(process_list);

    int status;
    pid_t wpid;
    printf("Waiting for all programs to finish...\n");
    while ((wpid = wait(&status)) > 0) {
        printf("Process %d has terminated.\n", wpid);
    }

    printf("\nDestroying the linked list and freeing memory...\n");
    destroy_list(process_list);
    process_list = NULL;

    printf("Execution complete. Exiting cleanly.\n");
    exit(EXIT_SUCCESS);
}
