#!/usr/bin/env bats

load bats-shared

@test "single line (no line end)" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
