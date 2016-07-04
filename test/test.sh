#!/bin/bash

echo "===>>>CLEANING<<<==="

make clean

echo "===>>>COMPILING<<<==="

make all

if [ $? -ne 0 ]; then
	exit
fi

LIST=`ls | egrep 'TEST_.+' | xargs`
echo "===>>>TESTING<<<==="

for file in $LIST; do
	echo -e "\e[1;32m[>>>$file<<<]\e[0m"
	./$file
done
