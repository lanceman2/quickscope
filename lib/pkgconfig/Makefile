
INSTALL_DIR = $(PREFIX)/lib/pkgconfig


INSTALLED := quickscope.pc


IN_VARS := PREFIX QUICKSCOPE_VERSION


QUICKSCOPE_VERSION := $(shell ../../bin/quickscope -V)

quickscope.pc: quickscope.pc.in


include ../../quickbuild.make
