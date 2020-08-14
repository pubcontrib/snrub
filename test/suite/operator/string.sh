. test/assert.sh

hint 'operator/string'

pass '("\"" ?)' '?'
pass '("\"" ##)' '"0"'
pass '("\"" #1#)' '"1"'
pass '("\"" #-1#)' '"-1"'
pass '("\"" #100#)' '"100"'
pass '("\"" #-100#)' '"-100"'
pass '("\"" "word")' '"word"'
fail '("\"")' '#4#'
fail '("\"" ("#" "text"))' '#3#'
