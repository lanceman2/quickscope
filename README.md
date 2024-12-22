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
apt install libgtk-3-dev libnetcdf-dev
```

## GUI (graphical user interface) Toolkit Programmers (RANT)

This does not have enough detail to be understandable by anyone but
me...

On a GNU/Linux (and maybe the BSDs) systems the two big desktop GUIs are
Qt and GTK.  Both, by design, leak lots of system resources.  Both are not
open to being fixed, by the current developers.  Both are bloated.  Both
make it hard for advanced programmers to do things like use the pthreads
API (application programming interface) and control the main loop of the
program.  Both force you to have a shit ton of redundant code in your
programs so you can work around all the bloat.  Both are the
center-of-the-universe.  Both are not robust.  The scope of what these
software packages do is just to large.

Example of non-robust APIs: Both Qt and GTK have app class objects which
have a main loop thing in them.  In both APIs the app object once created
can never be fully destroyed, that is they always leave system resources
that can never cleaned up by the API.  So unless you effectively change
the linker loader so that it that knows how to clean up for these
libraries you are stuck with a group of libraries that in your program
process.  I had plans to do so innovative things that are now shelved.

GTK3 was chosen as the widget API used by quickscope after spending months
fighting bugs in GTK4.  As of 2024, December 4 GTK4 does not appear to
be ready for "prime time" (at least on Debian 12).

We are fortunate that the kernel can be thought of as being able to clean
up processes; that is the idea of system process is robust.  Though we
keep in our head that programming libraries are self cleaning modules, in
most cases they are not.  The idea of long running processes is not
available to desktop applications which can change modules as they run is
not so possible.  Put another way, inter-process plug and play is not
possible, because most libraries leak system resources when they are
unloaded.  The modules (and dependent libraries) will tend to be
constructed of codes that are made for a particular plug and play
framework.

So my dream of making an interactive on-the-fly computer programming GUI
is not so easy when it assumes that modules are robust (as a module).
Processes cannot be long lived as modules are changed (loaded and
unloaded).  The system resource leaks would build up as modules are
changed.  I need to write my own dynamic linker loader that automatically
cleans up system resources for the libraries.

Currently the dynamic linker loader automatically handles memory mapping
and unmapping sections of the library code.

Is it possible to have the linker loader automatically clean up (for
example) an open file? Not without incurring lots of overhead.  The linker
loader currently has no way to know if the file should be cleaned up or
not, so I don't see how this can be done.  A file that is hidden from the
API and in a library may not be cleaned up.


