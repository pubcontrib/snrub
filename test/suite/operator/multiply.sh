#!/bin/sh
. test/assert.sh

hint 'operator/multiply'

pass '("*" #10# #5#)' '#50#'
fail '("*")' '#4#'
fail '("*" #5#)' '#4#'
fail '("*" ? ?)' '#4#'
fail '("*" "word" "word")' '#4#'
