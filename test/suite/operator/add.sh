#!/bin/sh
. test/assert.sh

hint 'operator/add'

pass '("+" #10# #5#)' '#15#'
pass '("+" "left" "right")' '"leftright"'
pass '("+" "" "word")' '"word"'
pass '("+" "word" "")' '"word"'
pass '("+" "\t\r\n" "\\\"")' '"\t\r\n\\\""'
fail '("+")' '#4#'
fail '("+" #5#)' '#4#'
fail '("+" ? ?)' '#4#'
