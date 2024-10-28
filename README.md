# quickscope
2D scope/plotter.

This will be a very flexible 2D plotter with two modes: oscilloscope and
static display modes.  The static display mode will keep a fixed buffer
and be a replacement to the Debain quickplot package by the original
author.

## Status
Just starting.

## Dependences
Current development is on Debian GNU/Linux 12 with a GNONE desktop.
``` bash
apt install libgtk-4-dev libgtk-4-doc
```

## GUI (graphical user interface) Toolkit Programmers Have Fat Heads (RANT)
On a GNU/Linux (and maybe the BSDs) systems the two big desktop GUIs  are
Qt and GTK.  Both, by design, leak lots of system resources.  Both are not
open to being fixed.  Both are bloated.  Both make it hard for advanced
programmers to do things like use the pthreads API (application
programming interface) and control the main loop of the program.  Both
force you to have a shit ton of redundant code in your programs so you can
work around all the bloat.  Both are the center-of-the-universe.  Both are
not robust.  The scope of what these software packages do is just to
large.  They get in the developer's way.

### Main Loop Function Hooks

### Library Destructors

