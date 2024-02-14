/* ************************************************************************
 * Copyright (C) 2018-2024 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell cop-
 * ies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IM-
 * PLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNE-
 * CTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * ************************************************************************ */

#pragma once

#include "testing_common.hpp"

template <typename T>
void testing_hbmv_strided_batched_bad_arg(const Arguments& arg)
{
    auto rocblas_hbmv_strided_batched_fn = arg.api == FORTRAN
                                               ? rocblas_hbmv_strided_batched<T, true>
                                               : rocblas_hbmv_strided_batched<T, false>;

    auto rocblas_hbmv_strided_batched_fn_64 = arg.api == FORTRAN_64
                                                  ? rocblas_hbmv_strided_batched_64<T, true>
                                                  : rocblas_hbmv_strided_batched_64<T, false>;

    for(auto pointer_mode : {rocblas_pointer_mode_host, rocblas_pointer_mode_device})
    {
        rocblas_local_handle handle{arg};
        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, pointer_mode));

        const rocblas_fill   uplo        = rocblas_fill_upper;
        const int64_t        N           = 100;
        const int64_t        K           = 5;
        const int64_t        lda         = 100;
        const int64_t        incx        = 1;
        const int64_t        incy        = 1;
        const int64_t        batch_count = 2;
        const rocblas_stride stride_A    = 10000;
        const rocblas_stride stride_x    = 100;
        const rocblas_stride stride_y    = 100;

        device_vector<T> alpha_d(1), beta_d(1), one_d(1), zero_d(1);

        const T alpha_h(1), beta_h(2), one_h(1), zero_h(0);

        const T* alpha = &alpha_h;
        const T* beta  = &beta_h;
        const T* one   = &one_h;
        const T* zero  = &zero_h;

        if(pointer_mode == rocblas_pointer_mode_device)
        {
            CHECK_HIP_ERROR(hipMemcpy(alpha_d, alpha, sizeof(*alpha), hipMemcpyHostToDevice));
            alpha = alpha_d;
            CHECK_HIP_ERROR(hipMemcpy(beta_d, beta, sizeof(*beta), hipMemcpyHostToDevice));
            beta = beta_d;
            CHECK_HIP_ERROR(hipMemcpy(one_d, one, sizeof(*one), hipMemcpyHostToDevice));
            one = one_d;
            CHECK_HIP_ERROR(hipMemcpy(zero_d, zero, sizeof(*zero), hipMemcpyHostToDevice));
            zero = zero_d;
        }

        rocblas_int banded_matrix_row = K + 1;

        // Allocate device memory
        device_strided_batch_matrix<T> dAb(banded_matrix_row, N, lda, stride_A, batch_count);
        device_strided_batch_vector<T> dx(N, incx, stride_x, batch_count);
        device_strided_batch_vector<T> dy(N, incy, stride_y, batch_count);

        // Check device memory allocation
        CHECK_DEVICE_ALLOCATION(dAb.memcheck());
        CHECK_DEVICE_ALLOCATION(dy.memcheck());
        CHECK_DEVICE_ALLOCATION(dy.memcheck());

        DAPI_EXPECT(rocblas_status_invalid_handle,
                    rocblas_hbmv_strided_batched_fn,
                    (nullptr,
                     uplo,
                     N,
                     K,
                     alpha,
                     dAb,
                     lda,
                     stride_A,
                     dx,
                     incx,
                     stride_x,
                     beta,
                     dy,
                     incy,
                     stride_y,
                     batch_count));

        DAPI_EXPECT(rocblas_status_invalid_value,
                    rocblas_hbmv_strided_batched_fn,
                    (handle,
                     rocblas_fill_full,
                     N,
                     K,
                     alpha,
                     dAb,
                     lda,
                     stride_A,
                     dx,
                     incx,
                     stride_x,
                     beta,
                     dy,
                     incy,
                     stride_y,
                     batch_count));

        DAPI_EXPECT(rocblas_status_invalid_pointer,
                    rocblas_hbmv_strided_batched_fn,
                    (handle,
                     uplo,
                     N,
                     K,
                     nullptr,
                     dAb,
                     lda,
                     stride_A,
                     dx,
                     incx,
                     stride_x,
                     beta,
                     dy,
                     incy,
                     stride_y,
                     batch_count));

        DAPI_EXPECT(rocblas_status_invalid_pointer,
                    rocblas_hbmv_strided_batched_fn,
                    (handle,
                     uplo,
                     N,
                     K,
                     alpha,
                     dAb,
                     lda,
                     stride_A,
                     dx,
                     incx,
                     stride_x,
                     nullptr,
                     dy,
                     incy,
                     stride_y,
                     batch_count));

        if(pointer_mode == rocblas_pointer_mode_host)
        {
            DAPI_EXPECT(rocblas_status_invalid_pointer,
                        rocblas_hbmv_strided_batched_fn,
                        (handle,
                         uplo,
                         N,
                         K,
                         alpha,
                         nullptr,
                         lda,
                         stride_A,
                         dx,
                         incx,
                         stride_x,
                         beta,
                         dy,
                         incy,
                         stride_y,
                         batch_count));

            DAPI_EXPECT(rocblas_status_invalid_pointer,
                        rocblas_hbmv_strided_batched_fn,
                        (handle,
                         uplo,
                         N,
                         K,
                         alpha,
                         dAb,
                         lda,
                         stride_A,
                         nullptr,
                         incx,
                         stride_x,
                         beta,
                         dy,
                         incy,
                         stride_y,
                         batch_count));

            DAPI_EXPECT(rocblas_status_invalid_pointer,
                        rocblas_hbmv_strided_batched_fn,
                        (handle,
                         uplo,
                         N,
                         K,
                         alpha,
                         dAb,
                         lda,
                         stride_A,
                         dx,
                         incx,
                         stride_x,
                         beta,
                         nullptr,
                         incy,
                         stride_y,
                         batch_count));
        }

        // If N==0, all pointers can be nullptr without error
        DAPI_CHECK(rocblas_hbmv_strided_batched_fn,
                   (handle,
                    uplo,
                    0,
                    K,
                    nullptr,
                    nullptr,
                    lda,
                    stride_A,
                    nullptr,
                    incx,
                    stride_x,
                    nullptr,
                    nullptr,
                    incy,
                    stride_y,
                    batch_count));

        // When alpha==0, A and x can be nullptr without error
        DAPI_CHECK(rocblas_hbmv_strided_batched_fn,
                   (handle,
                    uplo,
                    N,
                    K,
                    zero,
                    nullptr,
                    lda,
                    stride_A,
                    nullptr,
                    incx,
                    stride_x,
                    beta,
                    dy,
                    incy,
                    stride_y,
                    batch_count));

        // When alpha==0 && beta==1, A, x and y can be nullptr without error
        DAPI_CHECK(rocblas_hbmv_strided_batched_fn,
                   (handle,
                    uplo,
                    N,
                    K,
                    zero,
                    nullptr,
                    lda,
                    stride_A,
                    nullptr,
                    incx,
                    stride_x,
                    one,
                    nullptr,
                    incy,
                    stride_y,
                    batch_count));

        // If batch_count==0, all pointers can be nullptr without error
        DAPI_CHECK(rocblas_hbmv_strided_batched_fn,
                   (handle,
                    uplo,
                    N,
                    K,
                    nullptr,
                    nullptr,
                    lda,
                    stride_A,
                    nullptr,
                    incx,
                    stride_x,
                    nullptr,
                    nullptr,
                    incy,
                    stride_y,
                    0));
    }
}

