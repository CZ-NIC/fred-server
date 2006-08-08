#!/bin/sh
DATABASE="dbname=ccreg_test user=pblaha host=petr"
echo DATABASE=$DATABASE
time ./db_test  "$DATABASE" 1000 1000 > log1 &
time ./db_test  "$DATABASE" 2000 1000 > log2 &
time ./db_test  "$DATABASE" 3000 1000 > log3 &
time ./db_test  "$DATABASE" 4000 1000 > log4 &
time ./db_test  "$DATABASE" 5000 1000 > log5 &
