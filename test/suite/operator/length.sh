#!/bin/sh
. test/assert.sh

hint 'operator/length'

pass '("| |" "")' '#0#'
pass '("| |" "1")' '#1#'
pass '("| |" "22")' '#2#'
pass '("| |" "333")' '#3#'
fail '("| |")' '#4#'
fail '("| |" ?)' '#4#'
fail '("| |" #100#)' '#4#'
fail '("| |" ("#" "text"))' '#3#'
fail '("| |" ("#" "text"))' '#3#'
