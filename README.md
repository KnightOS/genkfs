# genkfs

Writes [KFS filesystems](http://www.knightos.org/documentation/kfs.html) into ROM files.

## Usage

    $ genkfs [options...] <rom file> <model directory>

genkfs will assume you want to write the filesystem to the maximum size acceptable for
KnightOS, which is 0x04-0xXX, where 0xXX is the last non-protected sector of the device.
You can use a few options to tweak this:

* `-d <page>` or `--dat <page>` - given the page in hex, this can specify an alternative
  page to begin the DAT on.
* `-f <page>` or `--fat <page>` - given the page in hex, this can specify an alternative
  page to begin the FAT on.

## Installation

On Windows, do the following under cygwin after installing cmake and your favorite C
compiler.

    $ cmake .
    $ make
    # make install # as root
