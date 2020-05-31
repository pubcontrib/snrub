#!/bin/sh
. test/assert.sh

hint 'operator/equal'

pass '("=" #100# #100#)' '#1#'
pass '("=" #100# #200#)' '#0#'
pass '("=" #200# #100#)' '#0#'
pass '("=" #-100# #-100#)' '#1#'
pass '("=" #-100# #-200#)' '#0#'
pass '("=" #-200# #-100#)' '#0#'
fail '("=")' '#4#'
fail '("=" #1#)' '#4#'
fail '("=" ? ?)' '#4#'
fail '("=" "word" "word")' '#4#'
