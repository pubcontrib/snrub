. test/assert.sh

hint 'whitespace'

# Whitespace Expression
pass '' '?'
pass ' ' '?'
pass "`printf '\011'`" '?'
pass "`printf '\012'`" '?'
pass "`printf '\015'`" '?'
pass "`printf '\011\012\015\040'`" '?'

# Whitespace Independence
pass '("x<-""string""word")("x->""string")' '"word"'
pass ' ( "x<-" "string" "word" ) ( "x->" "string" ) ' '"word"'
pass '	(	"x<-"	"string"	"word"	)	(	"x->"	"string"	)	' '"word"'
pass '
(
"x<-"
"string"
"word"
)
(
"x->"
"string"
)
' '"word"'
