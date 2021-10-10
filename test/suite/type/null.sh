. test/assert.sh

hint 'type/null'

# Null Literal
pass '?' '?'
pass '' '?'
pass ' ' '?'
pass "`printf '\011'`" '?'
pass "`printf '\012'`" '?'
pass "`printf '\015'`" '?'
pass "`printf '\011\012\015\040'`" '?'
pass '`comments`' '?'
pass '`?`' '?'
pass '`#1#`' '?'
pass '`[#1# "one"]`' '?'
pass '`("#" "1")`' '?'

# Syntax Error
fail "`printf '\007'`" '#1#'
