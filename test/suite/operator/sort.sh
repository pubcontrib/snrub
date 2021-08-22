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
pass '("<|>" [{"a" ?} {"b" ?}] #0#)' '[{"a" ?} {"b" ?}]'
pass '("<|>" [{"a" ?} {"b" ?}] #1#)' '[{"b" ?} {"a" ?}]'
pass '("<|>" [{"b" ?} {"a" ?}] #0#)' '[{"a" ?} {"b" ?}]'
pass '("<|>" [{"b" ?} {"a" ?}] #1#)' '[{"b" ?} {"a" ?}]'
pass '("<|>" [{"a" #1#} {"a" #2#}] #0#)' '[{"a" #1#} {"a" #2#}]'
pass '("<|>" [{"a" #1#} {"a" #2#}] #1#)' '[{"a" #2#} {"a" #1#}]'
pass '("<|>" [{"a" #2#} {"a" #1#}] #0#)' '[{"a" #1#} {"a" #2#}]'
pass '("<|>" [{"a" #2#} {"a" #1#}] #1#)' '[{"a" #2#} {"a" #1#}]'
pass '("<|>" [{"a" ?} [#1#] "b" #2# ?] #0#)' '[? #2# "b" [#1#] {"a" ?}]'
pass '("<|>" [{"c" ?} {"b" ?} {"a" ?}
[#3#] [#2#] [#1#]
"c" "b" "a"
#3# #2# #1#
? ? ?] #0#)' '[? ? ? #1# #2# #3# "a" "b" "c" [#1#] [#2#] [#3#] {"a" ?} {"b" ?} {"c" ?}]'

# Argument Error
fail '("<|>")' '#4#'
fail '("<|>" ?)' '#4#'
fail '("<|>" ? ?)' '#4#'
fail '("<|>" ? ##)' '#4#'
fail '("<|>" ? "")' '#4#'
fail '("<|>" ? [])' '#4#'
fail '("<|>" ? {})' '#4#'
fail '("<|>" ##)' '#4#'
fail '("<|>" ## ?)' '#4#'
fail '("<|>" ## ##)' '#4#'
fail '("<|>" ## "")' '#4#'
fail '("<|>" ## [])' '#4#'
fail '("<|>" ## {})' '#4#'
fail '("<|>" "")' '#4#'
fail '("<|>" "" ?)' '#4#'
fail '("<|>" "" ##)' '#4#'
fail '("<|>" "" "")' '#4#'
fail '("<|>" "" [])' '#4#'
fail '("<|>" "" {})' '#4#'
fail '("<|>" [])' '#4#'
fail '("<|>" [] ?)' '#4#'
fail '("<|>" [] "")' '#4#'
fail '("<|>" [] [])' '#4#'
fail '("<|>" [] {})' '#4#'
fail '("<|>" {})' '#4#'
fail '("<|>" {} ?)' '#4#'
fail '("<|>" {} ##)' '#4#'
fail '("<|>" {} "")' '#4#'
fail '("<|>" {} [])' '#4#'
fail '("<|>" {} {})' '#4#'

# Error Propogation
fail '("<|>" ("#" "text") #0#)' '#3#'
fail '("<|>" [] ("#" "text"))' '#3#'
