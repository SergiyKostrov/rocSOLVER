/* ************************************************************************
 * Copyright (c) 2018-2022 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once

#include <cinttypes>
#include <cstdio>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <rocblas/rocblas.h>

#include "rocblas_init.hpp"
#include "rocblas_test.hpp"

/* ============================================================================================
 */
/*! \brief  base-class to allocate/deallocate device memory */
template <typename T, size_t PAD, typename U>
class d_vector
{
private:
    size_t size, bytes;

public:
    inline size_t nmemb() const noexcept
    {
        return size;
    }

#ifdef ROCSOLVER_CLIENTS_TEST
    U guard[PAD];
    d_vector(size_t s)
        : size(s)
        , bytes((s + PAD * 2) * sizeof(T))
    {
        // Initialize guard with random data
        if(PAD > 0)
        {
            rocblas_init_nan(guard, PAD);
        }
    }
#else
    d_vector(size_t s)
        : size(s)
        , bytes(s ? s * sizeof(T) : sizeof(T))
    {
    }
#endif

    T* device_vector_setup()
    {
        T* d;
        if((hipMalloc)(&d, bytes) != hipSuccess)
        {
            fmt::print(stderr, "Error allocating {} bytes ({} GB)\n", bytes, bytes >> 30);
            d = nullptr;
        }
#ifdef ROCSOLVER_CLIENTS_TEST
        else
        {
            if(PAD > 0)
            {
                // Copy guard to device memory before allocated memory
                hipMemcpy(d, guard, sizeof(guard), hipMemcpyHostToDevice);

                // Point to allocated block
                d += PAD;

                // Copy guard to device memory after allocated memory
                hipMemcpy(d + size, guard, sizeof(guard), hipMemcpyHostToDevice);
            }
        }
#endif
        return d;
    }

    void device_vector_check(T* d)
    {
#ifdef ROCSOLVER_CLIENTS_TEST
        if(PAD > 0)
        {
            U host[PAD];

            // Copy device memory after allocated memory to host
            hipMemcpy(host, d + this->size, sizeof(guard), hipMemcpyDeviceToHost);

            // Make sure no corruption has occurred
            EXPECT_EQ(memcmp(host, guard, sizeof(guard)), 0);

            // Point to guard before allocated memory
            d -= PAD;

            // Copy device memory after allocated memory to host
            hipMemcpy(host, d, sizeof(guard), hipMemcpyDeviceToHost);

            // Make sure no corruption has occurred
            EXPECT_EQ(memcmp(host, guard, sizeof(guard)), 0);
        }
#endif
    }

    void device_vector_teardown(T* d)
    {
        if(d != nullptr)
        {
#ifdef ROCSOLVER_CLIENTS_TEST
            if(PAD > 0)
            {
                U host[PAD];

                // Copy device memory after allocated memory to host
                hipMemcpy(host, d + this->size, sizeof(guard), hipMemcpyDeviceToHost);

                // Make sure no corruption has occurred
                EXPECT_EQ(memcmp(host, guard, sizeof(guard)), 0);

                // Point to guard before allocated memory
                d -= PAD;

                // Copy device memory after allocated memory to host
                hipMemcpy(host, d, sizeof(guard), hipMemcpyDeviceToHost);

                // Make sure no corruption has occurred
                EXPECT_EQ(memcmp(host, guard, sizeof(guard)), 0);
            }
#endif
            // Free device memory
            CHECK_HIP_ERROR((hipFree)(d));
        }
    }
};
