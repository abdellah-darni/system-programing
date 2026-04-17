#!/bin/bash

gcc -o file_info file_info.c
gcc -o my_chmod my_chmod.c
gcc -o my_chown my_chown.c
gcc -o my_chgrp my_chgrp.c
gcc -o my_delete my_delete.c

touch test_target.txt

./file_info test_target.txt

./my_chmod test_target.txt u x
./file_info test_target.txt

sudo ./my_chgrp root test_target.txt
./file_info test_target.txt

sudo ./my_chown root test_target.txt
./file_info test_target.txt

sudo ./my_delete test_target.txt
