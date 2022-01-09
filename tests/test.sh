#!/bin/bash

# ensure tests are built
pushd ..
	bash build.sh T
popd
echo "------------------- TESTS BUILT -------------------"
echo

# run tests
pushd ../cmake/build

RED='\033[0;31m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
GRAY='\033[1;30m'
NC='\033[0m' # No Color

VERBOSE=1

# SCRIPT CONSTANTS

TESTS_DIR="../../tests"
TEST_SECTIONS=$(ls "$TESTS_DIR")
EXTENSION="cc"

# tempfiles and cleanup
TMP_STDOUT="$(pwd)/$(mktemp ./tmp_out.XXX)"
TMP_STDERR="$(pwd)/$(mktemp ./tmp_err.XXX)"
TMP_RETURN="$(pwd)/$(mktemp ./tmp_ret.XXX)"
cleanup(){ rm -f "$TMP_STDOUT" "$TMP_STDERR" "$TMP_RETURN"; }
trap "cleanup" EXIT SIGINT



usage(){
	echo "Usage: $0 [-hv]"
	echo "       run tests"
}

display(){
	if [ $# -eq 1 ]; then
		LEVEL=0
		STRING="$1"
	elif [ $# -eq 2 ]; then
		LEVEL="$1"
		STRING="$2"
	fi

	if [ $VERBOSE -gt $LEVEL ]; then
		echo -e "$STRING"
	fi
}

display_sameline(){
	if [ $# -eq 1 ]; then
		LEVEL=0
		STRING="$1"
	elif [ $# -eq 2 ]; then
		LEVEL="$1"
		STRING="$2"
	fi

	if [ $VERBOSE -gt $LEVEL ]; then
		echo -ne "$STRING"\\r
	fi
}

source_if_exists(){
	if [ -f "$1" ]; then
		source "$1" > /dev/null
	fi
}


run_test(){
  # clear database before each tests
  clear_db
	TEST_SECTION="$1"
	TEST_NAME="$2"
	TEST_NUMBER="$3"
	EXEC="./$TEST_NAME"

	# if no command file, don't do test
	if [ ! -f "$EXEC" ]; then
		echo "ERROR: $EXEC not found" >&2
		return 2
	fi

	# get expected return code, or 0 as default
	EXPECTED_RET=0


	display_sameline "$TEST_NUMBER. [ .... ] ${YELLOW}$TEST_NAME${NC}"

	# run command
	(
		# run command, capture stdout, stderr
		"$EXEC" > "$TMP_STDOUT" 2> "$TMP_STDERR"
		# capture return code
		echo $? > "$TMP_RETURN"
	)

	OUTPUT=$(cat "$TMP_STDOUT")
	ERROR_OUTPUT=$(cat "$TMP_STDERR")
	RET=$(cat "$TMP_RETURN")

	# compare return code
	if [[ "$RET" != "$EXPECTED_RET" ]]; then
		display "$TEST_NUMBER. [${RED}FAILED${NC}] ${YELLOW}$TEST_NAME${NC}"
		if [ ! -z "$ERROR_OUTPUT" ]; then display "${RED}STDERR:${NC} $ERROR_OUTPUT"; display ""; fi
		return 1
	fi

	# if we reached here, test passed
	display "$TEST_NUMBER. [${GREEN}PASSED${NC}] ${YELLOW}$TEST_NAME${NC}"
}

test_section() {
	SECTION=$1

	# parse test names
	TESTS_IN_SECTION=$(ls "$TESTS_DIR/$SECTION" | grep ".cc")
	TESTS_IN_SECTION=$(for t in ${TESTS_IN_SECTION}; do echo "${t%.$EXTENSION}"; done)

	# build tests
	BUILD_FAILED=0
	display_sameline "[   BUILDING...   ]"
	for t in ${TESTS_IN_SECTION}; do
		make "$t" > "/dev/null" 2> "$TMP_STDERR"
		RET=$?

		# check if build failed

		if [ ! "$RET" -eq  0 ]; then
			BUILD_FAILED=1
			display "[${RED}BUILD FAILURE${NC}] ${YELLOW}$t${NC}"
			cat "$TMP_STDERR"
		fi
	done

	# don't run tests if build failed
	if [ $BUILD_FAILED -eq 1 ]; then
		return 1
	fi
	display "[${GREEN}BUILD SUCCESSFUL${NC}]"

	# run tests
	for t in ${TESTS_IN_SECTION}; do
		run_test "$SECTION" "$t" "$TEST_NUMBER"
		RET=$?

		# udpate passed and failed
		if [ "$RET" -eq  0 ]; then
			NUM_PASSED=$[$NUM_PASSED+1]
		elif [ "$RET" -eq  1 ]; then
			NUM_FAILED=$[$NUM_FAILED+1]
		fi

		TEST_NUMBER=$[$TEST_NUMBER+1]
	done
}

clean_all_tests() {
	echo "Removing all test executables..."
	for SECTION in $TEST_SECTIONS; do
		TESTS_IN_SECTION=$(ls "$TESTS_DIR/$SECTION" | grep ".cc")
		TESTS_IN_SECTION=$(for t in ${TESTS_IN_SECTION}; do echo "${t%.$EXTENSION}"; done)
		for t in ${TESTS_IN_SECTION}; do
			TEST_FILE="$TESTS_DIR/$SECTION/$t"
			if [ -f "$TEST_FILE" ]; then
				echo "rm -f $TEST_FILE"
				rm "$TEST_FILE"
			fi
		done
	done
}

clear_db() {
  rm -rf db*
}

for arg in "$@"
do
    case $arg in
        -q|--quiet)
        VERBOSE=0
        shift # Remove --initialize from processing
        ;;
        -c|--clean)
        clean_all_tests && exit 0
        shift # Remove --initialize from processing
        ;;
        -h|--help)
        usage && exit 0
        shift # Remove argument name from processing
        ;;
        *)
        POSITIONAL+=("$1")
        shift # Remove generic argument from processing
        ;;
    esac
done

set -- "${POSITIONAL[@]}" # restore positional parameters

NUM_PASSED=0
NUM_FAILED=0
TEST_NUMBER=1


for s in $TEST_SECTIONS; do
	TEST_SPLITTER=" ${BLUE}===${NC} ${YELLOW}$s${NC} ${BLUE}===${NC}"
	display "$TEST_SPLITTER"
	test_section $s
	display ""
done

SPLITTER="${BLUE}=======================${NC}"

echo -e "$SPLITTER"
echo -e "${NC}Tests Passed:${NC} ${YELLOW}$NUM_PASSED${NC} / ${YELLOW}$[$NUM_PASSED+NUM_FAILED]${NC}"
echo -e "$SPLITTER"

popd

if [ "$NUM_FAILED" -ne "0" ]; then
	exit 1
fi

echo "----------"
