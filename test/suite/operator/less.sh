. test/assert.sh

hint 'operator/less'

# Object Comparision
pass '("<" ? ?)' '#0#'
pass '("<" #100# #100#)' '#0#'
pass '("<" #100# #200#)' '#1#'
pass '("<" #200# #100#)' '#0#'
pass '("<" #-100# #-100#)' '#0#'
pass '("<" #-100# #-200#)' '#0#'
pass '("<" #-200# #-100#)' '#1#'
pass '("<" "word" "word")' '#0#'
pass '("<" "not" "same")' '#1#'
pass '("<" "same" "not")' '#0#'
pass '("<" "word" "WORD")' '#0#'
pass '("<" "WORD" "word")' '#1#'
pass '("<" "word" " word ")' '#0#'
pass '("<" " word " "word")' '#1#'
pass '("<" "100" #100#)' '#0#'
pass '("<" #100# "100")' '#0#'
pass '("<" "?" ?)' '#0#'
pass '("<" ? "?")' '#0#'
pass '("<" #0# ?)' '#0#'
pass '("<" ? #0#)' '#0#'

# Argument Error
fail '("<")' '#4#'
fail '("<" #1#)' '#4#'

# Error Propogation
fail '("<" ("#" "text") #100#)' '#3#'
fail '("<" #100# ("#" "text"))' '#3#'
