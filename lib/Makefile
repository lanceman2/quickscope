SUBDIRS := pkgconfig

INSTALL_DIR = $(PREFIX)/lib

root := ..

include $(root)/pkg-gtk+-3.0.make


libquickscope.so_SOURCES :=\
 debug.c\
 app.c\
 window.c\
 constructor.c\
 tabMenu.c\
 graph.c\
 DrawGrids.c\
 actions.c\
 callbacks.c\
 commonGtkCallbacks.c\
 cssStyle.c
libquickscope.so_CPPFLAGS :=\
 $(GTK_CFLAGS)\
 -DQS_LIB_CODE
libquickscope.so_CFLAGS :=\
 -fvisibility=hidden\
 -DQS_EXPORT="__attribute__((visibility(\"default\"))) extern"\
 -DEXPORT="QS_EXPORT"
#
# good read:
#   https://amir.rachum.com/shared-libraries/
#
# The order of these LDFLAGS can matter if we add more library
# dependencies.
#
# At some point we need an operating system package manager that
# fixes this kind of (DSO dependency hell) shit, like NixOS.
#
# Run in a shell:
#
#   readelf -d libquickscope.so
#
libquickscope.so_LDFLAGS :=\
 $(GTK_LDFLAGS)\
 -Wl,--retain-symbols-file=retain-symbols.txt
libquickscope.so: retain-symbols.txt


include $(root)/quickbuild.make

