. test/assert.sh

hint 'operator/catch'

# Error Catching
pass '("><" ?)' '?'
pass '("><" #100#)' '?'
pass '("><" "text")' '?'
pass '("><" [])' '?'
pass '("><" ("<>" ?))' '?'
pass '("><" ("<>" #100#))' '#100#'
pass '("><" ("<>" "text"))' '"text"'
pass '("><" ("<>" []))' '[]'
pass '("><" ("#" "text"))' '#3#'
pass '("><" ("#" "text") ("/" #1# #0#))' '#3#'
pass '("><" ("..." ("#" "text") ("x<-" "set" "+"))) ("x->" "set")' '?'
pass '("><" ("..." ("x<-" "set" "+") ("#" "text"))) ("x->" "set")' '"+"'

# Argument Error
fail '("><")' '#4#'
