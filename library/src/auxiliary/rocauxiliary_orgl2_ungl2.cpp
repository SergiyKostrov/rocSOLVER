/* ************************************************************************
 * Copyright (c) 2019-2022 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#include "rocauxiliary_orgl2_ungl2.hpp"

template <typename T>
rocblas_status rocsolver_orgl2_ungl2_impl(rocblas_handle handle,
                                          const rocblas_int m,
                                          const rocblas_int n,
                                          const rocblas_int k,
                                          T* A,
                                          const rocblas_int lda,
                                          T* ipiv)
{
    const char* name = (!rocblas_is_complex<T> ? "orgl2" : "ungl2");
    ROCSOLVER_ENTER_TOP(name, "-m", m, "-n", n, "-k", k, "--lda", lda);

    if(!handle)
        return rocblas_status_invalid_handle;

    // argument checking
    rocblas_status st = rocsolver_orgl2_orglq_argCheck(handle, m, n, k, lda, A, ipiv);
    if(st != rocblas_status_continue)
        return st;

    // working with unshifted arrays
    rocblas_int shiftA = 0;

    // normal (non-batched non-strided) execution
    rocblas_stride strideA = 0;
    rocblas_stride strideP = 0;
    rocblas_int batch_count = 1;

    // memory workspace sizes:
    // size for constants in rocblas calls
    size_t size_scalars;
    // size of arrays of pointers (for batched cases)
    size_t size_workArr;
    // extra requirements for calling LARF
    size_t size_Abyx;
    rocsolver_orgl2_ungl2_getMemorySize<false, T>(m, n, batch_count, &size_scalars, &size_Abyx,
                                                  &size_workArr);

    if(rocblas_is_device_memory_size_query(handle))
        return rocblas_set_optimal_device_memory_size(handle, size_scalars, size_Abyx, size_workArr);

    // memory workspace allocation
    void *scalars, *Abyx, *workArr;
    rocblas_device_malloc mem(handle, size_scalars, size_Abyx, size_workArr);
    if(!mem)
        return rocblas_status_memory_error;

    scalars = mem[0];
    Abyx = mem[1];
    workArr = mem[2];
    if(size_scalars > 0)
        init_scalars(handle, (T*)scalars);

    // execution
    return rocsolver_orgl2_ungl2_template<T>(handle, m, n, k, A, shiftA, lda, strideA, ipiv, strideP,
                                             batch_count, (T*)scalars, (T*)Abyx, (T**)workArr);
}

/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */

extern "C" {

rocblas_status rocsolver_sorgl2(rocblas_handle handle,
                                const rocblas_int m,
                                const rocblas_int n,
                                const rocblas_int k,
                                float* A,
                                const rocblas_int lda,
                                float* ipiv)
{
    return rocsolver_orgl2_ungl2_impl<float>(handle, m, n, k, A, lda, ipiv);
}

rocblas_status rocsolver_dorgl2(rocblas_handle handle,
                                const rocblas_int m,
                                const rocblas_int n,
                                const rocblas_int k,
                                double* A,
                                const rocblas_int lda,
                                double* ipiv)
{
    return rocsolver_orgl2_ungl2_impl<double>(handle, m, n, k, A, lda, ipiv);
}

rocblas_status rocsolver_cungl2(rocblas_handle handle,
                                const rocblas_int m,
                                const rocblas_int n,
                                const rocblas_int k,
                                rocblas_float_complex* A,
                                const rocblas_int lda,
                                rocblas_float_complex* ipiv)
{
    return rocsolver_orgl2_ungl2_impl<rocblas_float_complex>(handle, m, n, k, A, lda, ipiv);
}

rocblas_status rocsolver_zungl2(rocblas_handle handle,
                                const rocblas_int m,
                                const rocblas_int n,
                                const rocblas_int k,
                                rocblas_double_complex* A,
                                const rocblas_int lda,
                                rocblas_double_complex* ipiv)
{
    return rocsolver_orgl2_ungl2_impl<rocblas_double_complex>(handle, m, n, k, A, lda, ipiv);
}

} // extern C
