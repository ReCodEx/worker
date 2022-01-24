#!/usr/bin/env bats

load bats-shared

@test "numeric tokens" {
	run $EXE_FILE --numeric --float-tolerance 0.02 $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]
}

@test "numeric tokens (negative test)" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}

@test "numeric float tolerance" {
	run $EXE_FILE --numeric --float-tolerance 0.001 $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
}
