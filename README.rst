====
IVX7
====

IVX7 is a tool for reading, modifying, and writing Yaesu VX-7[R] configurations.

---------
Reference
---------

I used `this website
<http://hse.dyndns.org/hiroto/RFY_LAB/vx7/e/vx7_8500.htm>`_ for all
reverse engineering information.

-----------
Buildsystem
-----------

Installation
++++++++++++

make INSTALL=1 ivx7.release

--------
Examples
--------

Creating a new clone file using a template and pgm scripts::

    ivx7 load template.vx7 \
        script pgmscripts/defaults.pgm \
        script pgmscripts/simplex.pgm \
        script pgmscripts/southbay_rpt.pgm \
        script pgmscripts/carla.pgm \
        script pgmscripts/frs_murs.pgm \
        save new.vx7

Getting info about a clone file::

    ivx7 load new.vx7 info
