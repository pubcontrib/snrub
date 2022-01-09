. test/assert.sh

hint 'operator/demote'

# Variable Scope Demotion
pass '("</3" "local")' '?'
pass '("x<-" "local" "word")
("<3" "local")
("</3" "local")
("x->" "local")' '"word"'
pass '("x<-" "local" "word")
("<3" "local")
("~" "(\"</3\" \"local\") (\"x->\" \"local\")" ?)' '"word"'
pass '("x<-" "local" "word")
("<3" "local")
("~" "(\"</3\" \"local\")" ?)
("x->" "local")' '?'

# Argument Error
fail '("</3")' '#4#'
fail '("</3" ?)' '#4#'
fail '("</3" ##)' '#4#'
fail '("</3" [])' '#4#'
fail '("</3" {})' '#4#'

# Error Propogation
fail '("</3" ("<>" "ERROR") #0#)' '"ERROR"'
