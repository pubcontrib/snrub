. test/assert.sh

hint 'operator/type'

# Value Type
pass '("_" ?)' '"?"'
pass '("_" #100#)' '"##"'
pass '("_" "text")' '"\"\""'
pass '("_" [#1# #0# #0#])' '"[]"'
pass '("_" {"key" "value"})' '"{}"'

# Argument Error
fail '("_")' '#4#'

# Error Propogation
fail '("_" ("#" "text"))' '#3#'
