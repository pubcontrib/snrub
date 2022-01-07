. test/assert.sh

hint 'comment'

# Comment Expression
pass '`one-line comment`' '?'
pass '`
    multi-line
    comment
`' '?'
pass '`\``' '?'
pass '`\`nested comment\``' '?'

# Comment Precedence
pass '`comment` "after"' '"after"'
pass '"before" `comment`' '"before"'
pass '"before" `comment` "after"' '"after"'
pass '`comment` `comment`' '?'
pass '(`comment` "+" #10# #20#)' '#30#'
pass '("+" `comment` #10# #20#)' '#30#'
pass '("+" #10# `comment` #20#)' '#30#'
pass '("+" #10# #20# `comment`)' '#30#'

# Code Comment
pass '`\`comment\``' '?'
pass '`?`' '?'
pass '`#1#`' '?'
pass '`"word"`' '?'
pass '`[#1# #2# #3#]`' '?'
pass '`{"key" "value"}`' '?'
pass '`("<>" "error")`' '?'

# Syntax Error
fail '`' '#1#'
