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
pass '("><" ("#" "text"))' '#3#'
pass '("><" ("#" "text") ("/" #1# #0#))' '#3#'
pass '("><" ("..." ("#" "text") ("x<-" "set" "+"))) ("x->" "set")' '?'
pass '("><" ("..." ("x<-" "set" "+") ("#" "text"))) ("x->" "set")' '"+"'

# Argument Error
fail '("><")' '#4#'
