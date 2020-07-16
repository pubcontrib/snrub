#!/bin/sh
. test/assert.sh

hint 'operator/hash'

pass '("::" ?)' '#0#'
pass '("::" "")' '#0#'
pass '("::" "100")' '#145#'
pass '("::" "text")' '#453#'
pass '("::" "The wicked soup officiates the competition.")' '#4150#'
pass '("::" "TTvo8DqrYbo90sIP")' '#1419#'
pass '("::" "52f01e93-a547-42a8-9fea-a8e74fc2951f")' '#2433#'
pass '("::" ##)' '#0#'
pass '("::" #100#)' '#100#'
pass '("::" #-100#)' '#-100#'
pass '("::" #662802839#)' '#662802839#'
pass '("=" ("::" ?) ("::" ?))' '#1#'
pass '("=" ("::" ##) ("::" ##))' '#1#'
pass '("=" ("::" "") ("::" ""))' '#1#'
fail '("::")' '#4#'
fail '("::" ("#" "text"))' '#3#'
