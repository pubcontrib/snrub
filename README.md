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

        `null`
        ?

- `##`

        `number`
        #123456789#

- `""`

        `string`
        "text and words"

- `[]`

        `list`
        [? ## "" [] {}]

- `{}`

        `map`
        {"null" ? "number" ## "string" "" "list" [] "map" {}}

### Operators

- `x->`

        `recall`
        ("x->" "read-me")

- `x<-`

        `memorize`
        ("x<-" "override-me" "any value")

- `x--`

        `forget`
        ("x--" "remove-me")

- `$->`

        `get`
        ("$->" {"one" #1# "two" #2# "three" #3#} "two")

- `$<-`

        `set`
        ("$<-" {} "first" "value")

- `$--`

        `unset`
        ("$--" {"forget" "me"} "forget")

- `[o]->`

        `read`
        ("[o]->" "file_to_read.txt")

- `[o]<-`

        `write`
        ("[o]<-" "file_to_override.txt" "text")

- `[o]--`

        `remove`
        ("[o]--" "file_to_remove.txt")

- `<3`

        `promote`
        ("<3" "make-me-global")

- `</3`

        `demote`
        ("</3" "make-me-local")

- `+`

        `add`
        ("+" #75# #25#)

- `-`

        `subtract`
        ("-" #500# #400#)

- `*`

        `multiply`
        ("*" #20# #5#)

- `/`

        `divide`
        ("/" #400# #4#)

- `%`

        `modulo`
        ("%" #1000# #900#)

- `&`

        `and`
        ("&" #1# #1#)

- `|`

        `or`
        ("|" #0# #1#)

- `!`

        `not`
        ("!" #0#)

- `<`

        `less`
        ("<" #0# #100#)

- `>`

        `greater`
        (">" #100# #0#)

- `=`

        `equal`
        ("=" #100# #100#)

- `?`

        `conditional`
        ("?" #1# "true" "false")

- `o`

        `loop`
        ("x<-" "i" #1#)
        ("o" ("x->" "i") ("x<-" "i" #0#))

- `...`

        `chain`
        ("..." "first" "second" "..." "last")

- `><`

        `catch`
        ("><" ("/" #100# #0#))

- `<>`

        `throw`
        ("<>" "[ERROR] invalid format")

- `_`

        `type`
        ("_" "any value")

- `#`

        `number`
        ("#" "100")

- `"`

        `string`
        ("\"" #100#)

- `| |`

        `length`
        ("| |" "a string, list, or map")

- `^`

        `overload`
        ("^" "WRAP" "[(\"x->\" \"@\")]")
        ("WRAP" "item")

- `~`

        `evaluate`
        ("~" "(\"<\" (\"x->\" \"@\") #0#)" #-1#)

- `x[]`

        `variables`
        ("x[]")

- `$[]`

        `keys`
        ("$[]" {"first" #1# "second" #2# "third" #3#})

- `()[]`

        `operators`
        ("()[]")

- `<|>`

        `sort`
        ("<|>" [#3# #2# #5# #1# #4#])

- `[# #]`

        `slice`
        ("[# #]" "find me in this text" #6# #7#)

- `::`

        `hash`
        ("::" [#2# "bucket"])

- `;`

        `represent`
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

    snrub -f power.txt '#8#'

Run code from text:

    snrub -t '("+" "hello " "world")'

Run code from text with an argument:

    snrub -t '("*" ("x->" "@") ("x->" "@"))' '#8#'

Start an interactive scripting shell:

    snrub -i

## Support

Please forward all bug reports and feature requests via email. See the AUTHORS
file for contact information.
