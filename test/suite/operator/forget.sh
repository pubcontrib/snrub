. test/assert.sh

hint 'operator/forget'

# Forget Variable
pass '("x<-" "null" ?) ("x--" "null") ("x->" "null")' '?'
pass '("x<-" "number" #100#) ("x--" "number") ("x->" "number")' '?'
pass '("x<-" "string" "one hundred") ("x--" "string") ("x->" "string")' '?'
pass '("x<-" "list" [#1# #0# #0#]) ("x--" "list") ("x->" "list")' '?'
pass '("x<-" "found" #1#) ("x--" "found")' '?'
pass '("x--" "missing")' '?'
pass '("x--" "missing") ("x->" "missing")' '?'
pass '("x--" "@") ("x[]")' '[]'
pass '("x<-" "value" #100#) ("x--" "value") ("x[]")' '["@"]'

# Argument Error
fail '("x--")' '#4#'
fail '("x--" ?)' '#4#'
fail '("x--" #100#)' '#4#'
fail '("x--" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("x<-" ("#" "text"))' '#3#'
