TESTFILE_ID=$(basename $BATS_TEST_FILENAME | sed 's/[^0-9]//g')

CORRECT_FILE=$BATS_TEST_DIRNAME/$TESTFILE_ID.correct.in
RESULT_FILE=$BATS_TEST_DIRNAME/$TESTFILE_ID.result.in

ERROR_FILE=$BATS_TEST_DIRNAME/$TESTFILE_ID.error.out

if [ "$OS" = "Windows_NT" ]; then
	EXE_FILE='./recodex-token-judge.exe'
else
	EXE_FILE='./recodex-token-judge'
fi
