#!/bin/sh
. test/assert.sh

hint 'operator/modulo'

pass '("%" #10# #5#)' '#0#'
pass '("%" #10# #4#)' '#2#'
pass '("%" #10# #6#)' '#4#'
fail '("%")' '#4#'
fail '("%" #5#)' '#4#'
fail '("%" #1# #0#)' '#5#'
fail '("%" ? ?)' '#4#'
fail '("%" "word" "word")' '#4#'
