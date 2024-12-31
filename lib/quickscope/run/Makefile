# Why is this directory here in $(PREFIX)/lib/quickscope/run ?
#
# We are assuming that only the code in file
# $(PREFIX)/lib/libquickscope.so needs to see this stuff, so we put these
# files in a sub-directory of the directory where libquickscope.so is.
# The binary program bin/quickscope does not give a shit about these
# files, and it uses libquickscope.so to get information from these files
# in lib/quickscope/run/.

topdir := ../../..


INSTALL_DIR = $(PREFIX)/lib/quickscope/run

INSTALLED :=\
 style.css\
 window.ui


include $(topdir)/quickbuild.make
