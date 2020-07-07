#!/bin/sh
. test/assert.sh

hint 'operator/not'

pass '("!" #0#)' '#1#'
pass '("!" #1#)' '#0#'
pass '("!" #500#)' '#0#'
fail '("!")' '#4#'
fail '("!" ?)' '#4#'
fail '("!" "word")' '#4#'
fail '("!" ("#" "text"))' '#3#'
