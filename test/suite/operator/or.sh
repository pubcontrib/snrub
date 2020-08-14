. test/assert.sh

hint 'operator/or'

# Boolean Or
pass '("|" #0# #0#)' '#0#'
pass '("|" #0# #1#)' '#1#'
pass '("|" #1# #0#)' '#1#'
pass '("|" #1# #1#)' '#1#'
pass '("|" #0# #500#)' '#1#'
pass '("|" #500# #0#)' '#1#'
pass '("|" #500# #500#)' '#1#'

# Argument Mistype
fail '("|")' '#4#'
fail '("|" #1#)' '#4#'
fail '("|" ? ?)' '#4#'
fail '("|" "word" "word")' '#4#'

# Error Propogation
fail '("|" ("#" "text") #0#)' '#3#'
fail '("|" #0# ("#" "text"))' '#3#'
