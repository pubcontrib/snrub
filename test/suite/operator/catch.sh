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
pass '("><" ("#" "text"))' '#3#'
pass '("><" ("#" "text") ("/" #1# #0#))' '#3#'
pass '("><" ("..." ("#" "text") ("-->" "set" "+"))) ("<--" "set")' '?'
pass '("><" ("..." ("-->" "set" "+") ("#" "text"))) ("<--" "set")' '"+"'

# Argument Error
fail '("><")' '#4#'
