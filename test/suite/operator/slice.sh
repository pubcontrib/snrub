. test/assert.sh

hint 'operator/slice'

# Collection Slicing
pass '("[# #]" "abcde" #1# #5#)' '"abcde"'
pass '("[# #]" "abcde" #2# #4#)' '"bcd"'
pass '("[# #]" "abcde" #3# #3#)' '"c"'
pass '("[# #]" "abcde" #-1# #1#)' '"a"'
pass '("[# #]" "abcde" #5# #7#)' '"e"'
pass '("[# #]" "abcde" #4# #2#)' '"bcd"'
pass '("[# #]" "" #-1# #1#)' '""'
pass '("[# #]" [#10# #20# #30# #40# #50#] #1# #5#)' '[#10# #20# #30# #40# #50#]'
pass '("[# #]" [#10# #20# #30# #40# #50#] #2# #4#)' '[#20# #30# #40#]'
pass '("[# #]" [#10# #20# #30# #40# #50#] #3# #3#)' '[#30#]'
pass '("[# #]" [#10# #20# #30# #40# #50#] #-1# #1#)' '[#10#]'
pass '("[# #]" [#10# #20# #30# #40# #50#] #5# #7#)' '[#50#]'
pass '("[# #]" [#10# #20# #30# #40# #50#] #4# #2#)' '[#20# #30# #40#]'
pass '("[# #]" [] #-1# #1#)' '[]'

# Argument Error
fail '("[# #]")' '#4#'
fail '("[# #]" "word")' '#4#'
fail '("[# #]" [#1# #2# #3#])' '#4#'
fail '("[# #]" "word" ?)' '#4#'
fail '("[# #]" "word" #100#)' '#4#'
fail '("[# #]" "word" "word")' '#4#'
fail '("[# #]" "word" [#1# #2# #3#])' '#4#'
fail '("[# #]" "word" ? ?)' '#4#'
fail '("[# #]" "word" ? #100#)' '#4#'
fail '("[# #]" "word" ? "word")' '#4#'
fail '("[# #]" "word" ? [#1# #2# #3#])' '#4#'
fail '("[# #]" "word" #100# ?)' '#4#'
fail '("[# #]" "word" #100# "word")' '#4#'
fail '("[# #]" "word" #100# [#1# #2# #3#])' '#4#'
fail '("[# #]" "word" "word" ?)' '#4#'
fail '("[# #]" "word" "word" #100#)' '#4#'
fail '("[# #]" "word" "word" "word")' '#4#'
fail '("[# #]" "word" "word" [#1# #2# #3#])' '#4#'
fail '("[# #]" "word" [#1# #2# #3#] ?)' '#4#'
fail '("[# #]" "word" [#1# #2# #3#] #100#)' '#4#'
fail '("[# #]" "word" [#1# #2# #3#] "word")' '#4#'
fail '("[# #]" "word" [#1# #2# #3#] [#1# #2# #3#])' '#4#'
fail '("[# #]" [#1# #2# #3#] ?)' '#4#'
fail '("[# #]" [#1# #2# #3#] #100#)' '#4#'
fail '("[# #]" [#1# #2# #3#] "word")' '#4#'
fail '("[# #]" [#1# #2# #3#] [#1# #2# #3#])' '#4#'
fail '("[# #]" [#1# #2# #3#] ? ?)' '#4#'
fail '("[# #]" [#1# #2# #3#] ? #100#)' '#4#'
fail '("[# #]" [#1# #2# #3#] ? "word")' '#4#'
fail '("[# #]" [#1# #2# #3#] ? [#1# #2# #3#])' '#4#'
fail '("[# #]" [#1# #2# #3#] #100# ?)' '#4#'
fail '("[# #]" [#1# #2# #3#] #100# "word")' '#4#'
fail '("[# #]" [#1# #2# #3#] #100# [#1# #2# #3#])' '#4#'
fail '("[# #]" [#1# #2# #3#] "word" ?)' '#4#'
fail '("[# #]" [#1# #2# #3#] "word" #100#)' '#4#'
fail '("[# #]" [#1# #2# #3#] "word" "word")' '#4#'
fail '("[# #]" [#1# #2# #3#] "word" [#1# #2# #3#])' '#4#'
fail '("[# #]" [#1# #2# #3#] [#1# #2# #3#] ?)' '#4#'
fail '("[# #]" [#1# #2# #3#] [#1# #2# #3#] #100#)' '#4#'
fail '("[# #]" [#1# #2# #3#] [#1# #2# #3#] "word")' '#4#'
fail '("[# #]" [#1# #2# #3#] [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("[# #]" ("#" "text") #2# #4#)' '#3#'
fail '("[# #]" "abcde" ("#" "text") #4#)' '#3#'
fail '("[# #]" "abcde" #2# ("#" "text"))' '#3#'