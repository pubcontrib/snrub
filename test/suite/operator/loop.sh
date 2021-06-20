. test/assert.sh

hint 'operator/loop'

# Conditional Looping
pass '("x<-" "count" #0#)
("o"
    ("<" ("x->" "count") #10#)
    ("x<-" "count" ("+" ("x->" "count") #1#))
)' '?'
pass '("x<-" "count" #0#)
("o"
    ("<" ("x->" "count") #10#)
    ("x<-" "count" ("+" ("x->" "count") #1#))
)
("x->" "count")' '#10#'

# Argument Error
fail '("o")' '#4#'
fail '("o" ?)' '#4#'
fail '("o" #100#)' '#4#'
fail '("o" "word")' '#4#'
fail '("o" [#1# #2# #3#])' '#4#'
fail '("o" ? ?)' '#4#'
fail '("o" ? #100#)' '#4#'
fail '("o" ? "word")' '#4#'
fail '("o" ? [#1# #2# #3#])' '#4#'
fail '("o" "word" ?)' '#4#'
fail '("o" "word" #100#)' '#4#'
fail '("o" "word" "word")' '#4#'
fail '("o" "word" [#1# #2# #3#])' '#4#'
fail '("o" [#1# #2# #3#] ?)' '#4#'
fail '("o" [#1# #2# #3#] #100#)' '#4#'
fail '("o" [#1# #2# #3#] "word")' '#4#'
fail '("o" [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("o" ("#" "text") ("..."))' '#3#'
fail '("o" #1# ("#" "text"))' '#3#'
