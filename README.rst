========
PRJSTART
========

prjstart is a generic C project framework. The buildsystem supports
both vanilla C and Cocoa projects (OS X). It handles the C, Obj-C,
and C++ languages. It has been tested on Linux and OS X.


-----------
Buildsystem
-----------

The Makefile and files under `buildsystem' should not need to be
modified. Targets are configured with target makefiles. Two examples
are under `targets'; one for a vanilla C project, and one for a Cocoa
project. Targets can support multiple build configs, such as debug
and release.

Running make alone will provide a help and list of detected targets.

::

    $ make
    usage: make <target[.config]>
           make TARGETMK=<target makefile> [CONFIG=<config>]
           make all
           make clean
    other options:
           VERBOSE    setting this to 1 enables verbose output
           INSTALL    setting this to 1 runs the install script for
                      each goal specified
    targets:
     * cocoa
     * vanilla

Target Makefiles
++++++++++++++++

TARGET
    The target makefile's basename is used to create this variable unless
    it is defined in the environment.

SOURCES
    List of all sources to compile for the target.

LIBRARIES
    List of all required libraries to link against. Each library
    becomes a -l<library> flag during the link stage.
    NOTE: If you are using the standard C++ libary (e.g. iostream),
    you must add stdc++ to this list.

OPTIONS
    Global build options for all configurations. Each option becomes
    a -D<option> style flag.

FRAMEWORKS
    List of frameworks to build against. (Obj-C / OS X)

All flags can be appended to in the target makefile as well. For
example, `CPPFLAGS += <flag>'


------------------
Commands Framework
------------------

TO BE WRITTEN


-------
Logging
-------

see include/logging.h
