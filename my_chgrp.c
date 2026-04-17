// my_chgrp.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <grp.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <groupname> <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *groupname = argv[1];
    const char *file_path = argv[2];

    struct group *grp = getgrnam(groupname);
    
    if (grp == NULL) {
        fprintf(stderr, "Error: Group '%s' does not exist.\n", groupname);
        exit(EXIT_FAILURE);
    }

    if (chown(file_path, -1, grp->gr_gid) == -1) {
        perror("chgrp");
        exit(EXIT_FAILURE);
    }

    printf("Success: Group of '%s' updated to '%s' (GID: %u).\n", 
           file_path, groupname, grp->gr_gid);

    exit(EXIT_SUCCESS);
}