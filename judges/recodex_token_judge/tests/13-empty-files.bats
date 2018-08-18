#!/usr/bin/env bats

load bats-shared

@test "empty files" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]
}
