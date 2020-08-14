. test/assert.sh

hint 'operator/subtract'

# Numeric Subtraction
pass '("-" #10# #5#)' '#5#'

# Argument Error
fail '("-")' '#4#'
fail '("-" #5#)' '#4#'
fail '("-" ? ?)' '#4#'
fail '("-" "word" "word")' '#4#'

# Error Propogation
fail '("-" ("#" "text") #5#)' '#3#'
fail '("-" #10# ("#" "text"))' '#3#'
