. test/assert.sh

hint 'operator/value'

# Variable Value
pass '("x->" "key")' '?'
pass '("-->" "null" ?) ("x->" "null")' '?'
pass '("-->" "number" #1#) ("x->" "number")' '#1#'
pass '("-->" "string" "word") ("x->" "string")' '"word"'
pass '("-->" "list" [#1# #2# #3#]) ("x->" "list")' '[#1# #2# #3#]'
pass '("-->" "key" "before") ("-->" "key" "after") ("x->" "key")' '"after"'
pass '("-->" "key" "before") ("-->" "key" ?) ("x->" "key")' '?'
pass '("-->" "key" "before") ("-->" "key" #1#) ("x->" "key")' '#1#'

# Argument Error
fail '("x->")' '#4#'
fail '("x->" ?)' '#4#'
fail '("x->" #100#)' '#4#'
fail '("x->" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("x->" ("#" "text"))' '#3#'
