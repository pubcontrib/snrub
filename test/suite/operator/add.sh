#!/bin/sh
. test/assert.sh

hint 'operator/add'

pass '("+" #10# #5#)' '#15#'
fail '("+")' '#4#'
fail '("+" #5#)' '#4#'
fail '("+" ? ?)' '#4#'
fail '("+" "word" "word")' '#4#'