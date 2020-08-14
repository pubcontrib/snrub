. test/assert.sh

hint 'type/number'

# Number Literal
pass '##' '#0#'
pass '#0#' '#0#'
pass '#1#' '#1#'
pass '#10#' '#10#'
pass '#01#' '#1#'
pass '#-1#' '#-1#'

# Type Error
fail '#word#' '#3#'

# Syntax Error
fail '#' '#1#'
fail '#1' '#1#'
fail '1#' '#1#'
