#!/usr/bin/env bats

load bats-shared

@test "shuffled + ignore eol + comments" {
	run $EXE_FILE --shuffled-tokens --ignore-line-ends --allow-comments $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 0 ]
	[ "${lines[0]}" -eq 1 ]	
}

@test "shuffled + ignore eol + comments (negative test)" {
	run $EXE_FILE --shuffled-tokens --allow-comments $CORRECT_FILE $RESULT_FILE
	[ "$status" -eq 1 ]
	echo "$output" | diff -abB - $ERROR_FILE
}
