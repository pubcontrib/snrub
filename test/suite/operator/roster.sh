#!/bin/sh
. test/assert.sh

hint 'operator/roster'

pass '("---")' '[]'
pass '("-->" "a" ?) ("---")' '[]'
pass '("-->" "a" #1#) ("---")' '["a"]'
pass '("-->" "a" #1#) ("-->" "b" #2#) ("---")' '["a" "b"]'
pass '("-->" "b" #1#) ("-->" "a" #2#) ("---")' '["a" "b"]'
pass '("-->" "a" #1#) ("-->" "a" #2#) ("---")' '["a"]'
pass '("-->" "a" #1#) ("-->" "a" ?) ("---")' '[]'
