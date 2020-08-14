. test/assert.sh

hint 'type/list'

# List Literal
pass '[]' '[]'
pass '[ ]' '[]'
pass '[? ? ?]' '[? ? ?]'
pass '[#1# #2# #3#]' '[#1# #2# #3#]'
pass '["one" "two" "three"]' '["one" "two" "three"]'
pass '[[#1#] [#2# #2#] [#3# #3# #3#]]' '[[#1#] [#2# #2#] [#3# #3# #3#]]'
pass '[? #2# "three" [#4# #4# #4# #4#]]' '[? #2# "three" [#4# #4# #4# #4#]]'
pass '[("#" "10") ("#" "20") ("#" "30")]' '[#10# #20# #30#]'

# Error Propogation
fail '[("#" "10") ("#" "text") ("#" "30")]' '#3#'

# Syntax Error
fail '[' '#1#'
fail ']' '#1#'
