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

# Code String
pass '"`comment`"' '"`comment`"'
pass '"?"' '"?"'
pass '"#1#"' '"#1#"'
pass '"\"word\""' '"\"word\""'
pass '"[#1# #2# #3#]"' '"[#1# #2# #3#]"'
pass '"{\"key\" \"value\"}"' '"{\"key\" \"value\"}"'
pass '"(\"<>\" \"error\")"' '"(\"<>\" \"error\")"'

# Syntax Error
fail '"' '#1#'
fail '"word' '#1#'
fail 'word"' '#1#'

# Type Error
index=1

while [ $index -lt 256 ]
do
    # 9: hortizontal tab
    # 10: line feed
    # 13: carriage return
    # 32 - 126 printable characters

    printable=0

    if [ $index -ge 32 -a $index -le 126 ]
    then
        printable=1
    fi

    if [ $index -ne 9 -a $index -ne 10 -a $index -ne 13 -a $printable -ne 1 ]
    then
        octal=`printf '\\%03o' $index`
        byte=`printf '%b' $octal`
        fail "$byte" '#1#'

        text=`printf '"%s"' $byte`
        fail "$text" '#3#'
    fi

    index=`expr $index + 1`
done
