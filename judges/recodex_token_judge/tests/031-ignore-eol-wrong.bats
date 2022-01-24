#!/usr/bin/env bats

load bats-shared

@test "ignore line ends (wrong answer)" {
	run $EXE_FILE --ignore-line-ends $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
