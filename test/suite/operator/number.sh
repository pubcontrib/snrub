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

# Type Conversion Error
fail '("#" "-")' '#3#'
fail '("#" "word")' '#3#'

# Argument Mistype
fail '("#")' '#4#'

# Error Propogation
fail '("#" ("#" "text"))' '#3#'
