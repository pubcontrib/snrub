. test/assert.sh

hint 'operator/catch'

# Error Catching
pass '("><" ?)' '?'
pass '("><" #100#)' '?'
pass '("><" "word")' '?'
pass '("><" [#1# #2# #3#])' '?'
pass '("><" {"key" "value"})' '?'
pass '("><" ("<>" ?))' '?'
pass '("><" ("<>" #100#))' '#100#'
pass '("><" ("<>" "word"))' '"word"'
pass '("><" ("<>" [#1# #2# #3#]))' '[#1# #2# #3#]'
pass '("><" ("<>" {"key" "value"}))' '{"key" "value"}'
pass '("><" ("<>" "ERROR") ("/" #1# #0#))' '"ERROR"'
pass '("><" ("..." ("<>" "ERROR") ("x<-" "set" "+"))) ("x->" "set")' '?'
pass '("><" ("..." ("x<-" "set" "+") ("<>" "ERROR"))) ("x->" "set")' '"+"'

# Argument Error
fail '("><")' '#4#'
