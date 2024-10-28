root := ..

include $(root)/pkg-gtk4.make



QS_LIB := -L../lib -lquickscope -Wl,-rpath=\$$ORIGIN/../lib


malloc_no_free_SOURCES := malloc.c ../lib/debug.c
malloc_no_free_CPPFLAGS := -DNO_FREE

004_malloc_free_SOURCES := malloc.c ../lib/debug.c

006_spew_SOURCES := 006_spew.c
006_spew_LDFLAGS := $(QS_LIB)
006_spew_CPPFLAGS := -I../include



test: build
	./run_tests


include ../quickbuild.make

