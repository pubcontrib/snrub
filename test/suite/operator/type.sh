. test/assert.sh

hint 'operator/type'

# Object Typing
pass '("_" ?)' '?'
pass '("_" #100#)' '"##"'
pass '("_" "text")' '"\"\""'
pass '("_" [#1# #0# #0#])' '"[]"'

# Argument Mistype
fail '("_")' '#4#'

# Error Propogation
fail '("_" ("#" "text"))' '#3#'
