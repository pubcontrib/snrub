. test/assert.sh

hint 'operator/recall'

# Recall Variable
pass '("x->" "key")' '?'
pass '("x<-" "null" ?) ("x->" "null")' '?'
pass '("x<-" "number" #1#) ("x->" "number")' '#1#'
pass '("x<-" "string" "word") ("x->" "string")' '"word"'
pass '("x<-" "list" [#1# #2# #3#]) ("x->" "list")' '[#1# #2# #3#]'
pass '("x<-" "key" "before") ("x<-" "key" "after") ("x->" "key")' '"after"'
pass '("x<-" "key" "before") ("x<-" "key" ?) ("x->" "key")' '?'
pass '("x<-" "key" "before") ("x<-" "key" #1#) ("x->" "key")' '#1#'

# Argument Error
fail '("x->")' '#4#'
fail '("x->" ?)' '#4#'
fail '("x->" #100#)' '#4#'
fail '("x->" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("x->" ("#" "text"))' '#3#'
