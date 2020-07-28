#!/bin/sh
. test/assert.sh

hint 'type/list'

pass '[]' '[]'
pass '[ ]' '[]'
pass '[? ? ?]' '[? ? ?]'
pass '[#1# #2# #3#]' '[#1# #2# #3#]'
pass '["one" "two" "three"]' '["one" "two" "three"]'
pass '[? #2# "three"]' '[? #2# "three"]'
fail '[' '#1#'
fail ']' '#1#'
