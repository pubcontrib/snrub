. test/assert.sh

hint 'operator/number'

# Number Casting
pass '("#" ?)' '?'
pass '("#" ##)' '#0#'
pass '("#" #0#)' '#0#'
pass '("#" #00#)' '#0#'
pass '("#" #000#)' '#0#'
pass '("#" #1#)' '#1#'
pass '("#" #01#)' '#1#'
pass '("#" #001#)' '#1#'
pass '("#" #-1#)' '#-1#'
pass '("#" #-01#)' '#-1#'
pass '("#" #-001#)' '#-1#'
pass '("#" #-000000000#)' '#0#'
pass '("#" #000000000#)' '#0#'
pass '("#" #-123456789#)' '#-123456789#'
pass '("#" #123456789#)' '#123456789#'
pass '("#" #-999999999#)' '#-999999999#'
pass '("#" #999999999#)' '#999999999#'
pass '("#" "")' '#0#'
pass '("#" "0")' '#0#'
pass '("#" "00")' '#0#'
pass '("#" "000")' '#0#'
pass '("#" "1")' '#1#'
pass '("#" "01")' '#1#'
pass '("#" "001")' '#1#'
pass '("#" "-1")' '#-1#'
pass '("#" "-01")' '#-1#'
pass '("#" "-001")' '#-1#'
pass '("#" "-000000000")' '#0#'
pass '("#" "000000000")' '#0#'
pass '("#" "-123456789")' '#-123456789#'
pass '("#" "123456789")' '#123456789#'
pass '("#" "-999999999")' '#-999999999#'
pass '("#" "999999999")' '#999999999#'

# Type Error
fail '("#" "word")' '#3#'
fail '("#" "1.0#")' '#3#'
fail '("#" "1,2,3")' '#3#'
fail '("#" "1_2_3")' '#3#'
fail '("#" "1 2 3")' '#3#'
fail '("#" "-")' '#3#'
fail '("#" "--10")' '#3#'
fail '("#" "10-")' '#3#'
fail '("#" "1-0")' '#3#'
fail '("#" "-10-")' '#3#'
fail '("#" "0000000000")' '#3#'
fail '("#" "1000000000")' '#3#'

# Argument Error
fail '("#")' '#4#'
fail '("#" [])' '#4#'
fail '("#" {})' '#4#'

# Error Propogation
fail '("#" ("<>" "ERROR"))' '"ERROR"'
