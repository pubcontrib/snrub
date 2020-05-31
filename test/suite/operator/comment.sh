#!/bin/sh
. test/assert.sh

hint 'operator/comment'

pass '("~" "comments")' '?'
pass '("-->" "key" ("~" "comments")) ("<--" "key")' '?'
pass '("-->" "key" "word") ("-->" "key" ("~" "comments")) ("<--" "key")' '?'
pass '("-->" "key" ("~" "comments")) ("-->" "key" "word") ("<--" "key")' '"word"'
fail '("~")' '#4#'
