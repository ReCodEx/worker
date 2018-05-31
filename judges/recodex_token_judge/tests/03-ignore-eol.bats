#!/usr/bin/env bats

load bats-shared

@test "ignore line ends" {
	run $EXE_FILE --ignore-line-ends $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]
}

@test "ignore line ends (negative test)" {
	run $EXE_FILE $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}

@test "ignore line ends switch conflicts" {
	run $EXE_FILE --ignore-line-ends --ignore-empty-lines $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 2 ]
	[[ "${lines[0]}" =~ "Error:" ]]
}
