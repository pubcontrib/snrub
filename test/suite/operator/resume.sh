. test/assert.sh

hint 'operator/resume'

# Resume Operator
pass '("()--" "fn")' '?'
pass '("()<-" "fn" "#123#") ("()--" "fn")' '?'
fail '("()<-" "fn" "#123#") ("()--" "fn") ("fn" ?)' '#4#'
pass '("()--" "+")' '?'
pass '("()--" "+") ("+" #5# #10#)' '#15#'

# Argument Error
fail '("()--" ?)' '#4#'
fail '("()--" ##)' '#4#'
fail '("()--" [])' '#4#'
fail '("()--" {})' '#4#'

# Error Propogation
fail '("()--" ("<>" "ERROR"))' '"ERROR"'
