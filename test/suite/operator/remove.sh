. test/assert.sh

hint 'operator/remove'

# Argument Error
fail '("[o]--")' '#4#'
fail '("[o]--" ?)' '#4#'
fail '("[o]--" #100#)' '#4#'
fail '("[o]--" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("[o]--" ("#" "text"))' '#3#'
