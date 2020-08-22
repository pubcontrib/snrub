. test/assert.sh

hint 'operator/value'

# Variable Value
pass '("<--" "key")' '?'
pass '("-->" "null" ?) ("<--" "null")' '?'
pass '("-->" "number" #1#) ("<--" "number")' '#1#'
pass '("-->" "string" "word") ("<--" "string")' '"word"'
pass '("-->" "list" [#1# #2# #3#]) ("<--" "list")' '[#1# #2# #3#]'
pass '("-->" "key" "before") ("-->" "key" "after") ("<--" "key")' '"after"'
pass '("-->" "key" "before") ("-->" "key" ?) ("<--" "key")' '?'
pass '("-->" "key" "before") ("-->" "key" #1#) ("<--" "key")' '#1#'

# Argument Error
fail '("<--")' '#4#'
fail '("<--" ?)' '#4#'
fail '("<--" #100#)' '#4#'
fail '("<--" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("<--" ("#" "text"))' '#3#'
