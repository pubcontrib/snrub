#!/bin/sh
. test/assert.sh

hint 'type/list'

pass '[]' '[]'
pass '[ ]' '[]'
pass '[? ? ?]' '[? ? ?]'
pass '[#1# #2# #3#]' '[#1# #2# #3#]'
pass '["one" "two" "three"]' '["one" "two" "three"]'
pass '[[#1#] [#2# #2#] [#3# #3# #3#]]' '[[#1#] [#2# #2#] [#3# #3# #3#]]'
pass '[? #2# "three" [#4# #4# #4# #4#]]' '[? #2# "three" [#4# #4# #4# #4#]]'
fail '[' '#1#'
fail ']' '#1#'
