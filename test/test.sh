#!/bin/bash

LIST=`ls | egrep 'TEST_.+' | xargs`

echo "===>>>CLEANING<<<==="

make clean

echo "===>>>COMPILING<<<==="

make all

if [ $? -ne 0 ]; then
	exit
fi

echo "===>>>TESTING<<<==="

for file in $LIST; do
	echo "   >>>$file<<<   "
	./$file
done
