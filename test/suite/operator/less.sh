. test/assert.sh

hint 'operator/less'

# Value Comparision
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
pass '("<" [? #100# "word" [? #200# "WORD"]] [? #100# "word" [? #200# "WORD"]])' '#0#'
pass '("<" [? #100# "word" [? #200# "WORD"]] [? #200# "word" [? #200# "WORD"]])' '#1#'
pass '("<" [? #100# "word" [? #200# "WORD"]] [? #100# "not" [? #200# "WORD"]])' '#0#'
pass '("<" [? #100# "word" [? #200# "WORD"]] [? #100# "word" [? #-200# "WORD"]])' '#0#'
pass '("<" [? #100# "word" [? #200# "WORD"]] [? #100# "word" [? #200# "NOT"]])' '#0#'
pass '("<" [? #200# "word" [? #200# "WORD"]] [? #100# "word" [? #200# "WORD"]])' '#0#'
pass '("<" [? #100# "not" [? #200# "WORD"]] [? #100# "word" [? #200# "WORD"]])' '#1#'
pass '("<" [? #100# "word" [? #-200# "WORD"]] [? #100# "word" [? #200# "WORD"]])' '#1#'
pass '("<" [? #100# "word" [? #200# "NOT"]] [? #100# "word" [? #200# "WORD"]])' '#1#'
pass '("<" [#100#] [#100# #200#])' '#1#'
pass '("<" [#100# #200#] [#100#])' '#0#'
pass '("<" [] [#100#])' '#1#'
pass '("<" [#100#] [])' '#0#'
pass '("<" {"a" ? "b" ? "c" ?} {"a" ? "b" ? "c" ?})' '#0#'
pass '("<" {"c" ? "b" ? "a" ?} {"a" ? "b" ? "c" ?})' '#0#'
pass '("<" {"a" ? "b" ? "c" ?} {"c" ? "b" ? "a" ?})' '#0#'
pass '("<" {"a" ? "b" ? "c" ?} {"a" ? "b" ?})' '#0#'
pass '("<" {"a" ? "b" ?} {"a" ? "b" ? "c" ?})' '#1#'
pass '("<" {"a" ? "b" ? "c" ? "d" ?} {"a" ? "b" ? "c" ?})' '#0#'
pass '("<" {"a" ? "b" ? "c" ?} {"a" ? "b" ? "c" ? "d" ?})' '#1#'
pass '("<" {"b" ?} {"a" ?})' '#0#'
pass '("<" {"a" ?} {"b" ?})' '#1#'
pass '("<" {"a" ?} {})' '#0#'
pass '("<" {} {"a" ?})' '#1#'
pass '("<" {"a" #0# "b" #0#} {"a" #0# "b" #0#})' '#0#'
pass '("<" {"a" #1# "b" #0#} {"a" #0# "b" #0#})' '#0#'
pass '("<" {"a" #0# "b" #0#} {"a" #1# "b" #0#})' '#1#'
pass '("<" {"a" #1# "b" #0#} {"a" #0# "b" #1#})' '#0#'
pass '("<" {"a" #0# "b" #0#} {"a" #1# "b" #0#})' '#1#'
pass '("<" {"a" #1# "b" #1#} {"a" #1# "b" #1#})' '#0#'
pass '("<" ? ?)' '#0#'
pass '("<" ? "")' '#1#'
pass '("<" ? ##)' '#1#'
pass '("<" ? [])' '#1#'
pass '("<" ? {})' '#1#'
pass '("<" ## ?)' '#0#'
pass '("<" ## ##)' '#0#'
pass '("<" ## "")' '#1#'
pass '("<" ## [])' '#1#'
pass '("<" ## {})' '#1#'
pass '("<" "" ?)' '#0#'
pass '("<" "" ##)' '#0#'
pass '("<" "" "")' '#0#'
pass '("<" "" [])' '#1#'
pass '("<" "" {})' '#1#'
pass '("<" [] ?)' '#0#'
pass '("<" [] ##)' '#0#'
pass '("<" [] "")' '#0#'
pass '("<" [] [])' '#0#'
pass '("<" [] {})' '#1#'
pass '("<" {} ?)' '#0#'
pass '("<" {} ##)' '#0#'
pass '("<" {} "")' '#0#'
pass '("<" {} [])' '#0#'
pass '("<" {} {})' '#0#'

# Argument Error
fail '("<")' '#4#'
fail '("<" #1#)' '#4#'

# Error Propogation
fail '("<" ("#" "text") #100#)' '#3#'
fail '("<" #100# ("#" "text"))' '#3#'
