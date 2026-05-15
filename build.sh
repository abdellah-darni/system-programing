#!/bin/bash

# Créer le dossier bin s'il n'existe pas
mkdir -p bin

echo "Compiling all C programs into ./bin/ ..."

# Compilation des commandes de fichiers
gcc -o bin/my_attr my_attr.c
gcc -o bin/my_chmod my_chmod.c
gcc -o bin/my_chown my_chown.c
gcc -o bin/my_chgrp my_chgrp.c
gcc -o bin/my_unlink my_unlink.c

# Compilation des commandes de répertoires
gcc -o bin/my_mkdir my_mkdir.c
gcc -o bin/my_rmdir my_rmdir.c
gcc -o bin/my_ls my_ls.c

# Compilation des commandes de processus
gcc -o bin/lance_app lance_app.c
gcc -o bin/track_apps track_apps.c

echo "Compilation complete! Your executables are in the 'bin' directory."
