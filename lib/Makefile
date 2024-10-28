SUBDIRS := pkgconfig

INSTALL_DIR = $(PREFIX)/lib

root := ..

include $(root)/pkg-gtk4.make


libquickscope.so_SOURCES :=\
 debug.c
libquickscope.so_CPPFLAGS :=\
 $(GTK_CFLAGS)\
 -DQS_LIB_CODE
libquickscope.so_CFLAGS :=\
 -fvisibility=hidden\
 -DSL_EXPORT="__attribute__((visibility(\"default\"))) extern"\
 -DEXPORT="SL_EXPORT"

# good read:
#   https://amir.rachum.com/shared-libraries/
#
# The order of these LDFLAGS can matter.
#
# At some put we need an operating system package manager that
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

