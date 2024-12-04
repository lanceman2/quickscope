
include ../pkg-gtk+-3.0.make

INSTALL_DIR = $(PREFIX)/bin
QS_LIB := -L../lib -lquickscope -Wl,-rpath=\$$ORIGIN/../lib

quickscope_SOURCES := quickscope.c
quickscope_CPPFLAGS := $(GTK_CFLAGS)
quickscope_LDFLAGS := $(QS_LIB)



include ../quickbuild.make
