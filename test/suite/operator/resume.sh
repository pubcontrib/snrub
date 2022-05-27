. test/assert.sh

hint 'operator/resume'

# Resume Operator
pass '("()--" "DO")' '?'
pass '("()<-" "DO" "#123#") ("()--" "DO")' '?'
fail '("()<-" "DO" "#123#") ("()--" "DO") ("DO" ?)' '#4#'
pass '("()--" "+")' '?'
pass '("()--" "+") ("+" #5# #10#)' '#15#'

# Argument Error
fail '("()--" ?)' '#4#'
fail '("()--" ##)' '#4#'
fail '("()--" [])' '#4#'
fail '("()--" {})' '#4#'

# Error Propogation
fail '("()--" ("<>" "ERROR"))' '"ERROR"'
