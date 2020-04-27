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

equal()
{
    text=$1
    expected=$2

    execute_text "$text"

    if [ $exit_code -ne 0 ]
    then
        echo "$text"
        echo "Test failed! Expected equal."
        exit 1
    fi

    if [ "$output" != "$expected" ]
    then
        echo "$text"
        echo "Test failed! Expected equal."
        exit 1
    fi
}

pass()
{
    text=$1

    execute_text "$text"

    if [ $exit_code -ne 0 ]
    then
        echo "$text"
        echo "Test failed! Expected pass."
        exit 1
    fi
}

fail()
{
    text=$1

    execute_text "$text"

    if [ $exit_code -eq 0 ]
    then
        echo "$text"
        echo "Test failed! Expected fail."
        exit 1
    fi
}

# Null
equal '?' '?'
equal '' '?'
equal ' ' '?'

# Number
equal '##' '#0#'
equal '#0#' '#0#'
equal '#1#' '#1#'
equal '#10#' '#10#'
equal '#01#' '#1#'
equal '#-1#' '#-1#'
fail '#'
fail '#1'
fail '1#'

# String
equal '""' '""'
equal '" "' '" "'
equal '"\\"' '"\\"'
equal '"\""' '"\""'
equal '"\t"' '"\t"'
equal '"\n"' '"\n"'
equal '"\r"' '"\r"'
equal '"\z"' '""'
equal '"\\ \" \t \n \r"' '"\\ \" \t \n \r"'
equal '"\t\"line\"\n"' '"\t\"line\"\n"'
equal '"word"' '"word"'
equal '"word word word"' '"word word word"'
fail '"'
fail '"word'
fail 'word"'

# Comment
equal '~("comments")' '?'
fail '~'
fail '~()'

# Value
equal '<("key")' '?'
equal '>("null" ?) <("null")' '?'
equal '>("number" #1#) <("number")' '#1#'
equal '>("string" "word") <("string")' '"word"'
equal '>("key" "before") >("key" "after") <("key")' '"after"'
equal '>("key" "before") >("key" ?) <("key")' '?'
fail '<'
fail '<()'

# Assign
equal '>("null" ?)' '?'
equal '>("number" #100#)' '?'
equal '>("string" "one hundred")' '?'
fail '>'
fail '>()'
fail '>("key")'

# Add
equal '+(#10# #5#)' '#15#'
fail '+'
fail '+()'
fail '+(#5#)'

# Subtract
equal '-(#10# #5#)' '#5#'
fail '-'
fail '-()'
fail '-(#5#)'

# Multiply
equal '*(#10# #5#)' '#50#'
fail '*'
fail '*()'
fail '*(#5#)'

# Divide
equal '/(#10# #5#)' '#2#'
fail '/'
fail '/()'
fail '/(#5#)'

# Errors
fail '/(#1# #0#)'

# Whitespace
equal '>("string""word")<("string")' '"word"'
equal ' >("string" "word") <("string") ' '"word"'
equal $'\t>("string"\t"word")\t<("string")\t' '"word"'
equal $'\n>("string"\n"word")\n<("string")\n' '"word"'
equal $'\r>("string"\r"word")\r<("string")\r' '"word"'
