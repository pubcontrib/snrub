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
pass '"\a000\a001\a002\a003\a004\a005\a006\a007\a008\a011\a012\a014\a015\a016\a017\a018\a019\a020\a021\a022\a023\a024\a025\a026\a027\a028\a029\a030\a031\a127"' '"\a000\a001\a002\a003\a004\a005\a006\a007\a008\a011\a012\a014\a015\a016\a017\a018\a019\a020\a021\a022\a023\a024\a025\a026\a027\a028\a029\a030\a031\a127"'
pass '"\a999"' '""'
pass '"\a"' '""'
pass '"\a65"' '""'
pass '"\a0650"' '"A0"'
pass '"\a-65"' '""'
pass '"\aCAT"' '""'
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

# Non-printable Symbols
index=0

while [ $index -lt 256 ]
do
    null=0
    whitespace=0
    printable=0
    extended=0

    if [ $index -eq 0 ]
    then
        null=1
    fi

    if [ $index -eq 9 -o $index -eq 10 -o $index -eq 13 ]
    then
        whitespace=1
    fi

    if [ $index -ge 32 -a $index -le 126 ]
    then
        printable=1
    fi

    if [ $index -ge 128 ]
    then
        extended=1
    fi

    if [ $extended -eq 0 ]
    then
        if [ $null -eq 0 -a $whitespace -eq 0 -a $printable -eq 0 ]
        then
            octal=`printf '\\%03o' $index`
            byte=`printf '%b' $octal`
            fail "$byte" '#1#'

            text=`printf '"%s"' $byte`
            fail "$text" '#3#'
        fi
    else
        sequence=`printf '"%sa%03d"' '\\' $index`
        pass "$sequence" "$sequence"
    fi

    index=`expr $index + 1`
done
