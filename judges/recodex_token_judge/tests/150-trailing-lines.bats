#!/usr/bin/env bats

load bats-shared

@test "correct file trailing lines" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - "${ERROR_FILE}1"
}

@test "result file trailing lines" {
	run $EXE_FILE $RESULT_FILE $CORRECT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - "${ERROR_FILE}2"
}
