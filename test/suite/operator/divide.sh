#!/bin/sh
source test/assert.sh

hint 'operator/divide'

pass '("/" #10# #5#)' '#2#'
fail '("/")' '#4#'
fail '("/" #5#)' '#4#'
fail '("/" #1# #0#)' '#5#'
fail '("/" ? ?)' '#4#'
fail '("/" "word" "word")' '#4#'
