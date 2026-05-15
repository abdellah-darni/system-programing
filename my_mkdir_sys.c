#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <dir_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *new_dir = argv[1];
    char dot_path[1024];
    char dotdot_path[1024];

    /* 1. Manually allocate a raw directory i-node using mknod.
     * We pass S_IFDIR to tell the kernel this i-node is a directory.
     */
    if (mknod(new_dir, S_IFDIR | 0755, 0) == -1) {
        perror("mknod failed (as expected on modern Linux)");
        exit(EXIT_FAILURE);
    }

    /* 2. Manually construct the path for the '.' hard link */
    snprintf(dot_path, sizeof(dot_path), "%s/.", new_dir);
    
    /* 3. Link the new directory to its own '.' entry */
    if (link(new_dir, dot_path) == -1) {
        perror("link dot failed");
        exit(EXIT_FAILURE);
    }

    /* 4. Manually construct the path for the '..' hard link */
    snprintf(dotdot_path, sizeof(dotdot_path), "%s/..", new_dir);
    
    /* 5. Link the parent directory (which is ".") to the new directory's ".." */
    if (link(".", dotdot_path) == -1) {
        perror("link dotdot failed");
        exit(EXIT_FAILURE);
    }

    printf("Successfully created directory from scratch!\n");
    exit(EXIT_SUCCESS);
}
