#!/bin/sh
. test/assert.sh

hint 'operator/hash'

pass '("::" "")' '#0#'
pass '("::" "100")' '#145#'
pass '("::" "text")' '#453#'
pass '("::" ##)' '#0#'
pass '("::" #100#)' '#100#'
pass '("::" #-100#)' '#-100#'
