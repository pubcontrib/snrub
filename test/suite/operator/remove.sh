. test/assert.sh

hint 'operator/remove'

# Argument Error
fail '("[o]--")' '#4#'
fail '("[o]--" ?)' '#4#'
fail '("[o]--" ##)' '#4#'
fail '("[o]--" [])' '#4#'
fail '("[o]--" {})' '#4#'

# Error Propogation
fail '("[o]--" ("<>" "ERROR"))' '"ERROR"'
