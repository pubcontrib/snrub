#!/bin/sh
. test/assert.sh

hint 'operator/add'

pass '("+" #10# #5#)' '#15#'
pass '("+" "left" "right")' '"leftright"'
fail '("+")' '#4#'
fail '("+" #5#)' '#4#'
fail '("+" ? ?)' '#4#'
