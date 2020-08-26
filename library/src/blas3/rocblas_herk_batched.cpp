/* ************************************************************************
 * Copyright 2016-2020 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "logging.hpp"
#include "rocblas_herk.hpp"
#include "utility.hpp"

namespace
{
    template <typename>
    constexpr char rocblas_herk_name[] = "unknown";
    template <>
    constexpr char rocblas_herk_name<rocblas_float_complex>[] = "rocblas_cherk_batched";
    template <>
    constexpr char rocblas_herk_name<rocblas_double_complex>[] = "rocblas_zherk_batched";

    template <typename T, typename U>
    rocblas_status rocblas_herk_batched_impl(rocblas_handle    handle,
                                             rocblas_fill      uplo,
                                             rocblas_operation transA,
                                             rocblas_int       n,
                                             rocblas_int       k,
                                             const U*          alpha,
                                             const T* const    A[],
                                             rocblas_int       lda,
                                             const U*          beta,
                                             T* const          C[],
                                             rocblas_int       ldc,
                                             rocblas_int       batch_count)
    {
        if(!handle)
            return rocblas_status_invalid_handle;

        RETURN_ZERO_DEVICE_MEMORY_SIZE_IF_QUERIED(handle);

        auto layer_mode = handle->layer_mode;
        if(layer_mode
           & (rocblas_layer_mode_log_trace | rocblas_layer_mode_log_bench
              | rocblas_layer_mode_log_profile))
        {
            auto uplo_letter   = rocblas_fill_letter(uplo);
            auto transA_letter = rocblas_transpose_letter(transA);

            if(handle->pointer_mode == rocblas_pointer_mode_host)
            {
                if(layer_mode & rocblas_layer_mode_log_trace)
                    log_trace(handle,
                              rocblas_herk_name<T>,
                              uplo,
                              transA,
                              n,
                              k,
                              log_trace_scalar_value(alpha),
                              A,
                              lda,
                              log_trace_scalar_value(beta),
                              C,
                              ldc,
                              batch_count);

                if(layer_mode & rocblas_layer_mode_log_bench)
                    log_bench(handle,
                              "./rocblas-bench -f herk_batched -r",
                              rocblas_precision_string<T>,
                              "--uplo",
                              uplo_letter,
                              "--transposeA",
                              transA_letter,
                              "-n",
                              n,
                              "-k",
                              k,
                              LOG_BENCH_SCALAR_VALUE(alpha),
                              "--lda",
                              lda,
                              LOG_BENCH_SCALAR_VALUE(beta),
                              "--ldc",
                              ldc,
                              "--batch_count",
                              batch_count);
            }
            else
            {
                if(layer_mode & rocblas_layer_mode_log_trace)
                    log_trace(handle,
                              rocblas_herk_name<T>,
                              uplo,
                              transA,
                              n,
                              k,
                              log_trace_scalar_value(alpha),
                              A,
                              lda,
                              log_trace_scalar_value(beta),
                              C,
                              ldc,
                              batch_count);
            }

            if(layer_mode & rocblas_layer_mode_log_profile)
                log_profile(handle,
                            rocblas_herk_name<T>,
                            "uplo",
                            uplo_letter,
                            "transA",
                            transA_letter,
                            "N",
                            n,
                            "K",
                            k,
                            "lda",
                            lda,
                            "ldc",
                            ldc,
                            "batch_count",
                            batch_count);
        }

        static constexpr rocblas_int    offset_C = 0, offset_A = 0;
        static constexpr rocblas_stride stride_C = 0, stride_A = 0;

        rocblas_status arg_status = rocblas_herk_arg_check(handle,
                                                           uplo,
                                                           transA,
                                                           n,
                                                           k,
                                                           alpha,
                                                           A,
                                                           offset_A,
                                                           lda,
                                                           stride_A,
                                                           beta,
                                                           C,
                                                           offset_C,
                                                           ldc,
                                                           stride_C,
                                                           batch_count);
        if(arg_status != rocblas_status_continue)
            return arg_status;

        return rocblas_herk_template(handle,
                                     uplo,
                                     transA,
                                     n,
                                     k,
                                     alpha,
                                     A,
                                     offset_A,
                                     lda,
                                     stride_A,
                                     beta,
                                     C,
                                     offset_C,
                                     ldc,
                                     stride_C,
                                     batch_count);
    }

}
/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */

extern "C" {

#ifdef IMPL
#error IMPL ALREADY DEFINED
#endif

#define IMPL(routine_name_, S_, T_)                                                \
    rocblas_status routine_name_(rocblas_handle    handle,                         \
                                 rocblas_fill      uplo,                           \
                                 rocblas_operation transA,                         \
                                 rocblas_int       n,                              \
                                 rocblas_int       k,                              \
                                 const S_*         alpha,                          \
                                 const T_* const   A[],                            \
                                 rocblas_int       lda,                            \
                                 const S_*         beta,                           \
                                 T_* const         C[],                            \
                                 rocblas_int       ldc,                            \
                                 rocblas_int       batch_count)                    \
    try                                                                            \
    {                                                                              \
        return rocblas_herk_batched_impl(                                          \
            handle, uplo, transA, n, k, alpha, A, lda, beta, C, ldc, batch_count); \
    }                                                                              \
    catch(...)                                                                     \
    {                                                                              \
        return exception_to_rocblas_status();                                      \
    }

IMPL(rocblas_cherk_batched, float, rocblas_float_complex);
IMPL(rocblas_zherk_batched, double, rocblas_double_complex);

#undef IMPL

} // extern "C"
