. test/assert.sh

hint 'operator/memorize'

# Memorize Variable
pass '("x<-" "null" ?)' '?'
pass '("x<-" "number" #100#)' '?'
pass '("x<-" "string" "word")' '?'
pass '("x<-" "list" [#1# #2# #3#])' '?'
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
fail '("x<-" ? ?)' '#4#'
fail '("x<-" ? ##)' '#4#'
fail '("x<-" ? "")' '#4#'
fail '("x<-" ? [])' '#4#'
fail '("x<-" ? {})' '#4#'
fail '("x<-" ##)' '#4#'
fail '("x<-" ## ?)' '#4#'
fail '("x<-" ## ##)' '#4#'
fail '("x<-" ## "")' '#4#'
fail '("x<-" ## [])' '#4#'
fail '("x<-" ## {})' '#4#'
fail '("x<-" "")' '#4#'
fail '("x<-" [])' '#4#'
fail '("x<-" [] ?)' '#4#'
fail '("x<-" [] ##)' '#4#'
fail '("x<-" [] "")' '#4#'
fail '("x<-" [] [])' '#4#'
fail '("x<-" [] {})' '#4#'
fail '("x<-" {})' '#4#'
fail '("x<-" {} ?)' '#4#'
fail '("x<-" {} ##)' '#4#'
fail '("x<-" {} "")' '#4#'
fail '("x<-" {} [])' '#4#'
fail '("x<-" {} {})' '#4#'

# Error Propogation
fail '("x<-" ("<>" "ERROR") #100#)' '"ERROR"'
fail '("x<-" "key" ("<>" "ERROR"))' '"ERROR"'
