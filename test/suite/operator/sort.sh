. test/assert.sh

hint 'operator/sort'

# Collection Sorting
pass '("<|>" [] #0#)' '[]'
pass '("<|>" [? ? ?] #0#)' '[? ? ?]'
pass '("<|>" [#1# #2# #3#] #0#)' '[#1# #2# #3#]'
pass '("<|>" [#2# #1# #3#] #0#)' '[#1# #2# #3#]'
pass '("<|>" [#2# #1# #2#] #0#)' '[#1# #2# #2#]'
pass '("<|>" ["a" "b" "c"] #0#)' '["a" "b" "c"]'
pass '("<|>" ["b" "a" "c"] #0#)' '["a" "b" "c"]'
pass '("<|>" ["b" "a" "b"] #0#)' '["a" "b" "b"]'
pass '("<|>" [[#1#] [#2#] [#3#]] #0#)' '[[#1#] [#2#] [#3#]]'
pass '("<|>" [[#2#] [#1#] [#3#]] #0#)' '[[#1#] [#2#] [#3#]]'
pass '("<|>" [[#2#] [#1#] [#2#]] #0#)' '[[#1#] [#2#] [#2#]]'
pass '("<|>" [? #3# "b" [#1#]] #0#)' '[? #3# "b" [#1#]]'
pass '("<|>" [[#1#] "b" #3# ?] #0#)' '[? #3# "b" [#1#]]'
pass '("<|>" [? [#2#] "b" #2# [#1#] "a" #1# [#3#] "c" #3# ?] #0#)' '[? ? #1# #2# #3# "a" "b" "c" [#1#] [#2#] [#3#]]'
pass '("<|>" [? [#2#] "b" #2# [#1#] "a" #1# [#3#] "c" #3# ?] #1#)' '[[#3#] [#2#] [#1#] "c" "b" "a" #3# #2# #1# ? ?]'
pass '("<|>" [{"a" ?} {"b" ?}] #0#)' '[{"a" ?} {"b" ?}]'
pass '("<|>" [{"a" ?} {"b" ?}] #1#)' '[{"b" ?} {"a" ?}]'

# Argument Error
fail '("<|>")' '#4#'
fail '("<|>" ?)' '#4#'
fail '("<|>" #100#)' '#4#'
fail '("<|>" "word")' '#4#'
fail '("<|>" [#1# #2# #3#])' '#4#'
fail '("<|>" ? ?)' '#4#'
fail '("<|>" ? #100#)' '#4#'
fail '("<|>" ? "word")' '#4#'
fail '("<|>" ? [#1# #2# #3#])' '#4#'
fail '("<|>" #100# ?)' '#4#'
fail '("<|>" #100# #100#)' '#4#'
fail '("<|>" #100# "word")' '#4#'
fail '("<|>" #100# [#1# #2# #3#])' '#4#'
fail '("<|>" "word" ?)' '#4#'
fail '("<|>" "word" "word")' '#4#'
fail '("<|>" "word" [#1# #2# #3#])' '#4#'
fail '("<|>" [#1# #2# #3#] ?)' '#4#'
fail '("<|>" [#1# #2# #3#] "word")' '#4#'
fail '("<|>" [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("<|>" ("#" "text") #0#)' '#3#'
fail '("<|>" [] ("#" "text"))' '#3#'
