#!/usr/bin/env bats

load bats-shared

@test "allow comments" {
	run $EXE_FILE --allow-comments $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
}

@test "allow comments (negative test)" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
