. test/assert.sh

hint 'operator/throw'

# Error Throwing
fail '("<>" ?)' '?'
fail '("<>" #100#)' '#100#'
fail '("<>" "text")' '"text"'
fail '("<>" [])' '[]'
fail '("-->" "error" ("><" ("#" "text")))
("<>" ("x->" "error"))' '#3#'
fail '("<>" #1#) ("<>" #2#) ("<>" #3#)' '#1#'
fail '("..." ("<>" #1#) ("..." ("<>" #2#) ("..." ("<>" #3#))))' '#1#'
fail '("..." ("..." ("..." ("<>" #3#)) ("<>" #2#)) ("<>" #1#))' '#3#'

# Argument Error
fail '("<>")' '#4#'

# Error Propogation
fail '("<>" ("#" "text"))' '#3#'
