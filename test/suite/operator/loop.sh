. test/assert.sh

hint 'operator/loop'

# Conditional Looping
pass '("x<-" "count" #0#)
("o"
    ("<" ("x->" "count") #10#)
    ("x<-" "count" ("+" ("x->" "count") #1#))
)' '?'
pass '("x<-" "count" #0#)
("o"
    ("<" ("x->" "count") #10#)
    ("x<-" "count" ("+" ("x->" "count") #1#))
)
("x->" "count")' '#10#'
pass '("o" #0#)' '?'
pass '("o" #0# ("x<-" "touched" "yes")) ("x->" "touched")' '?'
pass '("x<-" "continue" #-1#) ("o" ("x->" "continue") ("x<-" "continue" #0#))' '?'
pass '("x<-" "continue" #2#) ("o" ("x->" "continue") ("x<-" "continue" #0#))' '?'
pass '("o" ("=" ("x->" "continue") ?) ("..." ("x<-" "continue" "no") ?))' '?'
pass '("o" ("=" ("x->" "continue") ?) ("..." ("x<-" "continue" "no") ##))' '?'
pass '("o" ("=" ("x->" "continue") ?) ("..." ("x<-" "continue" "no") ""))' '?'
pass '("o" ("=" ("x->" "continue") ?) ("..." ("x<-" "continue" "no") []))' '?'
pass '("o" ("=" ("x->" "continue") ?) ("..." ("x<-" "continue" "no") {}))' '?'

# Argument Error
fail '("o")' '#4#'
fail '("o" ?)' '#4#'
fail '("o" ? ?)' '#4#'
fail '("o" ? ##)' '#4#'
fail '("o" ? "")' '#4#'
fail '("o" ? [])' '#4#'
fail '("o" ? {})' '#4#'
fail '("o" #1#)' '#4#'
fail '("o" "")' '#4#'
fail '("o" "" ?)' '#4#'
fail '("o" "" ##)' '#4#'
fail '("o" "" "")' '#4#'
fail '("o" "" [])' '#4#'
fail '("o" "" {})' '#4#'
fail '("o" [])' '#4#'
fail '("o" [] ?)' '#4#'
fail '("o" [] ##)' '#4#'
fail '("o" [] "")' '#4#'
fail '("o" [] [])' '#4#'
fail '("o" [] {})' '#4#'
fail '("o" {})' '#4#'
fail '("o" {} ?)' '#4#'
fail '("o" {} ##)' '#4#'
fail '("o" {} "")' '#4#'
fail '("o" {} [])' '#4#'
fail '("o" {} {})' '#4#'

# Error Propogation
fail '("o" ("#" "text") ("..."))' '#3#'
fail '("o" #1# ("#" "text"))' '#3#'
