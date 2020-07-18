#!/bin/sh
. test/assert.sh

hint 'expression'

pass '("-->" "number" #1#)
("-->" "number" ("+" ("<--" "number") ("<--" "number")))
("<--" "number")' '#2#'
pass '("-->" "operator" "+")
(("<--" "operator") #10# #5#)' '#15#'
pass '`comments`
"word"' '"word"'
pass '"word"
`comments`' '"word"'
pass '"before"
`comments`
"after"' '"after"'
pass '("-->" "path" "none")
("?" #1#
    ("..."
        ("-->" "path" "pass")
        ("<--" "path")
    )
    ("..."
        ("-->" "path" "fail")
        ("<--" "path")
    )
)' '"pass"'
pass '("-->" "path" "none")
("?" #0#
    ?
    ("..."
        ("-->" "path" "fail")
        ("<--" "path")
    )
)' '"fail"'
pass '("-->" "path" "none")
("?" #1#
    ("..."
        ("-->" "path" "pass")
        ("<--" "path")
    )
    ?
)' '"pass"'
fail '()' '#4#'
pass '("+" #1# #2# #3#)' '#3#'
fail '"+" #10# #5#)' '#1#'
fail '("+" #10# #5#' '#1#'
fail '(?)' '#4#'
fail '(#1#)' '#4#'
fail '("unknown")' '#4#'
fail '("<--"("<--"("<--"("<--"("<--"("<--"("<--"("<--"
("<--"("<--"("<--"("<--"("<--"("<--"("<--"("<--"
("<--"("<--"("<--"("<--"("<--"("<--"("<--"("<--"
("<--"("<--"("<--"("<--"("<--"("<--"("<--"("<--"
"k"
))))))))
))))))))
))))))))
))))))))' '#2#'
fail '("~" "." "." "." "." "." "." "."
"." "." "." "." "." "." "." "."
"." "." "." "." "." "." "." "."
"." "." "." "." "." "." "." "."
"!"
)' '#2#'
