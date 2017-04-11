#/usr/bin/env bash

cd ./docs
docs="docs.txt"
for doc in `ls`
do 
    cat $doc >> $docs
done
