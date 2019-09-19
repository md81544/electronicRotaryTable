## Electronic Rotary Table
This project is designed to allow precise rotation of a
work piece for working on with, for example, a milling
machine.

Typically a rotary table is used for this operation,
which is hand-cranked with a vernier-style graduated
display to allow accurate positioning. This allows
gears, for example, to be cut.

This project entails putting a stepper motor on the
worm drive which rotates the table, and driving it
with a Raspberry Pi. Unlike my [Electronic Lead Screw](https://github.com/md81544/electronicLeadScrew/)
project, this doesn't require precise timing or a
powerful stepper - all it's doing is repeatably rotating
the workpiece, which is then machined, then rotated
again, until complete.
