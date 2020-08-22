. test/assert.sh

hint 'operator/multiply'

# Numeric Multiplication
pass '("*" #10# #5#)' '#50#'

# Argument Error
fail '("*")' '#4#'
fail '("*" ?)' '#4#'
fail '("*" #100#)' '#4#'
fail '("*" "word")' '#4#'
fail '("*" [#1# #2# #3#])' '#4#'
fail '("*" ? ?)' '#4#'
fail '("*" ? #100#)' '#4#'
fail '("*" ? "word")' '#4#'
fail '("*" ? [#1# #2# #3#])' '#4#'
fail '("*" #100# ?)' '#4#'
fail '("*" #100# "word")' '#4#'
fail '("*" #100# [#1# #2# #3#])' '#4#'
fail '("*" "word" ?)' '#4#'
fail '("*" "word" #100#)' '#4#'
fail '("*" "word" "word")' '#4#'
fail '("*" "word" [#1# #2# #3#])' '#4#'
fail '("*" [#1# #2# #3#] ?)' '#4#'
fail '("*" [#1# #2# #3#] #100#)' '#4#'
fail '("*" [#1# #2# #3#] "word")' '#4#'
fail '("*" [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("*" ("#" "text") #5#)' '#3#'
fail '("*" #10# ("#" "text"))' '#3#'
