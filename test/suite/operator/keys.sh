. test/assert.sh

hint 'operator/keys'

# Keys Roster
pass '("$[]" {})' '[]'
pass '("$[]" {"a" ?})' '["a"]'
pass '("$[]" {"a" ? "b" ? "c" ?})' '["a" "b" "c"]'

# Argument Error
fail '("$[]")' '#4#'
fail '("$[]" ?)' '#4#'
fail '("$[]" #100#)' '#4#'
fail '("$[]" "word")' '#4#'
fail '("$[]" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("$[]" ("<>" "ERROR"))' '"ERROR"'
