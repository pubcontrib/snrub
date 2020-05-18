#!/bin/sh
source test/assert.sh

pass '("~" "comments")' '?'
pass '(">" "key" ("~" "comments")) ("<" "key")' '?'
pass '(">" "key" "word") (">" "key" ("~" "comments")) ("<" "key")' '?'
pass '(">" "key" ("~" "comments")) (">" "key" "word") ("<" "key")' '"word"'
fail '("~")' '#4#'
