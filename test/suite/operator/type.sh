. test/assert.sh

hint 'operator/type'

pass '("_" ?)' '?'
pass '("_" #100#)' '"##"'
pass '("_" "text")' '"\"\""'
pass '("_" [#1# #0# #0#])' '"[]"'

fail '("_")' '#4#'

fail '("_" ("#" "text"))' '#3#'
