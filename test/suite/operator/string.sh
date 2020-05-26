#!/bin/sh
. test/assert.sh

hint 'operator/string'

pass '("\"" ?)' '?'
pass '("\"" ##)' '"0"'
pass '("\"" #100#)' '"100"'
pass '("\"" #-100#)' '"-100"'
pass '("\"" "word")' '"word"'
fail '("\"")' '#4#'
