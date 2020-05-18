#!/bin/sh
. test/assert.sh

hint 'type/string'

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
