. test/assert.sh

hint 'operator/represent'

# Value Representation
pass '(";" ?)' '"?"'
pass '(";" #100#)' '"#100#"'
pass '(";" "text")' '"\"text\""'
pass '(";" [#1# #0# #0#])' '"[#1# #0# #0#]"'
pass '(";" {"key" "value"})' '"{\"key\" \"value\"}"'

# Argument Error
fail '(";")' '#4#'

# Error Propogation
fail '(";" ("#" "text"))' '#3#'
