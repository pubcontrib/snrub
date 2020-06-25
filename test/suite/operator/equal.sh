#!/bin/sh
. test/assert.sh

hint 'operator/equal'

pass '("=" ? ?)' '#1#'
pass '("=" #100# #100#)' '#1#'
pass '("=" #100# #200#)' '#0#'
pass '("=" #200# #100#)' '#0#'
pass '("=" #-100# #-100#)' '#1#'
pass '("=" #-100# #-200#)' '#0#'
pass '("=" #-200# #-100#)' '#0#'
pass '("=" "word" "word")' '#1#'
pass '("=" "not" "same")' '#0#'
pass '("=" "same" "not")' '#0#'
pass '("=" "word" "WORD")' '#0#'
pass '("=" "WORD" "word")' '#0#'
pass '("=" "word" "word ")' '#0#'
pass '("=" "word" " word")' '#0#'
fail '("=")' '#4#'
fail '("=" #1#)' '#4#'
