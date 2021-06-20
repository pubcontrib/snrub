. test/assert.sh

hint 'operator/get'

# Value Getting
pass '("$->" "abc" #0#)' '?'
pass '("$->" "abc" #1#)' '"a"'
pass '("$->" "abc" #2#)' '"b"'
pass '("$->" "abc" #3#)' '"c"'
pass '("$->" "abc" #4#)' '?'
pass '("$->" [?] #1#)' '?'
pass '("$->" [#10#] #1#)' '#10#'
pass '("$->" ["10"] #1#)' '"10"'
pass '("$->" [[#10#]] #1#)' '[#10#]'
pass '("$->" [#10# #20# #30#] #0#)' '?'
pass '("$->" [#10# #20# #30#] #1#)' '#10#'
pass '("$->" [#10# #20# #30#] #2#)' '#20#'
pass '("$->" [#10# #20# #30#] #3#)' '#30#'
pass '("$->" [#10# #20# #30#] #4#)' '?'

# Argument Error
fail '("$->")' '#4#'
fail '("$->" ?)' '#4#'
fail '("$->" #100#)' '#4#'
fail '("$->" "word")' '#4#'
fail '("$->" [#1# #2# #3#])' '#4#'
fail '("$->" ? ?)' '#4#'
fail '("$->" ? #100#)' '#4#'
fail '("$->" ? "word")' '#4#'
fail '("$->" ? [#1# #2# #3#])' '#4#'
fail '("$->" #100# ?)' '#4#'
fail '("$->" #100# #100#)' '#4#'
fail '("$->" #100# "word")' '#4#'
fail '("$->" #100# [#1# #2# #3#])' '#4#'
fail '("$->" "word" ?)' '#4#'
fail '("$->" "word" "word")' '#4#'
fail '("$->" "word" [#1# #2# #3#])' '#4#'
fail '("$->" [#1# #2# #3#] ?)' '#4#'
fail '("$->" [#1# #2# #3#] "word")' '#4#'
fail '("$->" [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("$->" ("#" "text") #1#)' '#3#'
fail '("$->" "abc "("#" "text"))' '#3#'
