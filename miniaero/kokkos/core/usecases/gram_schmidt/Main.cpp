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

#include <ParallelComm.hpp>

#include <stdlib.h>
#include <string>
#include <iostream>

#include <Kokkos_Macros.hpp>
#include <Kokkos_hwloc.hpp>
#include <GramSchmidt.hpp>

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int main( int argc , char ** argv )
{
  comm::Machine machine = comm::Machine::init( & argc , & argv );

  // const int comm_size = comm::size( machine );
  const int comm_rank = comm::rank( machine );
  const unsigned numa_count       = Kokkos::hwloc::get_available_numa_count();
  const unsigned cores_per_numa   = Kokkos::hwloc::get_available_cores_per_numa();
  const unsigned threads_per_core = Kokkos::hwloc::get_available_threads_per_core();

  const unsigned thread_capacity = numa_count * cores_per_numa * threads_per_core ;
  const unsigned gang_worker_capacity = cores_per_numa * threads_per_core ;

  const int cuda_device_count =
#if defined( KOKKOS_HAVE_CUDA )
    Kokkos::Cuda::detect_device_count() ;
#else
    0 ;
#endif

  int error = 0 ;
  int gang_count  = 0 ;
  int gang_worker = 0 ;
  int cuda_device = -1 ;

  int test_length_begin = 0 ;
  int test_length_end   = 0 ;
  int test_count        = 0 ;
  int test_iter         = 0 ;

  if ( 0 == comm_rank ) {

    error = argc <= 1 ;

    for ( int i = 1 ; 0 == error && i < argc ; ++i ) {
      const std::string arg( argv[i] );

      if ( arg == "host" ) {
        gang_count  = atoi( argv[++i] );
        gang_worker = atoi( argv[++i] );
      }
      else if ( arg == "cuda" ) {
        cuda_device = atoi( argv[++i] );
      }
      else if ( arg == "test" ) {
        test_length_begin = atoi( argv[++i] );
        test_length_end   = atoi( argv[++i] );
        test_count  = atoi( argv[++i] );
        test_iter   = atoi( argv[++i] );
        if ( test_length_end <= test_length_begin ||
             test_length_begin <= 0 ||
             test_count        <= 0 ) {
          error = 1 ;
        }
      }
      else {
        error = 1 ;
      }

      error = error
            || argc <= i
            || thread_capacity      < unsigned( gang_count * gang_worker )
            || gang_worker_capacity < unsigned( gang_worker )
            || cuda_device_count <= cuda_device
            ;
    }

    if ( error ) {
      std::cout << "input syntax:" << std::endl
                << argv[0] << " ( "
                << "host #GANG #WORKER | "
                << "cuda DEVICE# | "
                << "test #LENGTH_BEGIN #LENGTH_END #COUNT #ITER )"
                << std::endl
                << "thread capacity = " << numa_count << "x" << gang_worker_capacity
                << std::endl
                << "cuda devices = "
                << cuda_device_count
                << std::endl ;
    }
  }

#if defined( KOKKOS_HAVE_MPI )
  {
    int data[16] = { error , gang_count , gang_worker , cuda_device ,
                     test_length_begin , test_length_end , test_count , test_iter };

    MPI_Bcast( data , 16 , MPI_INT , 0 , machine.mpi_comm );

    error             = data[0] ;
    gang_count        = data[1] ;
    gang_worker       = data[2] ;
    cuda_device       = data[3] ;
    test_length_begin = data[4] ;
    test_length_end   = data[5] ;
    test_count        = data[6] ;
    test_iter         = data[7] ;
  }
#endif

  if ( 0 == error ) {

    if ( gang_count && gang_worker ) {
      Kokkos::Threads::initialize( gang_count * gang_worker );

      if ( test_iter ) {
        Test::driver_modified_gram_schmidt<Kokkos::Threads>
          ( test_length_begin ,
            test_length_end ,
            test_count ,
            test_iter ,
            machine );
      }
      else {
        Kokkos::Threads::print_configuration( std::cout );
      }

      Kokkos::Threads::finalize();
    }

#if defined( KOKKOS_HAVE_CUDA )
    if ( 0 <= cuda_device ) {
      Kokkos::Cuda::host_mirror_device_type::initialize();
      Kokkos::Cuda::SelectDevice select( ( cuda_device + comm_rank ) % cuda_device_count );
      Kokkos::Cuda::initialize( select );

      if ( test_iter ) {
        Test::driver_modified_gram_schmidt<Kokkos::Cuda>
          ( test_length_begin ,
            test_length_end ,
            test_count ,
            test_iter ,
            machine );
      }
      else {
        Kokkos::Cuda::print_configuration( std::cout );
      }

      Kokkos::Cuda::finalize();
      Kokkos::Cuda::host_mirror_device_type::finalize();
    }
#endif

  }

  comm::Machine::finalize();

  return 0 ;
}

