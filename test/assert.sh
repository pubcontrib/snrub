#!/bin/sh
hint()
{
    hint=$1
}

pass()
{
    run_test "$1" "$2" 0
}

fail()
{
    run_test "$1" "$2" 1
}

conclude()
{
    printf "%d tests run.\n" $count
    count=0
}

run_test()
{
    text=$1
    expected_output=$2
    expected_code=$3

    actual_output=`$PROGRAM -t "$text"`
    actual_code=$?

    if [ $actual_code != $expected_code ]
    then
        printf "[ERROR] Test case returned an unexpected exit code.\n"
        printf "Hint: %s\n" "$hint"
        printf "Source: %s\n" "$text"
        printf "Expected: %d\n" $expected_code
        printf "Actual: %d\n" $actual_code
        exit 1
    fi

    if [ "$actual_output" != "$expected_output" ]
    then
        printf "[ERROR] Test case returned an unexpected stdout.\n"
        printf "Hint: %s\n" "$hint"
        printf "Source: %s\n" "$text"
        printf "Expected: %s\n" "$expected_output"
        printf "Actual: %s\n" "$actual_output"
        exit 1
    fi

    count=$((count + 1))
}
