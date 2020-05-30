#!/bin/sh
. test/assert.sh

hint 'operator/conditional'

pass '("?" #0# ? ?)' '?'
pass '("?" #1# ? ?)' '?'
pass '("?" #0# #100# #200#)' '#200#'
pass '("?" #1# #100# #200#)' '#100#'
pass '("?" #500# #100# #200#)' '#100#'
pass '("?" #0# "one" "two")' '"two"'
pass '("?" #1# "one" "two")' '"one"'
fail '("?")' '#4#'
fail '("?" ? ? ?)' '#4#'
fail '("?" "word" ? ?)' '#4#'
