. test/assert.sh

hint 'operator/not'

# Boolean Not
pass '("!" #0#)' '#1#'
pass '("!" #1#)' '#0#'
pass '("!" #500#)' '#0#'

# Argument Error
fail '("!")' '#4#'
fail '("!" ?)' '#4#'
fail '("!" "")' '#4#'
fail '("!" [])' '#4#'
fail '("!" {})' '#4#'

# Error Propogation
fail '("!" ("#" "text"))' '#3#'
