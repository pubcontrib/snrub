. test/assert.sh

hint 'operator/promote'

# Variable Scope Promotion
pass '("<3" "global")' '?'
pass '("-->" "global" "word")
("<3" "global")
("x->" "global")' '"word"'
pass '("-->" "global" "word")
("<3" "global")
("~" "(\"x->\" \"global\")" ?)' '"word"'
pass '("~" "(\"-->\" \"global\" \"word\") (\"<3\" \"global\")" ?)
("x->" "global")' '"word"'
pass '("-->" "global" "before")
("~" "(\"-->\" \"global\" \"after\") (\"<3\" \"global\")" ?)
("x->" "global")' '"after"'

# Argument Error
fail '("<3")' '#4#'
fail '("<3" ?)' '#4#'
fail '("<3" #100#)' '#4#'
fail '("<3" [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("<3" ("#" "text") #0#)' '#3#'
