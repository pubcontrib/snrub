. test/assert.sh

hint 'operator/throw'

# Error Throwing
fail '("<>" #0#)' '#0#'
fail '("<>" #1#)' '#1#'
fail '("<>" #2#)' '#2#'
fail '("<>" #3#)' '#3#'
fail '("<>" #4#)' '#4#'
fail '("<>" #5#)' '#5#'
fail '("<>" #6#)' '#6#'
fail '("<>" #-255#)' '#-255#'
fail '("<>" #255#)' '#255#'
fail '("<>" ?)' '?'
fail '("<>" "text")' '"text"'
fail '("<>" [])' '[]'
fail '("-->" "error" ("><" ("#" "text")))
("<>" ("<--" "error"))' '#3#'

# Argument Error
fail '("<>")' '#4#'

# Error Propogation
fail '("<>" ("#" "text"))' '#3#'
