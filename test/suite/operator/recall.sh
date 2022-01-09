. test/assert.sh

hint 'operator/recall'

# Recall Variable
pass '("x->" "key")' '?'
pass '("x<-" "null" ?) ("x->" "null")' '?'
pass '("x<-" "number" #100#) ("x->" "number")' '#100#'
pass '("x<-" "string" "word") ("x->" "string")' '"word"'
pass '("x<-" "list" [#1# #2# #3#]) ("x->" "list")' '[#1# #2# #3#]'
pass '("x<-" "map" {"key" "value"}) ("x->" "map")' '{"key" "value"}'
pass '("x<-" "key" "before") ("x<-" "key" "after") ("x->" "key")' '"after"'
pass '("x<-" "key" "before") ("x<-" "key" ?) ("x->" "key")' '?'
pass '("x<-" "key" "before") ("x<-" "key" #1#) ("x->" "key")' '#1#'

# Argument Error
fail '("x->")' '#4#'
fail '("x->" ?)' '#4#'
fail '("x->" ##)' '#4#'
fail '("x->" [])' '#4#'
fail '("x->" {})' '#4#'

# Error Propogation
fail '("x->" ("<>" "ERROR"))' '"ERROR"'
