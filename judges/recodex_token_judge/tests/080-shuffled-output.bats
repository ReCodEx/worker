#!/usr/bin/env bats

load bats-shared

@test "shuffled tokens (error output)" {
	run $EXE_FILE --shuffled-tokens $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
