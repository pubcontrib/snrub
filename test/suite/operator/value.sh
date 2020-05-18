#!/bin/sh
source test/assert.sh

pass '("<" "key")' '?'
pass '(">" "null" ?) ("<" "null")' '?'
pass '(">" "number" #1#) ("<" "number")' '#1#'
pass '(">" "string" "word") ("<" "string")' '"word"'
pass '(">" "key" "before") (">" "key" "after") ("<" "key")' '"after"'
pass '(">" "key" "before") (">" "key" ?) ("<" "key")' '?'
pass '(">" "key" "before") (">" "key" #1#) ("<" "key")' '#1#'
fail '("<")' '#4#'
