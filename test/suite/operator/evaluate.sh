. test/assert.sh

hint 'operator/evaluate'

# Literal Evaluation
pass '("~" "?")' '?'
pass '("~" "#1#")' '#1#'
pass '("~" "\"one\"")' '"one"'
pass '("~" "[? #1# \"one\" []]")' '[? #1# "one" []]'
pass '("~" "(\"#\" \"1\")")' '#1#'

# Variable Persistence
pass '("~" "(\"-->\" \"string\" \"word\")") ("<--" "string")' '"word"'

# Argument Error
fail '("~")' '#4#'
fail '("~" ?)' '#4#'
fail '("~" #100#)' '#4#'
fail '("~" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("~" ("#" "text"))' '#3#'
fail '("~" "(\"#\" \"text\")")' '#3#'
