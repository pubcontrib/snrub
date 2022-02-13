. test/assert.sh

hint 'type/string'

# String Literal
pass '""' '""'
pass '" "' '" "'
pass '"0123456789"' '"0123456789"'
pass '"ABCDEFGHIJKLMNOPQRSTUVWXYZ"' '"ABCDEFGHIJKLMNOPQRSTUVWXYZ"'
pass '"abcdefghijklmnopqrstuvwxyz"' '"abcdefghijklmnopqrstuvwxyz"'
pass '"!#$%&'"'"'()*+,-./:;<=>?@[]^_`{|}~"' '"!#$%&'"'"'()*+,-./:;<=>?@[]^_`{|}~"'
pass '"\\\"\t\n\r"' '"\\\"\t\n\r"'
pass '"\a032"' '" "'
pass '"\a048\a049\a050\a051\a052\a053\a054\a055\a056\a057"' '"0123456789"'
pass '"\a065\a066\a067\a068\a069\a070\a071\a072\a073\a074\a075\a076\a077\a078\a079\a080\a081\a082\a083\a084\a085\a086\a087\a088\a089\a090"' '"ABCDEFGHIJKLMNOPQRSTUVWXYZ"'
pass '"\a097\a098\a099\a100\a101\a102\a103\a104\a105\a106\a107\a108\a109\a110\a111\a112\a113\a114\a115\a116\a117\a118\a119\a120\a121\a122"' '"abcdefghijklmnopqrstuvwxyz"'
pass '"\a033\a035\a036\a037\a038\a039\a040\a041\a042\a043\a044\a045\a046\a047\a058\a059\a060\a061\a062\a063\a064\a091\a093\a094\a095\a096\a123\a124\a125\a126"' '"!#$%&'"'"'()*+,-./:;<=>?@[]^_`{|}~"'
pass '"\a092\a034\a009\a010\a013"' '"\\\"\t\n\r"'
pass '"\a000"' '"\a000"'
pass '"\a999"' '""'
pass '"\a"' '""'
pass '"\a65"' '""'
pass '"\a0650"' '"A0"'
pass '"\a-65"' '""'
pass '"\aCAT"' '""'
pass '"\a001"' '"\a001"'
pass '"\a127"' '"\a127"'
pass '"\a001\a002\a003"' '"\a001\a002\a003"'
pass '"\a10"' '""'
pass '"\a0010"' '"\a0010"'
pass '"\z"' '""'
pass '"word word word"' '"word word word"'
pass '"word\tword\tword"' '"word\tword\tword"'
pass '"word\nword\nword"' '"word\nword\nword"'
pass '"word\rword\rword"' '"word\rword\rword"'
pass '"word\a000word\a000word"' '"word\a000word\a000word"'

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
