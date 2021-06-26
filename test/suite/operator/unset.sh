. test/assert.sh

hint 'operator/unset'

# Value Unsetting
pass '("$--" "abc" #0#)' '"abc"'
pass '("$--" "abc" #1#)' '"bc"'
pass '("$--" "abc" #2#)' '"ac"'
pass '("$--" "abc" #3#)' '"ab"'
pass '("$--" "abc" #4#)' '"abc"'
pass '("$--" [?] #1#)' '[]'
pass '("$--" [#10#] #1#)' '[]'
pass '("$--" ["10"] #1#)' '[]'
pass '("$--" [[#10#]] #1#)' '[]'
pass '("$--" [#10# #20# #30#] #0#)' '[#10# #20# #30#]'
pass '("$--" [#10# #20# #30#] #1#)' '[#20# #30#]'
pass '("$--" [#10# #20# #30#] #2#)' '[#10# #30#]'
pass '("$--" [#10# #20# #30#] #3#)' '[#10# #20#]'
pass '("$--" [#10# #20# #30#] #4#)' '[#10# #20# #30#]'

# Argument Error
fail '("$--")' '#4#'
fail '("$--" ?)' '#4#'
fail '("$--" #100#)' '#4#'
fail '("$--" "word")' '#4#'
fail '("$--" [#1# #2# #3#])' '#4#'
fail '("$--" ? ?)' '#4#'
fail '("$--" ? #100#)' '#4#'
fail '("$--" ? "word")' '#4#'
fail '("$--" ? [#1# #2# #3#])' '#4#'
fail '("$--" #100# ?)' '#4#'
fail '("$--" #100# #100#)' '#4#'
fail '("$--" #100# "word")' '#4#'
fail '("$--" #100# [#1# #2# #3#])' '#4#'
fail '("$--" "word" ?)' '#4#'
fail '("$--" "word" "word")' '#4#'
fail '("$--" "word" [#1# #2# #3#])' '#4#'
fail '("$--" [#1# #2# #3#] ?)' '#4#'
fail '("$--" [#1# #2# #3#] "word")' '#4#'
fail '("$--" [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("$--" ("#" "text") #1#)' '#3#'
fail '("$--" "abc" ("#" "text"))' '#3#'
