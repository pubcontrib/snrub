#!/bin/sh
. test/assert.sh

hint 'operator/modulo'

pass '("%" #10# #5#)' '#0#'
pass '("%" #10# #6#)' '#4#'
pass '("%" #-10# #6#)' '#-4#'
pass '("%" #10# #-6#)' '#4#'
pass '("%" #-10# #-6#)' '#-4#'
pass '("%" #0# #1#)' '#0#'
pass '("%" #6# #10#)' '#6#'
fail '("%")' '#4#'
fail '("%" #5#)' '#4#'
fail '("%" #1# #0#)' '#5#'
fail '("%" ? ?)' '#4#'
fail '("%" "word" "word")' '#4#'
fail '("%" ("#" "text") #5#)' '#3#'
fail '("%" #10# ("#" "text"))' '#3#'
