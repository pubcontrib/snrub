. test/assert.sh

hint 'operator/catch'

# Error Catching
pass '("><")' '?'
pass '("><" ?)' '?'
pass '("><" #100#)' '?'
pass '("><" "text")' '?'
pass '("><" [])' '?'
pass '("><" ("#" "text"))' '#3#'
pass '("><" ("#" "text") ("/" #1# #0#))' '#3#'
pass '("><" ("..." ("#" "text") ("-->" "set" "+"))) ("<--" "set")' '?'
pass '("><" ("..." ("-->" "set" "+") ("#" "text"))) ("<--" "set")' '"+"'
