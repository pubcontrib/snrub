. test/assert.sh

hint 'operator/type'

# Object Type
pass '("_" ?)' '?'
pass '("_" #100#)' '"##"'
pass '("_" "text")' '"\"\""'
pass '("_" [#1# #0# #0#])' '"[]"'

# Argument Error
fail '("_")' '#4#'

# Error Propogation
fail '("_" ("#" "text"))' '#3#'
