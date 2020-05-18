#!/bin/sh
source test/assert.sh

hint 'operator/subtract'

pass '("-" #10# #5#)' '#5#'
fail '("-")' '#4#'
fail '("-" #5#)' '#4#'
fail '("-" ? ?)' '#4#'
fail '("-" "word" "word")' '#4#'
