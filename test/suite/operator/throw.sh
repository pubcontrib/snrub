. test/assert.sh

hint 'operator/throw'

# Error Throwing
fail '("<>" ?)' '?'
fail '("<>" #100#)' '#100#'
fail '("<>" "word")' '"word"'
fail '("<>" [#1# #2# #3#])' '[#1# #2# #3#]'
fail '("<>" {"key" "value"})' '{"key" "value"}'
fail '("x<-" "error" ("><" ("#" "text")))
("<>" ("x->" "error"))' '#3#'
fail '("<>" #1#) ("<>" #2#) ("<>" #3#)' '#1#'
fail '("..." ("<>" #1#) ("..." ("<>" #2#) ("..." ("<>" #3#))))' '#1#'
fail '("..." ("..." ("..." ("<>" #3#)) ("<>" #2#)) ("<>" #1#))' '#3#'

# Argument Error
fail '("<>")' '#4#'

# Error Propogation
fail '("<>" ("#" "text"))' '#3#'
