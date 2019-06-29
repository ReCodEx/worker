#!/usr/bin/env bats

load bats-shared

@test "approx" {
	run $EXE_FILE --token-lcs-approx-max-window 4 $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - "${ERROR_FILE}1"
}

@test "approx (negative test)" {
	run $EXE_FILE --token-lcs-approx-max-window 11 $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - "${ERROR_FILE}2"
}
