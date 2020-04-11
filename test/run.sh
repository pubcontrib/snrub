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
pass '?'

# Number
pass '#0#'
pass '#1#'
pass '#10#'
pass '#01#'
pass '#-1#'

# String
pass '"word"'
pass '"more than one word"'

# Comment
pass '~("comments")'
fail '~'
fail '~()'

# Value
pass '<("key")'
fail '<'
fail '<()'

# Assign
pass '>("null" ?)'
pass '>("number" #100#)'
pass '>("string" "one hundred")'
fail '>'
fail '>()'
fail '>("key")'

# Add
pass '+(#10# #5#)'
fail '+'
fail '+()'
fail '+(#5#)'

# Subtract
pass '-(#10# #5#)'
fail '-'
fail '-()'
fail '-(#5#)'

# Multiply
pass '*(#10# #5#)'
fail '*'
fail '*()'
fail '*(#5#)'

# Divide
pass '/(#10# #5#)'
fail '/'
fail '/()'
fail '/(#5#)'

# Comments
pass '~("comments")'
fail '~'
fail '~()'

# Errors
fail '/(#1# #0#)'
