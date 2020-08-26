. test/assert.sh

hint 'operator/greater'

# Object Comparision
pass '(">" ? ?)' '#0#'
pass '(">" #100# #100#)' '#0#'
pass '(">" #100# #200#)' '#0#'
pass '(">" #200# #100#)' '#1#'
pass '(">" #-100# #-100#)' '#0#'
pass '(">" #-100# #-200#)' '#1#'
pass '(">" #-200# #-100#)' '#0#'
pass '(">" "word" "word")' '#0#'
pass '(">" "not" "same")' '#0#'
pass '(">" "same" "not")' '#1#'
pass '(">" "word" "WORD")' '#1#'
pass '(">" "WORD" "word")' '#0#'
pass '(">" "word" " word ")' '#1#'
pass '(">" " word " "word")' '#0#'
pass '(">" "100" #100#)' '#1#'
pass '(">" #100# "100")' '#0#'
pass '(">" "?" ?)' '#1#'
pass '(">" ? "?")' '#0#'
pass '(">" #0# ?)' '#1#'
pass '(">" ? #0#)' '#0#'

# Argument Error
fail '(">")' '#4#'
fail '(">" #1#)' '#4#'

# Error Propogation
fail '(">" ("#" "text") #100#)' '#3#'
fail '(">" #100# ("#" "text"))' '#3#'
