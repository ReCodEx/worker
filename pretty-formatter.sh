#!/bin/bash

for f in $(find ./src -name '*.h' -or -name '*.cpp')
do
	echo "Processing $f file..."
	clang-format-3.8 -style=file -i $f
done
