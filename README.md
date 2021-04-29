# Snrub
> Snrub is an open-source, platform-independent, and interpreted programming
language.

```
`set numbers a and b then put their sum into c`
("-->" "a" #300#)
("-->" "b" #200#)
("-->" "c" ("+" ("<--" "a") ("<--" "b")))
```

## License
Snrub is licensed under the MIT license. Review the LICENSE file included in the
project before using Snrub to understand your rights.

## Building
The codebase is written using the 1989 ANSI standard of C. An ANSI-compliant
compiler is required to build the program from source. Consider the following
minimums of the target data model additional requirements:

- `int` must be able to hold all values between -(2^32) and 2^32
- `size_t` must be able to hold all values between 0 and 2^32

Builds are automated using Make. Try one of the available targets: `all`,
`clean`, `check`, `install`, `uninstall`. Each target tries to conform to the
GNU Makefile best practices.

Fastest way to get the program built and installed onto a system with Make:
```shell
make install
```

## Running
Once you've built and installed the project you'll have the interpreter at your
system's disposal. Try `-h` for help.

Run code from a file:
```shell
snrub -f script.txt
```

Run code without a file:
```shell
snrub -t '("+" #300# #200#)'
```

Start an interactive scripting shell:
```shell
snrub -i
```

## Support
Please forward all bug reports and feature requests via email. See the AUTHORS
file for contact information.
