#!/bin/sh
. test/assert.sh

hint 'operator/chain'

pass '("...")' '?'
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
fail '("-->" "count" #0#)
("..."
    #not a number#
    ("-->" "count" ("+" ("<--" "count") #1#))
)
("<--" "count")' '#3#'
fail '("-->" "count" #0#)
("..."
    ("-->" "count" ("+" ("<--" "count") #1#))
    #not a number#
)
("<--" "count")' '#3#'
