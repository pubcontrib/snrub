. test/assert.sh

hint 'operator/string'

# String Casting
pass '("\"" ?)' '?'
pass '("\"" ##)' '"0"'
pass '("\"" #1#)' '"1"'
pass '("\"" #-1#)' '"-1"'
pass '("\"" #100#)' '"100"'
pass '("\"" #-100#)' '"-100"'
pass '("\"" "word")' '"word"'

# Argument Error
fail '("\"")' '#4#'
fail '("\"" [])' '#4#'
fail '("\"" {})' '#4#'

# Error Propogation
fail '("\"" ("<>" "ERROR"))' '"ERROR"'
