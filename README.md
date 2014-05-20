# genkfs

Writes [KFS filesystems](http://www.knightos.org/documentation/kfs.html) into ROM files.

## Usage

    $ genkfs <rom file> <model directory>

genkfs will assume you want to write the filesystem to the maximum size acceptable for
KnightOS, which is 0x04-0xXX, where 0xXX is the last non-protected sector of the device.

The model directory will be written to the new filesystem as the root.

TODO: Support placing the filesystem at arbituary areas in the ROM.

## Installation

For Linux/Mac/etc, install cmake, make, asciidoc, and a C compiler, then:

    $ cmake .
    $ make
    # make install # as root

On Windows, do the same thing under cygwin after installing cmake and your favorite C
compiler. You can also [download binaries](https://github.com/KnightOS/genkfs/releases)
from the releases page. On cygwin, install asciidoc.
