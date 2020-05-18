#!/bin/sh
source test/assert.sh

hint 'whitespace'

pass '(">""string""word")("<""string")' '"word"'
pass ' ( ">" "string" "word" ) ( "<" "string" ) ' '"word"'
pass '	(	">"	"string"	"word"	)	(	"<"	"string"	)	' '"word"'
pass '
(
">"
"string"
"word"
)
(
"<"
"string"
)
' '"word"'
