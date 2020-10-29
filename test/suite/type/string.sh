. test/assert.sh

hint 'type/string'

# String Literal
pass '""' '""'
pass '" "' '" "'
pass '"\\"' '"\\"'
pass '"\""' '"\""'
pass '"\t"' '"\t"'
pass '"\n"' '"\n"'
pass '"\r"' '"\r"'
pass '"\z"' '""'
pass '"\\ \" \t \n \r"' '"\\ \" \t \n \r"'
pass '"word"' '"word"'
pass '"word word word"' '"word word word"'
pass '"word\tword\tword"' '"word\tword\tword"'
pass '"word\nword\nword"' '"word\nword\nword"'
pass '"word\rword\rword"' '"word\rword\rword"'
pass '"1"' '"1"'
pass '"`comments`"' '"`comments`"'
pass '"?"' '"?"'
pass '"#1#"' '"#1#"'
pass '"[#1# \"one\"]"' '"[#1# \"one\"]"'
pass '"(\"#\" \"1\")"' '"(\"#\" \"1\")"'

# Syntax Error
fail '"' '#1#'
fail '"word' '#1#'
fail 'word"' '#1#'

# Type Error
fail "$(printf '\042\007\042')" '#3#'
fail "$(printf '\042\011\042')" '#3#'
fail "$(printf '\042\012\042')" '#3#'
fail "$(printf '\042\015\042')" '#3#'
