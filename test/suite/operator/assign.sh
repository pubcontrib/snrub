#!/bin/sh
. test/assert.sh

hint 'operator/assign'

pass '("-->" "null" ?)' '?'
pass '("-->" "number" #100#)' '?'
pass '("-->" "string" "one hundred")' '?'
pass '("-->" "list" [ #1# #0# #0# ])' '?'
fail '("-->")' '#4#'
fail '("-->" "key")' '#4#'
fail '("-->" ? "value")' '#4#'
fail '("-->" #1# "value")' '#4#'
fail '("-->" ("#" "text") #100#)' '#3#'
fail '("-->" "key" ("#" "text"))' '#3#'
