#!/bin/bash

EXECUTABLE=$1
shift

chmod a+x "$EXECUTABLE"

if [[ "$EXECUTABLE" == *.jar ]]; then
	EXECUTABLE="java -jar $EXECUTABLE"
fi

OUTPUT=`"$EXECUTABLE" "$@"`
echo $?
echo "$OUTPUT"
