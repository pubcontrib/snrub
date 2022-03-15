. test/assert.sh

hint 'operator/evaluate'

# Literal Evaluation
pass '("~" "?" ?)' '?'
pass '("~" "#100#" ?)' '#100#'
pass '("~" "\"word\"" ?)' '"word"'
pass '("~" "[#1# #2# #3#]" ?)' '[#1# #2# #3#]'
pass '("~" "{\"key\" \"value\"}" ?)' '{"key" "value"}'
pass '("~" "(\"#\" \"1\")" ?)' '#1#'

# Variable Persistence
pass '("~" "(\"x<-\" \"local\" \"word\")" ?) ("x->" "local")' '?'
pass '("x<-" "local" "before") ("~" "(\"x<-\" \"local\" \"after\")" ?) ("x->" "local")' '"before"'

# Argument Passthrough
pass '("~" "" ?) ("x->" "@")' '?'
pass '("~" "" #100#) ("x->" "@")' '?'
pass '("~" "" "word") ("x->" "@")' '?'
pass '("~" "" [#1# #2# #3#]) ("x->" "@")' '?'
pass '("~" "" {"key" "value"}) ("x->" "@")' '?'
pass '("~" "(\"x->\" \"@\")" ?)' '?'
pass '("~" "(\"x->\" \"@\")" #100#)' '#100#'
pass '("~" "(\"x->\" \"@\")" "word")' '"word"'
pass '("~" "(\"x->\" \"@\")" [#1# #2# #3#])' '[#1# #2# #3#]'
pass '("~" "(\"x->\" \"@\")" {"key" "value"})' '{"key" "value"}'
pass '("x<-" "@" "before") ("~" "" "after") ("x->" "@")' '"before"'
pass '("x<-" "@" "before") ("~" "(\"x<-\" \"@\" \"lastly\")" "after") ("x->" "@")' '"before"'

# Argument Error
fail '("~")' '#4#'
fail '("~" ?)' '#4#'
fail '("~" ? ?)' '#4#'
fail '("~" ? ##)' '#4#'
fail '("~" ? "")' '#4#'
fail '("~" ? [])' '#4#'
fail '("~" ? {})' '#4#'
fail '("~" ##)' '#4#'
fail '("~" ## ?)' '#4#'
fail '("~" ## ##)' '#4#'
fail '("~" ## "")' '#4#'
fail '("~" ## [])' '#4#'
fail '("~" ## {})' '#4#'
fail '("~" "")' '#4#'
fail '("~" [])' '#4#'
fail '("~" [] ?)' '#4#'
fail '("~" [] ##)' '#4#'
fail '("~" [] "")' '#4#'
fail '("~" [] [])' '#4#'
fail '("~" [] {})' '#4#'
fail '("~" {})' '#4#'
fail '("~" {} ?)' '#4#'
fail '("~" {} ##)' '#4#'
fail '("~" {} "")' '#4#'
fail '("~" {} [])' '#4#'
fail '("~" {} {})' '#4#'

# Error Propogation
fail '("~" ("<>" "ERROR") ?)' '"ERROR"'
fail '("~" "(\"<>\" \"ERROR\")" ?)' '"ERROR"'
fail '("~" "" ("<>" "ERROR"))' '"ERROR"'
