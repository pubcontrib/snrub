. test/assert.sh

hint 'operator/forget'

# Forget Variable
pass '("x<-" "null" ?) ("x--" "null") ("x->" "null")' '?'
pass '("x<-" "number" #100#) ("x--" "number") ("x->" "number")' '?'
pass '("x<-" "string" "word") ("x--" "string") ("x->" "string")' '?'
pass '("x<-" "list" [#1# #2# #3#]) ("x--" "list") ("x->" "list")' '?'
pass '("x<-" "map" {"key" "value"}) ("x--" "map") ("x->" "map")' '?'
pass '("x<-" "found" #1#) ("x--" "found")' '?'
pass '("x--" "missing")' '?'
pass '("x--" "missing") ("x->" "missing")' '?'

# Argument Error
fail '("x--")' '#4#'
fail '("x--" ?)' '#4#'
fail '("x--" ##)' '#4#'
fail '("x--" [])' '#4#'
fail '("x--" {})' '#4#'

# Error Propogation
fail '("x<-" ("<>" "ERROR"))' '"ERROR"'
