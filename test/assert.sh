#!/bin/sh
run_test()
{
    text=$1
    expected_output=$2
    expected_code=$3

    actual_output=`$PROGRAM -t "$text"`
    actual_code=$?

    if [ $actual_code != $expected_code ]
    then
        printf "%s\n" "$text"
        printf "%s\n" "$hint"
        printf "Test failed!\n"
        exit 1
    fi

    if [ "$actual_output" != "$expected_output" ]
    then
        printf "%s\n" "$text"
        printf "%s\n" "$hint"
        printf "Test failed!\n"
        exit 1
    fi

    count=$((count + 1))
}

pass()
{
    run_test "$1" "$2" 0
}

fail()
{
    run_test "$1" "$2" 1
}

print_results()
{
    printf "%d tests run.\n" $count
}

hint()
{
    hint=$1
}
