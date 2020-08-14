. test/assert.sh

hint 'operator/multiply'

# Numeric Multiplication
pass '("*" #10# #5#)' '#50#'

# Argument Error
fail '("*")' '#4#'
fail '("*" #5#)' '#4#'
fail '("*" ? ?)' '#4#'
fail '("*" "word" "word")' '#4#'

# Error Propogation
fail '("*" ("#" "text") #5#)' '#3#'
fail '("*" #10# ("#" "text"))' '#3#'