template <typename T>
void testing_hbmv_strided_batched(const Arguments& arg)
{
    auto rocblas_hbmv_strided_batched_fn = arg.api == FORTRAN
                                               ? rocblas_hbmv_strided_batched<T, true>
                                               : rocblas_hbmv_strided_batched<T, false>;

    auto rocblas_hbmv_strided_batched_fn_64 = arg.api == FORTRAN_64
                                                  ? rocblas_hbmv_strided_batched_64<T, true>
                                                  : rocblas_hbmv_strided_batched_64<T, false>;

    int64_t        N                 = arg.N;
    int64_t        K                 = arg.K;
    int64_t        lda               = arg.lda;
    int64_t        incx              = arg.incx;
    int64_t        incy              = arg.incy;
    int64_t        batch_count       = arg.batch_count;
    int64_t        banded_matrix_row = K + 1;
    T              h_alpha           = arg.get_alpha<T>();
    T              h_beta            = arg.get_beta<T>();
    rocblas_fill   uplo              = char2rocblas_fill(arg.uplo);
    rocblas_stride stride_A          = arg.stride_a;
    rocblas_stride stride_x          = arg.stride_x;
    rocblas_stride stride_y          = arg.stride_y;

    rocblas_local_handle handle{arg};

    // argument sanity check before allocating invalid memory
    bool invalid_size = N < 0 || K < 0 || lda <= K || !incx || !incy || batch_count < 0;
    if(invalid_size || !N || !batch_count)
    {
        DAPI_EXPECT(invalid_size ? rocblas_status_invalid_size : rocblas_status_success,
                    rocblas_hbmv_strided_batched_fn,
                    (handle,
                     uplo,
                     N,
                     K,
                     nullptr,
                     nullptr,
                     lda,
                     stride_A,
                     nullptr,
                     incx,
                     stride_x,
                     nullptr,
                     nullptr,
                     incy,
                     stride_y,
                     batch_count));

        return;
    }

    size_t size_A = lda * size_t(N);

    // Naming: `h` is in CPU (host) memory(eg hAB), `d` is in GPU (device) memory (eg dAb).
    // Allocate host memory
    host_strided_batch_matrix<T> hAb(banded_matrix_row, N, lda, stride_A, batch_count);
    host_strided_batch_vector<T> hx(N, incx, stride_x, batch_count);
    host_strided_batch_vector<T> hy(N, incy, stride_y, batch_count);
    host_strided_batch_vector<T> hy_gold(N, incy, stride_y, batch_count);
    host_vector<T>               halpha(1);
    host_vector<T>               hbeta(1);

    // Check host memory allocation
    CHECK_HIP_ERROR(hAb.memcheck());
    CHECK_HIP_ERROR(hx.memcheck());
    CHECK_HIP_ERROR(hy.memcheck());
    CHECK_HIP_ERROR(hy_gold.memcheck());

    // Allocate device memory
    device_strided_batch_matrix<T> dAb(banded_matrix_row, N, lda, stride_A, batch_count);
    device_strided_batch_vector<T> dx(N, incx, stride_x, batch_count);
    device_strided_batch_vector<T> dy(N, incy, stride_y, batch_count);
    device_vector<T>               d_alpha(1);
    device_vector<T>               d_beta(1);

    // Check device memory allocation
    CHECK_DEVICE_ALLOCATION(dAb.memcheck());
    CHECK_DEVICE_ALLOCATION(dx.memcheck());
    CHECK_DEVICE_ALLOCATION(dy.memcheck());
    CHECK_DEVICE_ALLOCATION(d_alpha.memcheck());
    CHECK_DEVICE_ALLOCATION(d_beta.memcheck());

    // Initialize data on host memory
    rocblas_init_matrix(
        hAb, arg, rocblas_client_alpha_sets_nan, rocblas_client_triangular_matrix, true);
    rocblas_init_vector(hx, arg, rocblas_client_alpha_sets_nan, false, true);
    rocblas_init_vector(hy, arg, rocblas_client_beta_sets_nan);
    halpha[0] = h_alpha;
    hbeta[0]  = h_beta;

    // copy vector is easy in STL; hy_gold = hy: save a copy in hy_gold which will be output of
    // CPU BLAS
    hy_gold.copy_from(hy);

    // copy data from CPU to device
    CHECK_HIP_ERROR(dAb.transfer_from(hAb));
    CHECK_HIP_ERROR(dx.transfer_from(hx));
    CHECK_HIP_ERROR(dy.transfer_from(hy));

    double cpu_time_used;
    double rocblas_error_host   = 0.0;
    double rocblas_error_device = 0.0;

    /* =====================================================================
           ROCBLAS
    =================================================================== */
    if(arg.unit_check || arg.norm_check)
    {
        if(arg.pointer_mode_host)
        {
            CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_host));

            handle.pre_test(arg);
            DAPI_CHECK(rocblas_hbmv_strided_batched_fn,
                       (handle,
                        uplo,
                        N,
                        K,
                        &h_alpha,
                        dAb,
                        lda,
                        stride_A,
                        dx,
                        incx,
                        stride_x,
                        &h_beta,
                        dy,
                        incy,
                        stride_y,
                        batch_count));
            handle.post_test(arg);

            // copy output from device to CPU
            CHECK_HIP_ERROR(hy.transfer_from(dy));
        }

        if(arg.pointer_mode_device)
        {
            CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_device));
            CHECK_HIP_ERROR(d_alpha.transfer_from(halpha));
            CHECK_HIP_ERROR(d_beta.transfer_from(hbeta));

            CHECK_HIP_ERROR(dy.transfer_from(hy_gold));

            handle.pre_test(arg);
            DAPI_CHECK(rocblas_hbmv_strided_batched_fn,
                       (handle,
                        uplo,
                        N,
                        K,
                        d_alpha,
                        dAb,
                        lda,
                        stride_A,
                        dx,
                        incx,
                        stride_x,
                        d_beta,
                        dy,
                        incy,
                        stride_y,
                        batch_count));

            handle.post_test(arg);
        }

        // CPU BLAS
        cpu_time_used = get_time_us_no_sync();

        for(size_t b = 0; b < batch_count; b++)
            ref_hbmv<T>(uplo, N, K, h_alpha, hAb[b], lda, hx[b], incx, h_beta, hy_gold[b], incy);

        cpu_time_used = get_time_us_no_sync() - cpu_time_used;

        if(arg.pointer_mode_host)
        {
            if(arg.unit_check)
                unit_check_general<T>(1, N, incy, stride_y, hy_gold, hy, batch_count);
            if(arg.norm_check)
                rocblas_error_host
                    = norm_check_general<T>('F', 1, N, incy, stride_y, hy_gold, hy, batch_count);
        }

        if(arg.pointer_mode_device)
        {
            CHECK_HIP_ERROR(hy.transfer_from(dy));

            if(arg.unit_check)
                unit_check_general<T>(1, N, incy, stride_y, hy_gold, hy, batch_count);
            if(arg.norm_check)
                rocblas_error_device
                    = norm_check_general<T>('F', 1, N, incy, stride_y, hy_gold, hy, batch_count);
        }
    }

    if(arg.timing)
    {
        double gpu_time_used;
        int    number_cold_calls = arg.cold_iters;
        int    total_calls       = number_cold_calls + arg.iters;
        CHECK_ROCBLAS_ERROR(rocblas_set_pointer_mode(handle, rocblas_pointer_mode_host));

        hipStream_t stream;
        CHECK_ROCBLAS_ERROR(rocblas_get_stream(handle, &stream));

        for(int iter = 0; iter < total_calls; iter++)
        {
            if(iter == number_cold_calls)
                gpu_time_used = get_time_us_sync(stream);

            DAPI_DISPATCH(rocblas_hbmv_strided_batched_fn,
                          (handle,
                           uplo,
                           N,
                           K,
                           &h_alpha,
                           dAb,
                           lda,
                           stride_A,
                           dx,
                           incx,
                           stride_x,
                           &h_beta,
                           dy,
                           incy,
                           stride_y,
                           batch_count));
        }

        gpu_time_used = get_time_us_sync(stream) - gpu_time_used;

        ArgumentModel<e_uplo,
                      e_N,
                      e_K,
                      e_alpha,
                      e_lda,
                      e_stride_a,
                      e_incx,
                      e_stride_x,
                      e_beta,
                      e_incy,
                      e_stride_y,
                      e_batch_count>{}
            .log_args<T>(rocblas_cout,
                         arg,
                         gpu_time_used,
                         hbmv_gflop_count<T>(N, K),
                         hbmv_gbyte_count<T>(N, K),
                         cpu_time_used,
                         rocblas_error_host,
                         rocblas_error_device);
    }
}
