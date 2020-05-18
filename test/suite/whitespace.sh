#!/bin/sh
source test/assert.sh

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
