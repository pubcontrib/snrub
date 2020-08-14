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
    ("<" ("<--" "index") ("<--" "goal"))
    ("..."
        ("-->" ("+" "variable-" ("\"" ("<--" "index"))) ("<--" "index"))
        ("-->" "index" ("+" ("<--" "index") #1#))
    )
)
("<--" "variable-1023")' '#1023#'

# Argument Mistype
fail '("-->")' '#4#'
fail '("-->" "key")' '#4#'
fail '("-->" ? "value")' '#4#'
fail '("-->" #1# "value")' '#4#'

# Error Propogation
fail '("-->" ("#" "text") #100#)' '#3#'
fail '("-->" "key" ("#" "text"))' '#3#'
