#!/usr/bin/env bats

load bats-shared

@test "shuffled numeric tokens" {
	run $EXE_FILE --shuffled-tokens --numeric --float-tolerance 0.001 $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]	
}

@test "shuffled numeric tokens (negative test)" {
	run $EXE_FILE --shuffled-tokens --numeric $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
