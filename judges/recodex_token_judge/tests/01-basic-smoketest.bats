#!/usr/bin/env bats

load bats-shared

@test "dry run without args" {
	run $EXE_FILE
	[ "$status" -eq 2 ]
	[[ "${lines[0]}" =~ "Error:" ]]
	[[ "${lines[1]}" =~ "Usage:" || "${lines[2]}" =~ "Usage:" ]]
}

@test "simple token comparisson" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]
}
