#!/bin/bash

EXECUTABLE=$1
shift

if [[ "$EXECUTABLE" == *.jar ]]; then
	EXECUTABLE="java -jar $EXECUTABLE"
fi

OUTPUT=`$EXECUTABLE $@`
echo $?
echo "$OUTPUT"
