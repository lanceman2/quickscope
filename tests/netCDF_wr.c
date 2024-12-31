
#include <stdio.h>
#include <inttypes.h>
#include <math.h>

#include <netcdf.h>

#include "../lib/debug.h"


int main(void) {

    int id = 0;
    int err = nc_create("tmp_test.nc", 0, &id);
    ASSERT(err == NC_NOERR);
    int dim;
    err = nc_def_dim(id, "x", NC_UNLIMITED, &dim);
    ASSERT(err == NC_NOERR);
    int dims[] = { dim };
    int sinVec;
    err = nc_def_var(id, "sine", NC_DOUBLE, 1/*1->vector*/,
		dims, &sinVec); 	
    ASSERT(err == NC_NOERR);

    err = nc_enddef(id);
    ASSERT(err == NC_NOERR);

    size_t indexp[] = { 0 }; 

    for(uint32_t i=0; i<1000;++i) {

        double y = sin(i/(2*M_PI));
        *indexp = i;
        err = nc_put_var1_double(id, sinVec, indexp, &y);
        ASSERT(err == NC_NOERR, "err=%d i=%" PRIu32, err, i);
    }
    err = nc_close(id);
    ASSERT(err == NC_NOERR);

#if 0 // Doing this and running with valgrind shows that nc_create() does
      // not leak memory on the second call to nc_create().  Okay, fucking
      // great, it only leaks if we dlopen() the library; which is what we
      // wanted to do. 8 bytes + 8 bytes + 1,848 bytes.
    err = nc_create("tmp_test2.nc", 0, &id);
    ASSERT(err == NC_NOERR);
    err = nc_close(id);
    ASSERT(err == NC_NOERR);
#endif

    return 0;
}
