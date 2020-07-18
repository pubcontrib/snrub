#!/bin/sh
. test/assert.sh

hint 'operator/type'

pass '("_" ?)' '?'
pass '("_" #100#)' '"#"'
pass '("_" "text")' '"\""'
fail '("_" ("#" "text"))' '#3#'
fail '("_")' '#4#'
