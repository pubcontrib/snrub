#!/bin/sh
. test/assert.sh

hint 'operator/index'

pass '("[#]" "abc" #0#)' '?'
pass '("[#]" "abc" #1#)' '"a"'
pass '("[#]" "abc" #2#)' '"b"'
pass '("[#]" "abc" #3#)' '"c"'
pass '("[#]" "abc" #4#)' '?'
pass '("[#]" [?] #1#)' '?'
pass '("[#]" [#10#] #1#)' '#10#'
pass '("[#]" ["10"] #1#)' '"10"'
pass '("[#]" [[#10#]] #1#)' '[#10#]'
pass '("[#]" [#10# #20# #30#] #0#)' '?'
pass '("[#]" [#10# #20# #30#] #1#)' '#10#'
pass '("[#]" [#10# #20# #30#] #2#)' '#20#'
pass '("[#]" [#10# #20# #30#] #3#)' '#30#'
pass '("[#]" [#10# #20# #30#] #4#)' '?'
fail '("[#]")' '#4#'
fail '("[#]" "abc")' '#4#'
fail '("[#]" ? #1#)' '#4#'
fail '("[#]" #123# #1#)' '#4#'
