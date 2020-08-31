. test/assert.sh

hint 'operator/throw'

# Error Throwing
fail '("<>")' '#4#'
fail '("<>" #0#)' '#0#'
fail '("<>" #1#)' '#1#'
fail '("<>" #2#)' '#2#'
fail '("<>" #3#)' '#3#'
fail '("<>" #4#)' '#4#'
fail '("<>" #5#)' '#5#'
fail '("<>" #6#)' '#6#'
fail '("<>" #100#)' '#100#'
fail '("<>" "text")' '#4#'
fail '("<>" [])' '#4#'
fail '("<>" ("#" "text"))' '#3#'
