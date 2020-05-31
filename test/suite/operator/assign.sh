#!/bin/sh
. test/assert.sh

hint 'operator/assign'

pass '("-->" "null" ?)' '?'
pass '("-->" "number" #100#)' '?'
pass '("-->" "string" "one hundred")' '?'
fail '("-->")' '#4#'
fail '("-->" "key")' '#4#'
fail '("-->" ? "value")' '#4#'
fail '("-->" #1# "value")' '#4#'
