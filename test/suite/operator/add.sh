. test/assert.sh

hint 'operator/add'

# Numeric Addition
pass '("+" #10# #5#)' '#15#'

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
fail '("+" #5#)' '#4#'
fail '("+" ? ?)' '#4#'

# Error Propogation
fail '("+" ("#" "text") #5#)' '#3#'
fail '("+" #10# ("#" "text"))' '#3#'
