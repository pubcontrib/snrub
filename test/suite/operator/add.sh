. test/assert.sh

hint 'operator/add'

pass '("+" #10# #5#)' '#15#'

pass '("+" "left" "right")' '"leftright"'
pass '("+" "" "word")' '"word"'
pass '("+" "word" "")' '"word"'
pass '("+" "\t\r\n" "\\\"")' '"\t\r\n\\\""'

pass '("+" [#1# #2#] [#3# #4#])' '[#1# #2# #3# #4#]'
pass '("+" [] [#3# #4#])' '[#3# #4#]'
pass '("+" [#1# #2#] [])' '[#1# #2#]'
pass '("+" [] [])' '[]'

fail '("+")' '#4#'
fail '("+" #5#)' '#4#'
fail '("+" ? ?)' '#4#'

fail '("+" ("#" "text") #5#)' '#3#'
fail '("+" #10# ("#" "text"))' '#3#'
