# Snrub

> Snrub is portability-focused, language-independent, scripting programming
> language.

This project is the home of the Snrub programming language including a reference
implementation, a collection of test cases demonstrating intended behavior, and
living documentation.

    `set numbers a and b then put their sum into c`
    ("x<-" "a" #300#)
    ("x<-" "b" #200#)
    ("x<-" "c" ("+" ("x->" "a") ("x->" "b")))

## Language

### Comment

    `one-line comment`

    `
        multi-line
        comment
    `

    `\`nested comment\``

### Whitespace

    `equivalent expressions`
    ("+"#1##2#)
    ("+" #1# #2#)
    (
        "+"
        #1#
        #2#
    )

### Types

- `?`

        `NULL`
        ?

- `##`

        `NUMBER`

        `positive number`
        #5000#

        `negative number`
        #-5000#

        `smallest number`
        #-999999999#

        `largest number`
        #999999999#

- `""`

        `STRING`

        `plain string`
        "text and words"

        `string with escaped symbols`
        "\n New Line \t Tab \r Carriage Return \\ Backslash \" Double Quote"

        `string with non-printable symbols`
        "A bell goes \a007"

- `[]`

        `LIST`
        [? ## "" [] {}]

- `{}`

        `MAP`
        {"null" ? "number" ## "string" "" "list" [] "map" {}}

### Operators

- `x->`

        `RECALL`
        ("x->" "read-me")

- `x<-`

        `MEMORIZE`
        ("x<-" "override-me" "any value")

- `x--`

        `FORGET`
        ("x--" "remove-me")

- `$->`

        `GET`
        ("$->" {"one" #1# "two" #2# "three" #3#} "two")

- `$<-`

        `SET`
        ("$<-" {} "first" "value")

- `$--`

        `UNSET`
        ("$--" {"forget" "me"} "forget")

- `[o]->`

        `READ`
        ("[o]->" "file_to_read.txt")

- `[o]<-`

        `WRITE`
        ("[o]<-" "file_to_override.txt" "text")

- `[o]--`

        `REMOVE`
        ("[o]--" "file_to_remove.txt")

- `+`

        `ADD`
        ("+" #75# #25#)

- `-`

        `SUBTRACT`
        ("-" #500# #400#)

- `*`

        `MULTIPLY`
        ("*" #20# #5#)

- `/`

        `DIVIDE`
        ("/" #400# #4#)

- `%`

        `MODULO`
        ("%" #1000# #900#)

- `&`

        `AND`
        ("&" #1# #1#)

- `|`

        `OR`
        ("|" #0# #1#)

- `!`

        `NOT`
        ("!" #0#)

- `<`

        `LESS`
        ("<" #0# #100#)

- `>`

        `GREATER`
        (">" #100# #0#)

- `=`

        `EQUAL`
        ("=" #100# #100#)

- `?`

        `CONDITIONAL`
        ("?" #1# "true" "false")

- `o`

        `LOOP`
        ("x<-" "i" #1#)
        ("o" ("<" ("x->" "i") #50#) ("x<-" "i" ("+" ("x->" "i") #1#)))
        ("x->" "i")

- `...`

        `CHAIN`
        ("..." "first" "second" "..." "last")

- `><`

        `CATCH`
        ("><" ("/" #100# #0#))

- `<>`

        `THROW`
        ("<>" "[ERROR] invalid format")

- `_`

        `TYPE`
        ("_" "any value")

- `#`

        `NUMBER`
        ("#" "100")

- `"`

        `STRING`
        ("\"" #100#)

- `| |`

        `LENGTH`
        ("| |" "a string, list, or map")

- `()<-`

        `OVERLOAD`
        ("()<-" "WRAP" "[(\"@\")]")
        ("WRAP" "item")

- `x^`

        `RIPOFF`
        ("x<-" "caller" "parent")
        ("~" "(\"x^\" \"caller\") (\"x->\" \"caller\")" ?)

- `()^`

        `MIME`
        ("()<-" "EXCLAIM" "(\"+\" (\"@\") \"!\")")
        ("~" "(\"()^\" \"EXCLAIM\") (\"EXCLAIM\" \"Mine\")" ?)

- `()--`

        `RESUME`
        ("()<-" "SQUARE" "(\"x<-\" \"#\" (\"@\")) (\"*\" (\"x->\" \"#\") (\"x->\" \"#\"))")
        ("()--" "SQUARE")
        ("SQUARE" #8#)

- `~`

        `EVALUATE`
        ("~" "(\"<\" (\"@\") #0#)" #-1#)

- `@`

        `ADVANCE`
        ("~" "[(\"@\") (\"@\") (\"@\")]" "first" "second" "third")

- `x[]`

        `VARIABLES`
        ("x[]")

- `$[]`

        `KEYS`
        ("$[]" {"first" #1# "second" #2# "third" #3#})

- `()[]`

        `OPERATORS`
        ("()[]")

- `<|>`

        `SORT`
        ("<|>" [#3# #2# #5# #1# #4#])

- `[# #]`

        `SLICE`
        ("[# #]" "find me in this text" #6# #7#)

- `::`

        `HASH`
        ("::" [#2# "bucket"])

- `;`

        `REPRESENT`
        (";" [#2# "code"])

## License

Snrub is licensed under the MIT license. Review the LICENSE file included in the
project before using Snrub to understand your rights.

## Building

The reference implementation is written using the 1989 ANSI standard of C. An
ANSI-compliant compiler is required to build the program from source. Consider
the following minimums of the target data model additional requirements:

- `int` must be able to hold all values between `[-2147483647, 2147483647]`
- `size_t` must be able to hold all values between `[0, 2147483647]`

Builds are optionally automated using Make. Only POSIX features are used in
the Makefile. Try one of the available targets: `all`, `clean`, `check`,
`install`, `uninstall`.

Perform a fresh build of the program, check build passes tests, then install it
to the system:

    make clean check install

## Running

Once you've built and installed the program you'll have the interpreter at your
system's disposal. Try the program option `-h` for usage notes.

Run code from a file:

    snrub -f hello.txt

Run code from a file with an argument:

    snrub -f square.txt '#8#'

Run code from text:

    snrub -t '("+" "hello " "world")'

Run code from text with an argument:

    snrub -t '("*" ("x->" "@") ("x->" "@"))' '#8#'

Start an interactive scripting shell:

    snrub -i

## Support

Please forward all bug reports and feature requests via email. See the AUTHORS
file for contact information.
