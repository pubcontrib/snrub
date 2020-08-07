#!/bin/sh
. test/assert.sh

hint 'operator/roster'

pass '("---")' '[]'
pass '("-->" "null" ?) ("---")' '[]'
pass '("-->" "number" #1#) ("---")' '["number"]'
pass '("-->" "string" "word") ("---")' '["string"]'
pass '("-->" "list" [#1# #2# #3#]) ("---")' '["list"]'
