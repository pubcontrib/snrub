. test/assert.sh

hint 'operator/evaluate'

# Literal Evaluation
pass '("~" "")' '?'
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
pass '("~" "(\"@\")" ?)' '?'
pass '("~" "(\"@\")" #100#)' '#100#'
pass '("~" "(\"@\")" "word")' '"word"'
pass '("~" "(\"@\")" [#1# #2# #3#])' '[#1# #2# #3#]'
pass '("~" "(\"@\")" {"key" "value"})' '{"key" "value"}'
pass '("~" "" ("<>" "ERROR"))' '?'
fail '("~" "(\"@\")" ("<>" "ERROR"))' '"ERROR"'

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
