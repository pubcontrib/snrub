. test/assert.sh

hint 'operator/number'

# Number Casting
pass '("#" ?)' '?'
pass '("#" ##)' '#0#'
pass '("#" #1#)' '#1#'
pass '("#" #-1#)' '#-1#'
pass '("#" #100#)' '#100#'
pass '("#" #-100#)' '#-100#'
pass '("#" "")' '#0#'
pass '("#" "1")' '#1#'
pass '("#" "-1")' '#-1#'
pass '("#" "100")' '#100#'
pass '("#" "-100")' '#-100#'

# Type Error
fail '("#" "-")' '#3#'
fail '("#" "word")' '#3#'

# Argument Error
fail '("#")' '#4#'
fail '("#" [#1# #2# #3#])' '#7#'

# Error Propogation
fail '("#" ("#" "text"))' '#3#'
