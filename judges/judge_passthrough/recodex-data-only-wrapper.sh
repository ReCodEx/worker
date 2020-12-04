#!/bin/bash

EXECUTABLE=$1
shift

chmod a+x "$EXECUTABLE"

if [[ "$EXECUTABLE" == *.jar ]]; then
	EXECUTABLE="java -jar $EXECUTABLE"
fi

TMPFILE=`mktemp`
OUTPUT=`"$EXECUTABLE" "$@" 2>"$TMPFILE"`

echo $?

echo "$OUTPUT" | base64 -w 0
echo

cat "$TMPFILE" | base64 -w 0
echo

rm -rf "$TMPFILE" 1>/dev/null 2>/dev/null
