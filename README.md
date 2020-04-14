# Snrub
> Snrub is an open-source, platform-independent, and interpreted programming
language.

## License
Snrub is licensed under the MIT license. Review the LICENSE file included in the
project before using Snrub to understand your rights.

## Building
The codebase is written using the 1989 ANSI standard of C. Builds are automated
using Make. Try one of the available targets: `all`, `clean`, `check`,
`install`, `uninstall`. Each target tries to conform to the GNU Makefile best
practices.

Fastest way to get the program built and installed into a system with Make:
```shell
make install
```

## Usage
Random sample of syntax:
```
~("comments")
>("set-number" #100#)
>("set-string" "one hundred")
+(#1# #2#)
-(#1# #2#)
*(#1# #2#)
/(#1# #2#)
<("get")
```

## Support
Please forward all bug reports and feature requests via email. See the AUTHORS
file for contact information.
