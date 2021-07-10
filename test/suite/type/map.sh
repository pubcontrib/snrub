. test/assert.sh

hint 'type/map'

# Map Literal
pass '{}' '{}'
pass '{ }' '{}'
pass '{"null" ?}' '{"null" ?}'
pass '{"number" #1#}' '{"number" #1#}'
pass '{"string" "one"}' '{"string" "one"}'
pass '{"list" [#1# #2# #3#]}' '{"list" [#1# #2# #3#]}'
pass '{"map" {"list" [#1# #2# #3#] "null" ? "number" #1# "string" "one"}}' '{"map" {"list" [#1# #2# #3#] "null" ? "number" #1# "string" "one"}}'
pass '{"list" [#1# #2# #3#] "null" ? "number" #1# "string" "one"}' '{"list" [#1# #2# #3#] "null" ? "number" #1# "string" "one"}'
pass '{"string" "one" "number" #1# "null" ? "list" [#1# #2# #3#]}' '{"list" [#1# #2# #3#] "null" ? "number" #1# "string" "one"}'
pass '{("\"" #1#) #1#}' '{"1" #1#}'
pass '{"1" ("#" "1")}' '{"1" #1#}'

# Syntax Error
fail '{' '#1#'
fail '}' '#1#'

# Argument Error
fail '{""}' '#4#'
fail '{? "value"}' '#4#'
fail '{## "value"}' '#4#'
fail '{[] "value"}' '#4#'
fail '{{} "value"}' '#4#'

# Error Propogation
fail '{("#" "text") "value"}' '#3#'
fail '{"key" ("#" "text")}' '#3#'
