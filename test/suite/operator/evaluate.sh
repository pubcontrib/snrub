. test/assert.sh

hint 'operator/evaluate'

# Literal Evaluation
pass '("~" "?" ?)' '?'
pass '("~" "#100#" ?)' '#100#'
pass '("~" "\"word\"" ?)' '"word"'
pass '("~" "[#1# #2# #3#]" ?)' '[#1# #2# #3#]'
pass '("~" "(\"#\" \"1\")" ?)' '#1#'

# Variable Persistence
pass '("~" "(\"-->\" \"local\" \"word\")" ?) ("<--" "local")' '?'
pass '("-->" "local" "before") ("~" "(\"-->\" \"local\" \"after\")" ?) ("<--" "local")' '"before"'

# Argument Passthrough
pass '("~" "(\"<--\" \"@\")" ?)' '?'
pass '("~" "(\"<--\" \"@\")" #100#)' '#100#'
pass '("~" "(\"<--\" \"@\")" "word")' '"word"'
pass '("~" "(\"<--\" \"@\")" [#1# #2# #3#])' '[#1# #2# #3#]'
pass '("-->" "@" "before") ("~" "" "after") ("<--" "@")' '"after"'
pass '("-->" "@" "before") ("~" "(\"-->\" \"@\" \"lastly\")" "after") ("<--" "@")' '"lastly"'
pass '("-->" "@" "before") ("</3" "@") ("~" "" "after") ("<--" "@")' '"after"'

# Argument Error
fail '("~")' '#4#'
fail '("~" ?)' '#4#'
fail '("~" #100#)' '#4#'
fail '("~" "word")' '#4#'
fail '("~" [#1# #2# #3#])' '#4#'
fail '("~" ? ?)' '#4#'
fail '("~" ? #100#)' '#4#'
fail '("~" ? "word")' '#4#'
fail '("~" ? [#1# #2# #3#])' '#4#'
fail '("~" #100# ?)' '#4#'
fail '("~" #100# "word")' '#4#'
fail '("~" #100# #100#)' '#4#'
fail '("~" #100# [#1# #2# #3#])' '#4#'
fail '("~" [#1# #2# #3#] ?)' '#4#'
fail '("~" [#1# #2# #3#] #100#)' '#4#'
fail '("~" [#1# #2# #3#] "word")' '#4#'
fail '("~" [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("~" ("#" "text") ?)' '#3#'
fail '("~" "(\"#\" \"text\")" ?)' '#3#'
fail '("~" "" ("#" "text"))' '#3#'
