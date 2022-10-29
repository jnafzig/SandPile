import numpy as np
cimport numpy as np
cimport cython


@cython.boundscheck(False)  # Deactivate bounds checking
@cython.wraparound(False)   # Deactivate negative indexing.
@cython.cdivision(True)
def _stabilize(
    np.int8_t[::1] pile,
    np.int8_t[::1] data,
    np.int32_t[::1] indices,
    np.int32_t[::1] indptr,
):
    cdef Py_ssize_t i, j = 0
    cdef np.int8_t spill
    cdef int done = 0
    cdef int iterations = 0

    while not done:
        done = 1
        iterations += 1
        for i in range(indptr.shape[0]-1):
            if pile[i] >= 4:
                spill = pile[i] >> 2
                pile[i] = pile[i] & 3
                for j in range(indptr[i], indptr[i+1]):
                    pile[indices[j]] += spill * data[j]
                done = 0
    return iterations


