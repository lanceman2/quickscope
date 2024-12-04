
libdir := $(shell pkg-config --variable=libdir gtk4)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
GTK_LDFLAGS := $(shell pkg-config --libs gtk+-3.0)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)

ifeq ($(libdir),)
# Use of GTK4 is required.
$(error software package gtk+-3.0 was not found)
endif

# Spew what GTK4 compiler options we have found
#$(info GTK_CFLAGS="$(GTK_CFLAGS)" GTK_LDFLAGS="$(GTK_LDFLAGS)")

undefine libdir
