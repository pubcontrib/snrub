. test/assert.sh

hint 'operator/assign'

# Variable Assignment
pass '("-->" "null" ?)' '?'
pass '("-->" "number" #100#)' '?'
pass '("-->" "string" "one hundred")' '?'
pass '("-->" "list" [#1# #0# #0#])' '?'
pass '("-->" "index" #0#)
("-->" "goal" #1024#)
("o"
    ("<" ("x->" "index") ("x->" "goal"))
    ("..."
        ("-->" ("+" "variable-" ("\"" ("x->" "index"))) ("x->" "index"))
        ("-->" "index" ("+" ("x->" "index") #1#))
    )
)
("x->" "variable-1023")' '#1023#'

# Argument Error
fail '("-->")' '#4#'
fail '("-->" ?)' '#4#'
fail '("-->" #100#)' '#4#'
fail '("-->" "word")' '#4#'
fail '("-->" [#1# #2# #3#])' '#4#'
fail '("-->" ? ?)' '#4#'
fail '("-->" ? #100#)' '#4#'
fail '("-->" ? "word")' '#4#'
fail '("-->" ? [#1# #2# #3#])' '#4#'
fail '("-->" #100# ?)' '#4#'
fail '("-->" #100# #100#)' '#4#'
fail '("-->" #100# "word")' '#4#'
fail '("-->" #100# [#1# #2# #3#])' '#4#'
fail '("-->" [#1# #2# #3#] ?)' '#4#'
fail '("-->" [#1# #2# #3#] #100#)' '#4#'
fail '("-->" [#1# #2# #3#] "word")' '#4#'
fail '("-->" [#1# #2# #3#] [#1# #2# #3#])' '#4#'

# Error Propogation
fail '("-->" ("#" "text") #100#)' '#3#'
fail '("-->" "key" ("#" "text"))' '#3#'
