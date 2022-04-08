/* ************************************************************************
 * Copyright 2018-2022 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once

//
// Local declaration of the host strided batch matrix.
//
template <typename T>
class host_strided_batch_matrix;

//!
//! @brief Implementation of a strided batched matrix on device.
//!
template <typename T>
class device_strided_batch_matrix : public d_vector<T>
{
public:
    //!
    //! @brief Disallow copying.
    //!
    device_strided_batch_matrix(const device_strided_batch_matrix&) = delete;

    //!
    //! @brief Disallow assigning.
    //!
    device_strided_batch_matrix& operator=(const device_strided_batch_matrix&) = delete;

    //!
    //! @brief Constructor.
    //! @param m           The number of rows of the Matrix.
    //! @param n           The number of cols of the Matrix.
    //! @param lda         The leading dimension of the Matrix.
    //! @param stride The stride.
    //! @param batch_count The batch count.
    //! @param HMM         HipManagedMemory Flag.
    //!
    explicit device_strided_batch_matrix(size_t         m,
                                         size_t         n,
                                         size_t         lda,
                                         rocblas_stride stride,
                                         rocblas_int    batch_count,
                                         bool           HMM = false)
        : d_vector<T>(calculate_nmemb(n, lda, stride, batch_count), HMM)
        , m_m(m)
        , m_n(n)
        , m_lda(lda)
        , m_stride(stride)
        , m_batch_count(batch_count)
    {
        bool valid_parameters = calculate_nmemb(n, lda, stride, batch_count) > 0;
        if(valid_parameters)
        {
            this->m_data = this->device_vector_setup();
        }
    }

    //!
    //! @brief Destructor.
    //!
    ~device_strided_batch_matrix()
    {
        if(nullptr != this->m_data)
        {
            this->device_vector_teardown(this->m_data);
            this->m_data = nullptr;
        }
    }

    //!
    //! @brief Returns the data pointer.
    //!
    T* data()
    {
        return this->m_data;
    }

    //!
    //! @brief Returns the data pointer.
    //!
    const T* data() const
    {
        return this->m_data;
    }

    //!
    //! @brief Returns the rows of the Matrix.
    //!
    size_t m() const
    {
        return this->m_m;
    }

    //!
    //! @brief Returns the cols of the Matrix.
    //!
    size_t n() const
    {
        return this->m_n;
    }

    //!
    //! @brief Returns the leading dimension of the Matrix.
    //!
    size_t lda() const
    {
        return this->m_lda;
    }

    //!
    //! @brief Returns the batch count.
    //!
    rocblas_int batch_count() const
    {
        return this->m_batch_count;
    }

    //!
    //! @brief Returns the stride value.
    //!
    rocblas_stride stride() const
    {
        return this->m_stride;
    }

    //!
    //! @brief Returns pointer.
    //! @param batch_index The batch index.
    //! @return A mutable pointer to the batch_index'th matrix.
    //!
    T* operator[](rocblas_int batch_index)
    {
        return (this->m_stride >= 0)
                   ? this->m_data + batch_index * this->m_stride
                   : this->m_data + (batch_index + 1 - this->m_batch_count) * this->m_stride;
    }

    //!
    //! @brief Returns non-mutable pointer.
    //! @param batch_index The batch index.
    //! @return A non-mutable mutable pointer to the batch_index'th matrix.
    //!
    const T* operator[](rocblas_int batch_index) const
    {
        return (this->m_stride >= 0)
                   ? this->m_data + batch_index * this->m_stride
                   : this->m_data + (batch_index + 1 - this->m_batch_count) * this->m_stride;
    }

    //!
    //! @brief Cast operator.
    //! @remark Returns the pointer of the first matrix.
    //!
    operator T*()
    {
        return (*this)[0];
    }

    //!
    //! @brief Non-mutable cast operator.
    //! @remark Returns the non-mutable pointer of the first matrix.
    //!
    operator const T*() const
    {
        return (*this)[0];
    }

    //!
    //! @brief Tell whether ressources allocation failed.
    //!
    explicit operator bool() const
    {
        return nullptr != this->m_data;
    }

    //!
    //! @brief Transfer data from a strided batched matrix on device.
    //! @param that That strided batched matrix on device.
    //! @return The hip error.
    //!
    hipError_t transfer_from(const host_strided_batch_matrix<T>& that)
    {
        return hipMemcpy(this->data(),
                         that.data(),
                         sizeof(T) * this->nmemb(),
                         this->use_HMM ? hipMemcpyHostToHost : hipMemcpyHostToDevice);
    }

    //!
    //! @brief Check if memory exists.
    //! @return hipSuccess if memory exists, hipErrorOutOfMemory otherwise.
    //!
    hipError_t memcheck() const
    {
        if(*this)
            return hipSuccess;
        else
            return hipErrorOutOfMemory;
    }

private:
    size_t         m_m{};
    size_t         m_n{};
    size_t         m_lda{};
    rocblas_stride m_stride{};
    rocblas_int    m_batch_count{};
    T*             m_data{};

    static size_t
        calculate_nmemb(size_t n, size_t lda, rocblas_stride stride, rocblas_int batch_count)
    {
        return lda * n + size_t(batch_count - 1) * std::abs(stride);
    }
};