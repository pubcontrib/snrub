#!/bin/sh
program=$1

if [ -z $program ]
then
    echo "Script failed! No program path given."
    exit 1
fi

execute_text()
{
    text=$1
    output=`$program -t "$text"`
    exit_code=$?
}

pass()
{
    text=$1
    expected=$2

    execute_text "$text"

    if [ $exit_code -ne 0 ]
    then
        echo "$text"
        echo "Test failed! Expected pass."
        exit 1
    fi

    if [ "$output" != "$expected" ]
    then
        echo "$text"
        echo "Test failed! Expected pass."
        exit 1
    fi
}

fail()
{
    text=$1
    expected=$2

    execute_text "$text"

    if [ $exit_code -eq 0 ]
    then
        echo "$text"
        echo "Test failed! Expected fail."
        exit 1
    fi

    if [ "$output" != "$expected" ]
    then
        echo "$text"
        echo "Test failed! Expected fail."
        exit 1
    fi
}

# Null
pass '?' '?'
pass '' '?'
pass ' ' '?'

# Number
pass '##' '#0#'
pass '#0#' '#0#'
pass '#1#' '#1#'
pass '#10#' '#10#'
pass '#01#' '#1#'
pass '#-1#' '#-1#'
fail '#' '#1#'
fail '#1' '#1#'
fail '1#' '#1#'

# String
pass '""' '""'
pass '" "' '" "'
pass '"\\"' '"\\"'
pass '"\""' '"\""'
pass '"\t"' '"\t"'
pass '"\n"' '"\n"'
pass '"\r"' '"\r"'
pass '"\z"' '""'
pass '"\\ \" \t \n \r"' '"\\ \" \t \n \r"'
pass '"\t\"line\"\n"' '"\t\"line\"\n"'
pass '"word"' '"word"'
pass '"word word word"' '"word word word"'
fail '"' '#1#'
fail '"word' '#1#'
fail 'word"' '#1#'

# Comment
pass '~("comments")' '?'
fail '~' '#1#'
fail '~()' '#4#'

# Value
pass '<("key")' '?'
pass '>("null" ?) <("null")' '?'
pass '>("number" #1#) <("number")' '#1#'
pass '>("string" "word") <("string")' '"word"'
pass '>("key" "before") >("key" "after") <("key")' '"after"'
pass '>("key" "before") >("key" ?) <("key")' '?'
fail '<' '#1#'
fail '<()' '#4#'

# Assign
pass '>("null" ?)' '?'
pass '>("number" #100#)' '?'
pass '>("string" "one hundred")' '?'
fail '>' '#1#'
fail '>()' '#4#'
fail '>("key")' '#4#'

# Add
pass '+(#10# #5#)' '#15#'
fail '+' '#1#'
fail '+()' '#4#'
fail '+(#5#)' '#4#'

# Subtract
pass '-(#10# #5#)' '#5#'
fail '-' '#1#'
fail '-()' '#4#'
fail '-(#5#)' '#4#'

# Multiply
pass '*(#10# #5#)' '#50#'
fail '*' '#1#'
fail '*()' '#4#'
fail '*(#5#)' '#4#'

# Divide
pass '/(#10# #5#)' '#2#'
fail '/' '#1#'
fail '/()' '#4#'
fail '/(#5#)' '#4#'

# Errors
fail '/(#1# #0#)' '#5#'

# Whitespace
pass '>("string""word")<("string")' '"word"'
pass ' >("string" "word") <("string") ' '"word"'
pass '
>("string"
"word")
<("string")
' '"word"'
