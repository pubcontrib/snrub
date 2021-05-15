. test/assert.sh

hint 'operator/throw'

# Error Throwing
fail '("<>" ?)' '?'
fail '("<>" #100#)' '#100#'
fail '("<>" "text")' '"text"'
fail '("<>" [])' '[]'
fail '("-->" "error" ("><" ("#" "text")))
("<>" ("<--" "error"))' '#3#'

# Argument Error
fail '("<>")' '#4#'

# Error Propogation
fail '("<>" ("#" "text"))' '#3#'
