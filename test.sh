#!/bin/bash

echo ""
echo " Compiling C Programs..."
echo ""
gcc -o my_attr my_attr.c
# gcc -o my_chmod my_chmod.c
# gcc -o my_chown my_chown.c
# gcc -o my_chgrp my_chgrp.c
# gcc -o my_unlink my_unlink.c
gcc -o my_mkdir my_mkdir.c
gcc -o my_rmdir my_rmdir.c
echo "Compilation complete."
echo ""

# echo ""
# echo " Creating 'test_target.txt'"
# echo ""
# touch test_target.txt
# ./my_attr test_target.txtq
# echo ""

# echo ""
# echo " Testing my_chmod (u x)"
# echo ""
# ./my_chmod test_target.txt u x
# ./my_attr test_target.txt
# echo ""

# echo ""
# echo " Testing my_chgrp (root)"
# echo ""
# sudo ./my_chgrp root test_target.txt
# ./my_attr test_target.txt
# echo ""

# echo ""
# echo " Testing my_chown (root)"
# echo ""
# sudo ./my_chown root test_target.txt
# ./my_attr test_target.txt
# echo ""

# echo ""
# echo " Testing my_unlink"
# echo ""
# sudo ./my_unlink test_target.txt

# echo ""
# echo "--- Verifying deletion (Expecting an error) ---"
# ./my_attr test_target.txt


echo ""
echo " Creating test_folder"
echo ""
./my_mkdir 0755 test_folder

ls


echo ""
echo " Test test_folder with my_attr"
echo ""
./my_attr test_folder

echo ""
echo " Removing test_folder"
echo ""
./my_rmdir test_folder

ls
