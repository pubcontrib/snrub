#!/bin/sh
. test/assert.sh

hint 'expression'

pass '(">" "number" #1#)
(">" "number" ("+" ("<" "number") ("<" "number")))
("<" "number")' '#2#'
pass '(">" "operator" "+")
(("<" "operator") #10# #5#)' '#15#'
pass '("~" "comments")
"word"' '"word"'
pass '"word"
("~" "comments")' '"word"'
pass '"before"
("~" "comments")
"after"' '"after"'
fail '()' '#4#'
fail '("+" #1# #2# #3#)' '#4#'
fail '"+" #10# #5#)' '#1#'
fail '("+" #10# #5#' '#1#'
fail '(?)' '#4#'
fail '(#1#)' '#4#'
fail '("unknown")' '#4#'
fail '("<"("<"("<"("<"("<"("<"("<"("<"
("<"("<"("<"("<"("<"("<"("<"("<"
("<"("<"("<"("<"("<"("<"("<"("<"
("<"("<"("<"("<"("<"("<"("<"("<"
"k"
))))))))
))))))))
))))))))
))))))))' '#2#'
