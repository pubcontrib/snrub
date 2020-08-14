. test/assert.sh

hint 'operator/loop'

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
fail '("o")' '#4#'
fail '("o" #1#)' '#4#'
fail '("o" ("#" "text") ("..."))' '#3#'
fail '("o" #1# ("#" "text"))' '#3#'
