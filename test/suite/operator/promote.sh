. test/assert.sh

hint 'operator/promote'

# Variable Scope Promotion
pass '("<3" "global")' '?'
pass '("x<-" "global" "word")
("<3" "global")
("x->" "global")' '"word"'
pass '("x<-" "global" "word")
("<3" "global")
("~" "(\"x->\" \"global\")" ?)' '"word"'
pass '("~" "(\"x<-\" \"global\" \"word\") (\"<3\" \"global\")" ?)
("x->" "global")' '"word"'
pass '("x<-" "global" "before")
("~" "(\"x<-\" \"global\" \"after\") (\"<3\" \"global\")" ?)
("x->" "global")' '"after"'

# Argument Error
fail '("<3")' '#4#'
fail '("<3" ?)' '#4#'
fail '("<3" ##)' '#4#'
fail '("<3" [])' '#4#'
fail '("<3" {})' '#4#'

# Error Propogation
fail '("<3" ("#" "text") #0#)' '#3#'
