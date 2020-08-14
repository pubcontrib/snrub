. test/assert.sh

hint 'operator/equal'

pass '("=" ? ?)' '#1#'
pass '("=" #100# #100#)' '#1#'
pass '("=" #100# #200#)' '#0#'
pass '("=" #200# #100#)' '#0#'
pass '("=" #-100# #-100#)' '#1#'
pass '("=" #-100# #-200#)' '#0#'
pass '("=" #-200# #-100#)' '#0#'
pass '("=" "word" "word")' '#1#'
pass '("=" "not" "same")' '#0#'
pass '("=" "same" "not")' '#0#'
pass '("=" "word" "WORD")' '#0#'
pass '("=" "WORD" "word")' '#0#'
pass '("=" "word" " word ")' '#0#'
pass '("=" " word " "word")' '#0#'
pass '("=" [] [])' '#1#'
pass '("=" [? #100# "word" [? #200# "WORD"]] [? #100# "word" [? #200# "WORD"]])' '#1#'
pass '("=" [? #100# "word" [? #200# "WORD"]] [? #200# "word" [? #200# "WORD"]])' '#0#'
pass '("=" [? #100# "word" [? #200# "WORD"]] [? #100# "not" [? #200# "WORD"]])' '#0#'
pass '("=" [? #100# "word" [? #200# "WORD"]] [? #100# "word" [? #-200# "WORD"]])' '#0#'
pass '("=" [? #100# "word" [? #200# "WORD"]] [? #100# "word" [? #200# "NOT"]])' '#0#'
pass '("=" [? #200# "word" [? #200# "WORD"]] [? #100# "word" [? #200# "WORD"]])' '#0#'
pass '("=" [? #100# "not" [? #200# "WORD"]] [? #100# "word" [? #200# "WORD"]])' '#0#'
pass '("=" [? #100# "word" [? #-200# "WORD"]] [? #100# "word" [? #200# "WORD"]])' '#0#'
pass '("=" [? #100# "word" [? #200# "NOT"]] [? #100# "word" [? #200# "WORD"]])' '#0#'
pass '("=" [#100#] [#100# #200#])' '#0#'
pass '("=" [#100# #200#] [#100#])' '#0#'
pass '("=" [] [#100#])' '#0#'
pass '("=" [#100#] [])' '#0#'
pass '("=" "100" #100#)' '#0#'
pass '("=" #100# "100")' '#0#'
pass '("=" "?" ?)' '#0#'
pass '("=" ? "?")' '#0#'
pass '("=" #0# ?)' '#0#'
pass '("=" ? #0#)' '#0#'
fail '("=")' '#4#'
fail '("=" #1#)' '#4#'
fail '("=" ("#" "text") ?)' '#3#'
fail '("=" ? ("#" "text"))' '#3#'
