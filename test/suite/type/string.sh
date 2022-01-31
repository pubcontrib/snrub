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
pass '"\a032"' '" "'
pass '"\a033"' '"!"'
pass '"\a048"' '"0"'
pass '"\a058"' '":"'
pass '"\a065"' '"A"'
pass '"\a091"' '"["'
pass '"\a097"' '"a"'
pass '"\a123"' '"{"'
pass '"\a126"' '"~"'
pass '"\a067\a065\a084"' '"CAT"'
pass '"\a000"' '""'
pass '"\a999"' '""'
pass '"\a"' '""'
pass '"\a65"' '""'
pass '"\a0650"' '"A0"'
pass '"\a-65"' '""'
pass '"\aCAT"' '""'
pass '"\a001"' '"\a001"'
pass '"\a009"' '"\t"'
pass '"\a010"' '"\n"'
pass '"\a013"' '"\r"'
pass '"\a034"' '"\""'
pass '"\a092"' '"\\"'
pass '"\a127"' '"\a127"'
pass '"\a001\a002\a003"' '"\a001\a002\a003"'
pass '"\a10"' '""'
pass '"\a0010"' '"\a0010"'
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
