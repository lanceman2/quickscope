root := ..

include $(root)/pkg-gtk+-3.0.make
include $(root)/pkg-netcdf.make


# We test that the GTK4 include flags come with the quickscope include
# flags from the pkg-config directive: "Requires: gtk4 >= 4.8.3" without
# installing the quickscope package:
QS_CFLAGS := $(shell PKG_CONFIG_PATH=$(root)/lib/pkgconfig pkg-config\
 --cflags quickscope)

QS_LIB := -L../lib -lquickscope -Wl,-rpath=\$$ORIGIN/../lib



malloc_no_free_SOURCES := malloc.c ../lib/debug.c
malloc_no_free_CPPFLAGS := -DNO_FREE

004_malloc_free_SOURCES := malloc.c ../lib/debug.c

006_spew_SOURCES := 006_spew.c
006_spew_LDFLAGS := $(QS_LIB)

011_window_SOURCES := 011_window.c
011_window_CPPFLAGS := $(GTK_CFLAGS)
011_window_LDFLAGS := $(QS_LIB)


gtkDrawingArea_SOURCES := gtkDrawingArea.c ../lib/debug.c
gtkDrawingArea_CPPFLAGS := $(GTK_CFLAGS)
gtkDrawingArea_LDFLAGS := $(GTK_LDFLAGS)


netCDF_wr_SOURCES := netCDF_wr.c ../lib/debug.c
netCDF_wr_CPPFLAGS := $(NETCDF_CFLAGS)
netCDF_wr_LDFLAGS := $(NETCDF_LDFLAGS) -lm

qsApp_run_SOURCES := qsApp_run.c
qsApp_run_LDFLAGS := $(QS_LIB)
qsApp_run_CPPFLAGS := $(QS_CFLAGS)

asciiDataWr_SOURCES := asciiDataWr.c ../lib/debug.c
asciiDataWr_LDFLAGS := -lm



CLEANFILES := $(wildcard tmp_*)


test: build
	./run_tests


include ../quickbuild.make

