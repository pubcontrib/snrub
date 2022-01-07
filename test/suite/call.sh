. test/assert.sh

hint 'call'

repeat()
{
    text=$1
    count=$2

    result="$text"
    index=1

    while [ $index -lt $count ]
    do
        result="$result $text"
        index=`expr $index + 1`
    done

    printf '%s' "$result"
}

# Call Expression
pass '("x<-" "number" #1#)
("x<-" "number" ("+" ("x->" "number") ("x->" "number")))
("x->" "number")' '#2#'
pass '("x<-" "operator" "+")
(("x->" "operator") #10# #5#)' '#15#'
pass '("x<-" "path" "none")
("?" #1#
    ("..."
        ("x<-" "path" "pass")
        ("x->" "path")
    )
    ("..."
        ("x<-" "path" "fail")
        ("x->" "path")
    )
)' '"pass"'
pass '("x<-" "path" "none")
("?" #0#
    ?
    ("..."
        ("x<-" "path" "fail")
        ("x->" "path")
    )
)' '"fail"'
pass '("x<-" "path" "none")
("?" #1#
    ("..."
        ("x<-" "path" "pass")
        ("x->" "path")
    )
    ?
)' '"pass"'
pass '("+" #1# #2# #3#)' '#3#'

# Syntax Error
fail '"+" #10# #5#)' '#1#'
fail '("+" #10# #5#' '#1#'

# Bounds Error
start=`repeat '("!"' 32`
end=`repeat ')' 32`
pass "$start #1#$end" '#1#'
start=`repeat '("!"' 33`
end=`repeat ')' 33`
fail "$start #1#$end" '#2#'
center=`repeat '?' 1024`
pass "(\"...\" $center)" '?'
fail '("x<-" "recurse" "(\"~\" (\"x->\" \"recurse\") ?)")
("<3" "recurse")
("~" ("x->" "recurse") ?)' '#2#'
fail '("x<-" "code" "(\"^\" \"OP\" (\"x->\" \"code\")) (\"OP\" ?)")
("<3" "code")
("^" "OP" ("x->" "code"))
("OP" ?)' '#2#'

# Argument Error
fail '()' '#4#'
fail '(?)' '#4#'
fail '(#1#)' '#4#'
fail '("unknown")' '#4#'
fail '("^" "OP" "") ("OP")' '#4#'

# Error Propogation
fail '(("#" "text"))' '#3#'
fail '("^" "OP" "") ("OP" ("#" "text"))' '#3#'
