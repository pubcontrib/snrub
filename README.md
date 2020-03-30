# Snrub
> Snrub is an open-source, platform-independent, and interpreted programming
language.

## License
Snrub is licensed under the MIT license. Review the LICENSE file included in the
project before using Snrub to understand your rights.

## Building
The codebase is written using the 1989 ANSI standard of C. Builds are automated
using Make.

To build run:
```shell
$ make
```

To clean up everything built run:
```shell
$ make clean
```

To build and install into local system run:
```shell
$ make install
```

To uninstall from local system run:
```shell
$ make uninstall
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
