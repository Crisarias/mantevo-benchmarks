/*
//@HEADER
// ************************************************************************
//
//   Kokkos: Manycore Performance-Portable Multidimensional Arrays
//              Copyright (2012) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact  H. Carter Edwards (hcedwar@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#include <iostream>

#include <gtest/gtest.h>

#include <Kokkos_Cuda.hpp>
#include <Kokkos_OpenMP.hpp>

namespace Test {

class cuda : public ::testing::Test {
protected:
  static void SetUpTestCase()
  {
    Kokkos::Cuda::print_configuration( std::cout );
    Kokkos::Cuda::host_mirror_device_type::initialize();
    Kokkos::Cuda::initialize( Kokkos::Cuda::SelectDevice(0) );
  }
  static void TearDownTestCase()
  {
    Kokkos::Cuda::finalize();
    Kokkos::Cuda::host_mirror_device_type::finalize();
  }
};

extern void test_device_cuda_tile();
extern void test_device_cuda_view_impl();
extern void test_device_cuda_view_api();
extern void test_device_cuda_crsarray();
extern void test_device_cuda_reduce();
extern void test_device_cuda_reduce_request();
extern void test_device_cuda_reduce_dynamic();
extern void test_device_cuda_reduce_dynamic_view();
extern void test_device_cuda_multi_reduce();
extern void test_device_cuda_shared_request();
extern void test_device_cuda_view_aggregate();
extern void test_device_cuda_atomic();
extern void test_device_cuda_scan();
extern void test_device_cuda_team_scan();
extern void test_device_cuda_compiler_macros();

TEST_F( cuda, view_impl )
{
  test_device_cuda_view_impl();
}

TEST_F( cuda, view_api )
{
  test_device_cuda_view_api();
}

TEST_F( cuda, crsarray )
{
  test_device_cuda_crsarray();
}

TEST_F( cuda, reduce )
{
  test_device_cuda_reduce();
}

TEST_F( cuda, reduce_request )
{
  test_device_cuda_reduce_request();
}

TEST_F( cuda, shared_request )
{
  test_device_cuda_shared_request();
}

TEST_F( cuda, reduce_dynamic )
{
  test_device_cuda_reduce_dynamic();
}

TEST_F( cuda, reduce_dynamic_view )
{
  test_device_cuda_reduce_dynamic_view();
}

TEST_F( cuda, multi_reduce )
{
  test_device_cuda_multi_reduce();
}

TEST_F( cuda, tile )
{
  test_device_cuda_tile();
}

TEST_F( cuda , view_aggregate )
{
  test_device_cuda_view_aggregate();
}

TEST_F( cuda, atomic )
{
  test_device_cuda_atomic();
}

TEST_F( cuda , scan )
{
  test_device_cuda_scan();
}

TEST_F( cuda , team_scan )
{
  test_device_cuda_team_scan();
}

TEST_F( cuda , compiler_macros )
{
  test_device_cuda_compiler_macros();
}

}

