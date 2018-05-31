#!/usr/bin/env bats

load bats-shared

@test "shuffled tokens" {
	run $EXE_FILE --shuffled-tokens $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]
}

@test "shuffled tokens (negative test)" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
