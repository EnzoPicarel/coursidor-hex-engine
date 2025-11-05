#!/bin/bash

if [ 1 -gt $# ]
then 
    nb=50
else
    nb=$1
fi

clear
make clean
make

echo "=============="
echo "Compiling DONE"
echo "=============="

nb_error=0
nb_error_valgrind=0
nb_nobody_win=0
for i in $(seq 1 "$nb")
do
    nb_obj=$((RANDOM % 5 + 1))
    exec=$(./install/server -m 4 -M 50 -O $nb_obj install/astar_player.so install/random_player.so)
    error_illegal=$(echo "$exec" | grep ILLEGAL)
    valgrind_output=$(valgrind --leak-check=full ./install/server -m 4 -M 50 -O $nb_obj install/astar_player.so install/random_player.so 2>&1)
    valgrind_error=$(echo "$valgrind_output" | grep "All heap blocks were freed")
    nobody_win=$(echo $exec | grep "NOBODY")
    if [ -n "$nobody_win" ]
    then
        nb_nobody_win=$((nb_nobody_win + 1))
        echo "$exec"
    fi
    if [ -n "$error_illegal" ]
    then
        nb_error=$((nb_error + 1))
        echo "$exec"
    fi

    if [ -z "$valgrind_error" ]
    then
        nb_error_valgrind=$((nb_error_valgrind + 1))
        echo "$exec"
        echo "$valgrind_output"
    fi
done


echo
echo "ON $nb EXECUTION $nb_error error, valgrind error : $nb_error_valgrind, nobody win : $nb_nobody_win"
