. test/assert.sh

hint 'operator/represent'

# Value Representation
pass '(";" ?)' '"?"'
pass '(";" #100#)' '"#100#"'
pass '(";" "word")' '"\"word\""'
pass '(";" [#1# #2# #3#])' '"[#1# #2# #3#]"'
pass '(";" {"key" "value"})' '"{\"key\" \"value\"}"'

# Argument Error
fail '(";")' '#4#'

# Error Propogation
fail '(";" ("<>" "ERROR"))' '"ERROR"'
