#!/bin/sh
program=$1

if [ -z $program ]
then
    printf "Script failed! No program path given.\n"
    exit 1
fi

run_test()
{
    text=$1
    expected_output=$2
    expected_code=$3

    actual_output=`$program -t "$text"`
    actual_code=$?

    if [ $actual_code != $expected_code ]
    then
        printf "%s\n" "$text"
        printf "Test failed!\n"
        exit 1
    fi

    if [ "$actual_output" != "$expected_output" ]
    then
        printf "%s\n" "$text"
        printf "Test failed!\n"
        exit 1
    fi
}

pass()
{
    run_test "$1" "$2" 0
}

fail()
{
    run_test "$1" "$2" 1
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
fail '#word#' '#3#'

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
pass '"word	word	word"' '"word\tword\tword"'
pass '"line
line
line"' '"line\nline\nline"'
pass '"1"' '"1"'
fail '"' '#1#'
fail '"word' '#1#'
fail 'word"' '#1#'
fail "$(printf '\042\007\042')" '#3#'

# Comment
pass '("~" "comments")' '?'
fail '("~")' '#4#'

# Value
pass '("<" "key")' '?'
pass '(">" "null" ?) ("<" "null")' '?'
pass '(">" "number" #1#) ("<" "number")' '#1#'
pass '(">" "string" "word") ("<" "string")' '"word"'
pass '(">" "key" "before") (">" "key" "after") ("<" "key")' '"after"'
pass '(">" "key" "before") (">" "key" ?) ("<" "key")' '?'
fail '("<")' '#4#'

# Assign
pass '(">" "null" ?)' '?'
pass '(">" "number" #100#)' '?'
pass '(">" "string" "one hundred")' '?'
fail '(">")' '#4#'
fail '(">" "key")' '#4#'

# Add
pass '("+" #10# #5#)' '#15#'
fail '("+")' '#4#'
fail '("+" #5#)' '#4#'

# Subtract
pass '("-" #10# #5#)' '#5#'
fail '("-")' '#4#'
fail '("-" #5#)' '#4#'

# Multiply
pass '("*" #10# #5#)' '#50#'
fail '("*")' '#4#'
fail '("*" #5#)' '#4#'

# Divide
pass '("/" #10# #5#)' '#2#'
fail '("/")' '#4#'
fail '("/" #5#)' '#4#'

# Errors
fail '(' '#1#'
fail '("<"("<"("<"("<"("<"("<"("<"("<"
("<"("<"("<"("<"("<"("<"("<"("<"
("<"("<"("<"("<"("<"("<"("<"("<"
("<"("<"("<"("<"("<"("<"("<"("<"
"k"
))))))))
))))))))
))))))))
))))))))' '#2#'
fail '(">" #1# #word#)' '#3#'
fail '(">" #1# #1#)' '#4#'
fail '("/" #1# #0#)' '#5#'

# Whitespace
pass '(">""string""word")("<""string")' '"word"'
pass ' ( ">" "string" "word" ) ( "<" "string" ) ' '"word"'
pass '	(	">"	"string"	"word"	)	(	"<"	"string"	)	' '"word"'
pass '
(
">"
"string"
"word"
)
(
"<"
"string"
)
' '"word"'

# Expressions
pass '()' '?'
fail '(?)' '#6#'
fail '(#1#)' '#6#'
