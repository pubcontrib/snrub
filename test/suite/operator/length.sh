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

# Argument Mistype
fail '("| |")' '#4#'
fail '("| |" ?)' '#4#'
fail '("| |" #100#)' '#4#'

# Error Propogation
fail '("| |" ("#" "text"))' '#3#'
