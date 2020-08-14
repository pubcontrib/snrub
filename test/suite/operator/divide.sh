. test/assert.sh

hint 'operator/divide'

# Numeric Division
pass '("/" #10# #5#)' '#2#'
pass '("/" #10# #6#)' '#1#'
pass '("/" #-10# #6#)' '#-1#'
pass '("/" #10# #-6#)' '#-1#'
pass '("/" #-10# #-6#)' '#1#'
pass '("/" #0# #1#)' '#0#'
pass '("/" #6# #10#)' '#0#'

# Argument Error
fail '("/")' '#4#'
fail '("/" #5#)' '#4#'
fail '("/" ? ?)' '#4#'
fail '("/" "word" "word")' '#4#'

# Arithmetic Error
fail '("/" #1# #0#)' '#5#'

# Error Propogation
fail '("/" ("#" "text") #5#)' '#3#'
fail '("/" #10# ("#" "text"))' '#3#'
