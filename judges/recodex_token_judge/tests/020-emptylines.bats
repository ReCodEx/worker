#!/usr/bin/env bats

load bats-shared

@test "empty lines skipping" {
	run $EXE_FILE --ignore-empty-lines $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]
}

@test "empty lines skipping (negative test)" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
