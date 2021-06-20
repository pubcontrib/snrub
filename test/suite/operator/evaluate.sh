. test/assert.sh

hint 'operator/evaluate'

# Literal Evaluation
pass '("~" "?" ?)' '?'
pass '("~" "#100#" ?)' '#100#'
pass '("~" "\"word\"" ?)' '"word"'
pass '("~" "[#1# #2# #3#]" ?)' '[#1# #2# #3#]'
pass '("~" "(\"#\" \"1\")" ?)' '#1#'

# Variable Persistence
pass '("~" "(\"x<-\" \"local\" \"word\")" ?) ("x->" "local")' '?'
pass '("x<-" "local" "before") ("~" "(\"x<-\" \"local\" \"after\")" ?) ("x->" "local")' '"before"'

# Argument Passthrough
pass '("~" "" ?) ("x->" "@")' '?'
pass '("~" "" #100#) ("x->" "@")' '?'
pass '("~" "" "word") ("x->" "@")' '?'
pass '("~" "" [#1# #2# #3#]) ("x->" "@")' '?'
pass '("~" "(\"x->\" \"@\")" ?)' '?'
pass '("~" "(\"x->\" \"@\")" #100#)' '#100#'
pass '("~" "(\"x->\" \"@\")" "word")' '"word"'
pass '("~" "(\"x->\" \"@\")" [#1# #2# #3#])' '[#1# #2# #3#]'
pass '("x<-" "@" "before") ("~" "" "after") ("x->" "@")' '"before"'
pass '("x<-" "@" "before") ("~" "(\"x<-\" \"@\" \"lastly\")" "after") ("x->" "@")' '"before"'
pass '("x<-" "@" "before") ("<3" "@") ("~" "" "after") ("x->" "@")' '"after"'
pass '("x<-" "@" "before") ("<3" "@") ("~" "" "after") ("~" "" "lastly") ("x->" "@")' '"lastly"'
pass '("x<-" "@" "before") ("<3" "@") ("~" "(\"x<-\" \"@\" \"lastly\")" "after") ("x->" "@")' '"lastly"'
pass '("x<-" "@" "before") ("~" "(\"<3\" \"@\")" "after") ("x->" "@")' '"after"'

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
