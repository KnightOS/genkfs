genkfs
======

genkfs - Writes KFS filesystems into ROM dumps

Installation
------------

**Linux, Mac**:

1. Install cmake and asciidoc
2. `cmake .`
3. `make`
4. `make install`

**Windows**

Install cygwin with cmake, asciidoc, gcc, and make. Then, follow the Linux
instructions.

Synopsis
--------
'genkfs' _input_ _model_

Description
-----------

_input_ is the ROM file you would like to write the filesystem to. _model_ is a
path to a directory that will be copied into / on the new filesystem.

Examples
--------

`genkfs input.rom ./temp`

Creates a KFS filesystem in input.rom and copies the contents of ./temp to the
root of the new filesystem.

Authors
-------

Maintained by Drew DeVault <sir@cmpwn.com>, who is assisted by other open
source contributors. For more information about genkfs development, see
<https://github.com/KnightOS/genkfs>.
