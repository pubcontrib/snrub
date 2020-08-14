. test/assert.sh

hint 'operator/modulo'

# Numeric Modulo
pass '("%" #10# #5#)' '#0#'
pass '("%" #10# #6#)' '#4#'
pass '("%" #-10# #6#)' '#-4#'
pass '("%" #10# #-6#)' '#4#'
pass '("%" #-10# #-6#)' '#-4#'
pass '("%" #0# #1#)' '#0#'
pass '("%" #6# #10#)' '#6#'

# Argument Error
fail '("%")' '#4#'
fail '("%" #5#)' '#4#'
fail '("%" ? ?)' '#4#'
fail '("%" "word" "word")' '#4#'

# Arithmetic Error
fail '("%" #1# #0#)' '#5#'

# Error Propogation
fail '("%" ("#" "text") #5#)' '#3#'
fail '("%" #10# ("#" "text"))' '#3#'
