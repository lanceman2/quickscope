
libdir := $(shell pkg-config --variable=libdir netcdf)


# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
NETCDF_LDFLAGS := $(shell pkg-config --libs netcdf)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)


NETCDF_CFLAGS := $(shell pkg-config --cflags netcdf)

ifeq ($(libdir),)
# Use of netCDF is not required.
$(warn software package netCDF was not found)
endif

# Spew what netCDF compiler options we have found
#$(info NETCDF_CFLAGS="$(NETCDF_CFLAGS)" NETCDF_LDFLAGS="$(NETCDF_LDFLAGS)")

undefine libdir
