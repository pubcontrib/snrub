#!/bin/sh
. test/assert.sh

hint 'operator/catch'

pass '("><")' '?'
pass '("><" ?)' '?'
pass '("><" #100#)' '?'
pass '("><" "text")' '?'
pass '("><" ("#" "text"))' '#3#'
pass '("><" ("#" "text") ("/" #1# #0#))' '#3#'
pass '("><" ("..." ("#" "text") ("-->" "set" "+"))) ("<--" "set")' '?'
pass '("><" ("..." ("-->" "set" "+") ("#" "text"))) ("<--" "set")' '"+"'
