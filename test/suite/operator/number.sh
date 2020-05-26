#!/bin/sh
. test/assert.sh

hint 'operator/number'

pass '("#" ?)' '?'
pass '("#" ##)' '#0#'
pass '("#" #100#)' '#100#'
pass '("#" #-100#)' '#-100#'
fail '("#" "word")' '#3#'
fail '("#")' '#4#'
