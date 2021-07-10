. test/assert.sh

hint 'operator/memorize'

# Memorize Variable
pass '("x<-" "null" ?)' '?'
pass '("x<-" "number" #100#)' '?'
pass '("x<-" "string" "one hundred")' '?'
pass '("x<-" "list" [#1# #0# #0#])' '?'
pass '("x<-" "map" {"key" "value"})' '?'
pass '("x<-" "index" #0#)
("x<-" "goal" #1024#)
("o"
    ("<" ("x->" "index") ("x->" "goal"))
    ("..."
        ("x<-" ("+" "variable-" ("\"" ("x->" "index"))) ("x->" "index"))
        ("x<-" "index" ("+" ("x->" "index") #1#))
    )
)
("x->" "variable-1023")' '#1023#'

# Argument Error
fail '("x<-")' '#4#'
fail '("x<-" ?)' '#4#'
fail '("x<-" #100#)' '#4#'
fail '("x<-" "word")' '#4#'
fail '("x<-" [#1# #2# #3#])' '#4#'
fail '("x<-" ? ?)' '#4#'
fail '("x<-" ? #100#)' '#4#'
fail '("x<-" ? "word")' '#4#'
fail '("x<-" ? [#1# #2# #3#])' '#4#'
fail '("x<-" #100# ?)' '#4#'
fail '("x<-" #100# #100#)' '#4#'
fail '("x<-" #100# "word")' '#4#'
fail '("x<-" #100# [#1# #2# #3#])' '#4#'
fail '("x<-" [#1# #2# #3#] ?)' '#4#'
fail '("x<-" [#1# #2# #3#] #100#)' '#4#'
fail '("x<-" [#1# #2# #3#] "word")' '#4#'
fail '("x<-" [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("x<-" ("#" "text") #100#)' '#3#'
fail '("x<-" "key" ("#" "text"))' '#3#'
