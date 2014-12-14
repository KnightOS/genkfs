# genkfs (1)

genkfs - Writes KFS filesystems into ROM dumps

## Installation

**Linux, Mac**:

1. Install cmake and asciidoc
2. `cmake .`
3. `make`
4. `make install`

**Windows**

Install cygwin with cmake, asciidoc, gcc, and make. Then, follow the Linux
instructions.

## Synopsis

genkfs *input* *model*

## Description

*input* is the ROM file you would like to write the filesystem to. *model* is a
path to a directory that will be copied into / on the new filesystem.

## Examples

`genkfs input.rom ./temp`

Creates a KFS filesystem in input.rom and copies the contents of ./temp to the
root of the new filesystem.

## Authors

Maintained by Drew DeVault <sir@cmpwn.com>, who is assisted by other open
source contributors. For more information about genkfs development, see
<https://github.com/KnightOS/genkfs>.

## Help, Bugs, Feedback

If you need help with KnightOS, want to keep up with progress, chat with
developers, or ask any other questions about KnightOS, you can hang out in the
IRC channel: [#knightos on irc.freenode.net](http://webchat.freenode.net/?channels=knightos).
 
To report bugs, please create [a GitHub issue](https://github.com/KnightOS/KnightOS/issues/new) or contact us on IRC.
 
If you'd like to contribute to the project, please see the [contribution guidelines](http://www.knightos.org/contributing).
