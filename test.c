#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char *argv[])
{
    const char *dir_path = (argc >= 2) ? argv[1] : ".";
    
    DIR *dir;
    struct dirent *entry;
    struct stat sb;
    char full_path[1024];

    if ((dir = opendir(dir_path)) == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

}