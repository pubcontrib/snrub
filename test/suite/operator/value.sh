. test/assert.sh

hint 'operator/value'

pass '("<--" "key")' '?'
pass '("-->" "null" ?) ("<--" "null")' '?'
pass '("-->" "number" #1#) ("<--" "number")' '#1#'
pass '("-->" "string" "word") ("<--" "string")' '"word"'
pass '("-->" "list" [#1# #2# #3#]) ("<--" "list")' '[#1# #2# #3#]'
pass '("-->" "key" "before") ("-->" "key" "after") ("<--" "key")' '"after"'
pass '("-->" "key" "before") ("-->" "key" ?) ("<--" "key")' '?'
pass '("-->" "key" "before") ("-->" "key" #1#) ("<--" "key")' '#1#'
fail '("<--")' '#4#'
fail '("<--" ("#" "text"))' '#3#'
