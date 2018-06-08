#!/bin/bash
set -e

export PATH=$(pwd):$PATH

cd test

for FILE in $(ls *.yml); do
	echo "running $FILE"
	pgspawn "$FILE"
done
