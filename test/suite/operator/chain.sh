. test/assert.sh

hint 'operator/chain'

# Completed Chain
pass '("..." ?)' '?'
pass '("..." #100#)' '#100#'
pass '("..." "one")' '"one"'
pass '("..." ? ? ?)' '?'
pass '("..." #100# #200# #300#)' '#300#'
pass '("..." "one" "two" "three")' '"three"'
pass '("-->" "count" #0#)
("..."
    ("-->" "count" ("+" ("<--" "count") #1#))
    ("-->" "count" ("+" ("<--" "count") #1#))
    ("-->" "count" ("+" ("<--" "count") #1#))
)
("<--" "count")' '#3#'

# Argument Error
fail '("...")' '#4#'

# Error Breaking
fail '("..." ("#" "text") ("unknown" #1# #2#) ("/" #10# #0#))' '#3#'
fail '("..." ("unknown" #1# #2#) ("/" #10# #0#) ("#" "text"))' '#4#'
fail '("..." ("/" #10# #0#) ("#" "text") ("unknown" #1# #2#))' '#5#'
