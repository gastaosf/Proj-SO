#!/bin/bash

inputdir=$1
outputdir=$2 
maxthreads=$3

if [ ! -d "$inputdir" ];then
    echo "Directory -- $inputdir -- does not exist."    
elif [ ! -d "$outputdir" ];then
    echo "Directory -- $outputdir -- does not exist."   
elif [ $maxthreads -le 0 ];then
    echo "Max threads must be a positive number"  
else
    for inputfile in $inputdir/*.txt
    do
        inputfile_basename=$(basename $inputfile .txt)
        for i in $(seq 1 $maxthreads)
        do
            echo Inputfile=$inputfile_basename NumThreads=$i
            ./tecnicofs ${inputfile} $outputdir/$inputfile_basename-$i.txt $i | grep "TecnicoFS completed in"
        done
    done
fi
