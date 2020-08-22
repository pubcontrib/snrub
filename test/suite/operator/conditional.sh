. test/assert.sh

hint 'operator/conditional'

# Conditional Branching
pass '("?" #0# ? ?)' '?'
pass '("?" #1# ? ?)' '?'
pass '("?" #1# ?)' '?'
pass '("?" #0# #100# #200#)' '#200#'
pass '("?" #1# #100# #200#)' '#100#'
pass '("?" #1# #100#)' '#100#'
pass '("?" #500# #100# #200#)' '#100#'
pass '("?" #0# "one" "two")' '"two"'
pass '("?" #1# "one" "two")' '"one"'
pass '("?" #1# "one")' '"one"'
pass '("?" #0# ("-->" "var" "one") ("-->" "var" "two")) ("<--" "var")' '"two"'
pass '("?" #1# ("-->" "var" "one") ("-->" "var" "two")) ("<--" "var")' '"one"'
pass '("?" #1# ("-->" "var" "one")) ("<--" "var")' '"one"'
pass '("-->" "count" #0#)
("?" #0# ("-->" "count" ("+" ("<--" "count") #1#)) ("-->" "count" ("+" ("<--" "count") #1#)))
("<--" "count")' '#1#'
pass '("-->" "count" #0#)
("?" #1# ("-->" "count" ("+" ("<--" "count") #1#)) ("-->" "count" ("+" ("<--" "count") #1#)))
("<--" "count")' '#1#'

# Argument Error
fail '("?")' '#4#'
fail '("?" #1#)' '#4#'
fail '("?" #0# ?)' '#4#'
fail '("?" ? ? ?)' '#4#'
fail '("?" "word" ? ?)' '#4#'
fail '("?" [#1# #2# #3#] ? ?)' '#4#'

# Error Propogation
fail '("?" ("#" "text") ? ?)' '#3#'
fail '("?" #1# ("#" "text") ?)' '#3#'
fail '("?" #0# ? ("#" "text"))' '#3#'
