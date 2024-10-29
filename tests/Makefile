root := ..

include $(root)/pkg-gtk4.make



QS_LIB := -L../lib -lquickscope -Wl,-rpath=\$$ORIGIN/../lib


malloc_no_free_SOURCES := malloc.c ../lib/debug.c
malloc_no_free_CPPFLAGS := -DNO_FREE

004_malloc_free_SOURCES := malloc.c ../lib/debug.c

006_spew_SOURCES := 006_spew.c
006_spew_LDFLAGS := $(QS_LIB)

009_display_SOURCES := 009_display.c
009_display_LDFLAGS := $(QS_LIB)

011_window_SOURCES := 011_window.c
011_window_LDFLAGS := $(QS_LIB)




test: build
	./run_tests


include ../quickbuild.make

