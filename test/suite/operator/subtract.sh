. test/assert.sh

hint 'operator/subtract'

# Numeric Subtraction
pass '("-" #1# #5#)' '#-4#'
pass '("-" #0# #5#)' '#-5#'
pass '("-" #5# #10#)' '#-5#'
pass '("-" #-5# #10#)' '#-15#'
pass '("-" #5# #-10#)' '#15#'
pass '("-" #-5# #-10#)' '#5#'
pass '("-" #10# #5#)' '#5#'
pass '("-" #-10# #5#)' '#-15#'
pass '("-" #10# #-5#)' '#15#'
pass '("-" #-10# #-5#)' '#-5#'
pass '("-" #999999999# #0#)' '#999999999#'
pass '("-" #-999999999# #0#)' '#-999999999#'
pass '("-" #-999999999# #-999999999#)' '#0#'

# Arithmetic Error
fail '("-" #999999999# #-1#)' '#2#'
fail '("-" #-999999999# #1#)' '#2#'
fail '("-" #-999999999# #999999999#)' '#2#'

# Argument Error
fail '("-")' '#4#'
fail '("-" ?)' '#4#'
fail '("-" ? ?)' '#4#'
fail '("-" ? ##)' '#4#'
fail '("-" ? "")' '#4#'
fail '("-" ? [])' '#4#'
fail '("-" ? {})' '#4#'
fail '("-" ##)' '#4#'
fail '("-" ## ?)' '#4#'
fail '("-" ## "")' '#4#'
fail '("-" ## [])' '#4#'
fail '("-" ## {})' '#4#'
fail '("-" "")' '#4#'
fail '("-" "" ?)' '#4#'
fail '("-" "" ##)' '#4#'
fail '("-" "" "")' '#4#'
fail '("-" "" [])' '#4#'
fail '("-" "" {})' '#4#'
fail '("-" [])' '#4#'
fail '("-" [] ?)' '#4#'
fail '("-" [] ##)' '#4#'
fail '("-" [] "")' '#4#'
fail '("-" [] [])' '#4#'
fail '("-" [] {})' '#4#'
fail '("-" {})' '#4#'
fail '("-" {} ?)' '#4#'
fail '("-" {} ##)' '#4#'
fail '("-" {} "")' '#4#'
fail '("-" {} [])' '#4#'
fail '("-" {} {})' '#4#'

# Error Propogation
fail '("-" ("#" "text") #5#)' '#3#'
fail '("-" #10# ("#" "text"))' '#3#'
