. test/assert.sh

hint 'operator/set'

# Value Setting
pass '("$<-" "abc" #0# "x")' '"abc"'
pass '("$<-" "abc" #1# "x")' '"xbc"'
pass '("$<-" "abc" #2# "x")' '"axc"'
pass '("$<-" "abc" #3# "x")' '"abx"'
pass '("$<-" "abc" #4# "x")' '"abc"'
pass '("$<-" [#10#] #1# ?)' '[?]'
pass '("$<-" [?] #1# #10#)' '[#10#]'
pass '("$<-" [?] #1# "10")' '["10"]'
pass '("$<-" [?] #1# [#10#])' '[[#10#]]'
pass '("$<-" [#10# #20# #30#] #0# #99#)' '[#10# #20# #30#]'
pass '("$<-" [#10# #20# #30#] #1# #99#)' '[#99# #20# #30#]'
pass '("$<-" [#10# #20# #30#] #2# #99#)' '[#10# #99# #30#]'
pass '("$<-" [#10# #20# #30#] #3# #99#)' '[#10# #20# #99#]'
pass '("$<-" [#10# #20# #30#] #4# #99#)' '[#10# #20# #30#]'

# Argument Error
fail '("$<-")' '#4#'
fail '("$<-" ?)' '#4#'
fail '("$<-" #100#)' '#4#'
fail '("$<-" "word")' '#4#'
fail '("$<-" [#1# #2# #3#])' '#4#'
fail '("$<-" ? ?)' '#4#'
fail '("$<-" ? #100#)' '#4#'
fail '("$<-" ? "word")' '#4#'
fail '("$<-" ? [#1# #2# #3#])' '#4#'
fail '("$<-" #100# ?)' '#4#'
fail '("$<-" #100# #100#)' '#4#'
fail '("$<-" #100# "word")' '#4#'
fail '("$<-" #100# [#1# #2# #3#])' '#4#'
fail '("$<-" "word" ?)' '#4#'
fail '("$<-" "word" #100#)' '#4#'
fail '("$<-" "word" "word")' '#4#'
fail '("$<-" "word" [#1# #2# #3#])' '#4#'
fail '("$<-" [#1# #2# #3#] ?)' '#4#'
fail '("$<-" [#1# #2# #3#] #100#)' '#4#'
fail '("$<-" [#1# #2# #3#] "word")' '#4#'
fail '("$<-" [#1# #2# #3#] [#1# #2# #3#])' '#4#'
fail '("$<-" "word" #100# ?)' '#4#'
fail '("$<-" "word" #100# #100#)' '#4#'
fail '("$<-" "word" #100# [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("$<-" ("#" "text") #1# "x")' '#3#'
fail '("$<-" "abc" ("#" "text") "x")' '#3#'
fail '("$<-" "abc" #1# ("#" "text"))' '#3#'
fail '("$<-" [#10# #20# #30#] #1# ("#" "text"))' '#3#'
