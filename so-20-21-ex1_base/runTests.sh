#!/bin/bash

# $1-inputdir $2-outputdir $3-maxthreads

make clean
make

for inputfile in "$1"/*.txt
do
    for i in $(seq 1 "$3")
    do
        echo Inputfile="$inputfile" NumThreads="$i"
        ./tecnicofs "$inputfile" "$2"/"$inputfile"-"$i" "$i" rwlock | grep "TecnicoFS completed in"
    done
done