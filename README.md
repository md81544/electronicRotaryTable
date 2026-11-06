## Electronic Rotary Table

### Building

You'll need CMake, ncurses and GNU readline (`sudo apt install cmake libncurses-dev libreadline-dev`
on a Pi, or `brew install cmake readline` on a Mac).

- `make` builds a version with the hardware mocked out, which runs on any machine
- `make release` builds the real thing (Raspberry Pi only, as it needs the pigpio library)

The stepper motor code lives in a separate repository, [stepperControl](https://github.com/md81544/stepperControl),
which CMake downloads automatically when you first build (so the first build needs network access). The
version used is pinned by `GIT_TAG` in `CMakeLists.txt`. If you're changing stepperControl at the same
time, clone it alongside and build against your local copy instead:

    make STEPPERCONTROL_DIR=../stepperControl

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

[See this code in action](https://youtu.be/4pt8MZhDIB4)

### TO DO
- [X] Several components are shared from the ELS project so should be placed in a separate library (stepperControl)
- [X] UI code (very simple CLI interface)
- [ ] Update UI to enable a step backwards if required
