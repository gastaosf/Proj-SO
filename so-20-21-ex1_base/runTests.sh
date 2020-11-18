#!/bin/bash

# $1-inputdir $2-outputdir $3-maxthreads

for inputfile in $1/*.txt
do
    inputfile_basename=$(basename $inputfile)
    for i in $(seq 1 $3)
    do
        echo Inputfile=$inputfile_basename NumThreads=$i
        ./tecnicofs ${inputfile} $2/$inputfile_basename-$i $i | grep "TecnicoFS completed in"
    done
done