. test/assert.sh

hint 'operator/add'

# Numeric Addition
pass '("+" #1# #5#)' '#6#'
pass '("+" #0# #5#)' '#5#'
pass '("+" #5# #10#)' '#15#'
pass '("+" #-5# #10#)' '#5#'
pass '("+" #5# #-10#)' '#-5#'
pass '("+" #-5# #-10#)' '#-15#'
pass '("+" #10# #5#)' '#15#'
pass '("+" #-10# #5#)' '#-5#'
pass '("+" #10# #-5#)' '#5#'
pass '("+" #-10# #-5#)' '#-15#'
pass '("+" #999999999# #0#)' '#999999999#'
pass '("+" #-999999999# #0#)' '#-999999999#'
pass '("+" #999999999# #-999999999#)' '#0#'

# String Concatenation
pass '("+" "left" "right")' '"leftright"'
pass '("+" "" "word")' '"word"'
pass '("+" "word" "")' '"word"'
pass '("+" "\t\r\n" "\\\"")' '"\t\r\n\\\""'

# List Concatenation
pass '("+" [#1# #2#] [#3# #4#])' '[#1# #2# #3# #4#]'
pass '("+" [] [#3# #4#])' '[#3# #4#]'
pass '("+" [#1# #2#] [])' '[#1# #2#]'
pass '("+" [] [])' '[]'

# Argument Error
fail '("+")' '#4#'
fail '("+" ?)' '#4#'
fail '("+" #100#)' '#4#'
fail '("+" "word")' '#4#'
fail '("+" [#1# #2# #3#])' '#4#'
fail '("+" ? ?)' '#4#'
fail '("+" ? #100#)' '#4#'
fail '("+" ? "word")' '#4#'
fail '("+" ? [#1# #2# #3#])' '#4#'
fail '("+" #100# ?)' '#4#'
fail '("+" #100# "word")' '#4#'
fail '("+" #100# [#1# #2# #3#])' '#4#'
fail '("+" "word" ?)' '#4#'
fail '("+" "word" #100#)' '#4#'
fail '("+" "word" [#1# #2# #3#])' '#4#'
fail '("+" [#1# #2# #3#] ?)' '#4#'
fail '("+" [#1# #2# #3#] #100#)' '#4#'
fail '("+" [#1# #2# #3#] "word")' '#4#'

# Arithmetic Error
fail '("+" #999999999# #1#)' '#5#'
fail '("+" #-999999999# #-1#)' '#5#'
fail '("+" #999999999# #999999999#)' '#5#'

# Error Propogation
fail '("+" ("#" "text") #5#)' '#3#'
fail '("+" #10# ("#" "text"))' '#3#'
