. test/assert.sh

hint 'type/call'

repeat()
{
    text=$1
    count=$2

    result="$text"
    index=1

    while [ $index -lt $count ]
    do
        result="$result $text"
        index=$((index + 1))
    done

    printf "%s" "$result"
}

pass '("-->" "number" #1#)
("-->" "number" ("+" ("<--" "number") ("<--" "number")))
("<--" "number")' '#2#'
pass '("-->" "operator" "+")
(("<--" "operator") #10# #5#)' '#15#'
pass '`comments` "after"' '"after"'
pass '"before" `comments`' '"before"'
pass '"before" `comments` "after"' '"after"'
pass '`comments` `comments`' '?'
pass '(`comments` "+" #10# #20#)' '#30#'
pass '("+" `comments` #10# #20#)' '#30#'
pass '("+" #10# `comments` #20#)' '#30#'
pass '("+" #10# #20# `comments`)' '#30#'
pass '`("-->" "number" #1#) ("<--" "number")`' '?'
pass '`("#" "text")`' '?'
pass '`\``' '?'
pass '("-->" "path" "none")
("?" #1#
    ("..."
        ("-->" "path" "pass")
        ("<--" "path")
    )
    ("..."
        ("-->" "path" "fail")
        ("<--" "path")
    )
)' '"pass"'
pass '("-->" "path" "none")
("?" #0#
    ?
    ("..."
        ("-->" "path" "fail")
        ("<--" "path")
    )
)' '"fail"'
pass '("-->" "path" "none")
("?" #1#
    ("..."
        ("-->" "path" "pass")
        ("<--" "path")
    )
    ?
)' '"pass"'
pass '("-->""string""word")("<--""string")' '"word"'
pass ' ( "-->" "string" "word" ) ( "<--" "string" ) ' '"word"'
pass '	(	"-->"	"string"	"word"	)	(	"<--"	"string"	)	' '"word"'
pass '
(
"-->"
"string"
"word"
)
(
"<--"
"string"
)
' '"word"'
fail '`' '#1#'
fail '()' '#4#'
pass '("+" #1# #2# #3#)' '#3#'
fail '"+" #10# #5#)' '#1#'
fail '("+" #10# #5#' '#1#'
fail '(?)' '#4#'
fail '(#1#)' '#4#'
fail '("unknown")' '#4#'
fail '(("#" "text"))' '#3#'
start=`repeat '("..."' 32`
end=`repeat ')' 32`
pass "$start$end" '?'
start=`repeat '("..."' 33`
end=`repeat ')' 33`
fail "$start$end" '#2#'
center=`repeat '?' 1023`
pass "(\"...\" $center)" '?'
center=`repeat '?' 1024`
fail "(\"...\" $center)" '#2#'
