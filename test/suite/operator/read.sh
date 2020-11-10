. test/assert.sh

hint 'operator/read'

# Argument Error
fail '("^")' '#4#'
fail '("^" ?)' '#4#'
fail '("^" #100#)' '#4#'
fail '("^" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("^" ("#" "text"))' '#3#'
