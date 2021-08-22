. test/assert.sh

hint 'operator/length'

# Collection Length
pass '("| |" "")' '#0#'
pass '("| |" "1")' '#1#'
pass '("| |" "22")' '#2#'
pass '("| |" "333")' '#3#'
pass '("| |" [])' '#0#'
pass '("| |" [#1#])' '#1#'
pass '("| |" [#1# #2#])' '#2#'
pass '("| |" [#1# #2# #3#])' '#3#'
pass '("| |" {})' '#0#'
pass '("| |" {"one" #1#})' '#1#'
pass '("| |" {"one" #1# "two" #2#})' '#2#'
pass '("| |" {"one" #1# "two" #2# "three" #3#})' '#3#'

# Argument Error
fail '("| |")' '#4#'
fail '("| |" ?)' '#4#'
fail '("| |" ##)' '#4#'

# Error Propogation
fail '("| |" ("#" "text"))' '#3#'
