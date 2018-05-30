#!/usr/bin/env bats

load bats-shared

@test "case insensitive" {
	run $EXE_FILE --case-insensitive $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]
}

@test "case insensitive (negative test)" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
