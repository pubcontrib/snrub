. test/assert.sh

hint 'operator/overload'

# Operator Overloading
pass '("^" "SQ" "(\"*\" (\"x->\" \"@\") (\"x->\" \"@\"))") ("SQ" #10#)' '#100#'
fail '("^" "SQ" "(\"*\" (\"x->\" \"@\") (\"x->\" \"@\"))") ("~" "(\"SQ\" #10#)" ?)' '#4#'
fail '("^" "SQ" "(\"*\" (\"x->\" \"@\") (\"x->\" \"@\"))") ("^" "DO" "(\"SQ\" #10#)") ("DO" ?)' '#4#'
pass '("^" "OP" "\"first\"") ("^" "OP" "\"last\"") ("OP" ?)' '"last"'
pass '("^" "!" "(\"+\" (\"x->\" \"@\") \"!!!\")") ("!" "hello")' '"hello!!!"'
pass '("^" "x<-" "(\"x<-\" \"global\" (\"x->\" \"@\")) (\"<3\" \"global\")") ("x<-" "word") ("x->" "global")' '"word"'

# Literal Evaluation
pass '("^" "OP" "?") ("OP" ?)' '?'
pass '("^" "OP" "#100#") ("OP" ?)' '#100#'
pass '("^" "OP" "\"word\"") ("OP" ?)' '"word"'
pass '("^" "OP" "[#1# #2# #3#]") ("OP" ?)' '[#1# #2# #3#]'
pass '("^" "OP" "{\"key\" \"value\"}") ("OP" ?)' '{"key" "value"}'
pass '("^" "OP" "(\"#\" \"1\")") ("OP" ?)' '#1#'

# Variable Persistence
pass '("^" "OP" "(\"x<-\" \"local\" \"word\")") ("OP" ?) ("x->" "local")' '?'
pass '("^" "OP" "(\"x<-\" \"local\" \"after\")") ("x<-" "local" "before") ("OP" ?) ("x->" "local")' '"before"'

# Argument Passthrough
pass '("^" "OP" "") ("OP" ?) ("x->" "@")' '?'
pass '("^" "OP" "") ("OP" #100#) ("x->" "@")' '?'
pass '("^" "OP" "") ("OP" "word") ("x->" "@")' '?'
pass '("^" "OP" "") ("OP" [#1# #2# #3#]) ("x->" "@")' '?'
pass '("^" "OP" "") ("OP" {"key" "value"}) ("x->" "@")' '?'
pass '("^" "OP" "(\"x->\" \"@\")") ("OP" ?)' '?'
pass '("^" "OP" "(\"x->\" \"@\")") ("OP" #100#)' '#100#'
pass '("^" "OP" "(\"x->\" \"@\")") ("OP" "word")' '"word"'
pass '("^" "OP" "(\"x->\" \"@\")") ("OP" [#1# #2# #3#])' '[#1# #2# #3#]'
pass '("^" "OP" "(\"x->\" \"@\")") ("OP" {"key" "value"})' '{"key" "value"}'
pass '("^" "OP" "") ("x<-" "@" "before") ("OP" "after") ("x->" "@")' '"before"'
pass '("^" "OP" "(\"x<-\" \"@\" \"lastly\")") ("x<-" "@" "before") ("OP" "after") ("x->" "@")' '"before"'
pass '("^" "OP" "") ("x<-" "@" "before") ("<3" "@") ("OP" "after") ("x->" "@")' '"after"'
pass '("^" "OP" "") ("x<-" "@" "before") ("<3" "@") ("OP" "after") ("OP" "lastly") ("x->" "@")' '"lastly"'
pass '("^" "OP" "(\"x<-\" \"@\" \"lastly\")") ("x<-" "@" "before") ("<3" "@") ("OP" "after") ("x->" "@")' '"lastly"'
pass '("^" "OP" "(\"<3\" \"@\")") ("x<-" "@" "before") ("OP" "after") ("x->" "@")' '"after"'

# Argument Error
fail '("^")' '#4#'
fail '("^" ?)' '#4#'
fail '("^" ? ?)' '#4#'
fail '("^" ? ##)' '#4#'
fail '("^" ? "")' '#4#'
fail '("^" ? [])' '#4#'
fail '("^" ? {})' '#4#'
fail '("^" ##)' '#4#'
fail '("^" ## ?)' '#4#'
fail '("^" ## ##)' '#4#'
fail '("^" ## "")' '#4#'
fail '("^" ## [])' '#4#'
fail '("^" ## {})' '#4#'
fail '("^" "")' '#4#'
fail '("^" [])' '#4#'
fail '("^" [] ?)' '#4#'
fail '("^" [] ##)' '#4#'
fail '("^" [] "")' '#4#'
fail '("^" [] [])' '#4#'
fail '("^" [] {})' '#4#'
fail '("^" {})' '#4#'
fail '("^" {} ?)' '#4#'
fail '("^" {} ##)' '#4#'
fail '("^" {} "")' '#4#'
fail '("^" {} [])' '#4#'
fail '("^" {} {})' '#4#'

# Error Propogation
fail '("^" ("#" "text") "(\"#\" \"100\")")' '#3#'
fail '("^" "OP" ("#" "text"))' '#3#'