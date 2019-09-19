## Electronic Rotary Table
This project is designed to allow precise rotation of a
workpiece for machining with, for example, a milling
machine.

Typically a rotary table is used for this operation,
which is hand-cranked with a vernier-style graduated
display to allow accurate positioning. This allows
gears, for example, to be cut.

This project entails putting a stepper motor on the
worm drive which rotates the table, and controlling it
with a Raspberry Pi. Unlike my [Electronic Lead Screw](https://github.com/md81544/electronicLeadScrew/)
project, this doesn't require precise timing or a
powerful stepper - all it's doing is repeatably rotating
the workpiece, which is then machined, then rotated
again, until complete.

### TO DO
- [ ] Several components are shared from the ELS project so should be placed in a separate git submodule
- [ ] UI code (very simple CLI interface)
