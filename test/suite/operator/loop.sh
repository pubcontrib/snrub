. test/assert.sh

hint 'operator/loop'

# Conditional Looping
pass '("-->" "count" #0#)
("o"
    ("<" ("<--" "count") #10#)
    ("-->" "count" ("+" ("<--" "count") #1#))
)' '?'
pass '("-->" "count" #0#)
("o"
    ("<" ("<--" "count") #10#)
    ("-->" "count" ("+" ("<--" "count") #1#))
)
("<--" "count")' '#10#'

# Argument Error
fail '("o")' '#4#'
fail '("o" #1#)' '#4#'

# Error Propogation
fail '("o" ("#" "text") ("..."))' '#3#'
fail '("o" #1# ("#" "text"))' '#3#'
