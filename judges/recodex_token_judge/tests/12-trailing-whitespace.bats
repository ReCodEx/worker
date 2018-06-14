#!/usr/bin/env bats

load bats-shared

@test "ignore trailing whitespace" {
	run $EXE_FILE --ignore-trailing-whitespace $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]
}

@test "ignore trailing whitespace (negative test)" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
