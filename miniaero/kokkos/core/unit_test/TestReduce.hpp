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

#include <stdexcept>
#include <sstream>
#include <iostream>

#include <Kokkos_ParallelReduce.hpp>

/*--------------------------------------------------------------------------*/

namespace Test {

template< typename ScalarType , class DeviceType >
class ReduceFunctor
{
public:
  typedef DeviceType  device_type ;
  typedef typename device_type::size_type size_type ;

  struct value_type {
    ScalarType value[3] ;
  };

  const size_type nwork ;

  ReduceFunctor( const size_type & arg_nwork ) : nwork( arg_nwork ) {}

  ReduceFunctor( const ReduceFunctor & rhs )
    : nwork( rhs.nwork ) {}

  KOKKOS_INLINE_FUNCTION
  void init( value_type & dst ) const
  {
    dst.value[0] = 0 ;
    dst.value[1] = 0 ;
    dst.value[2] = 0 ;
  }

  KOKKOS_INLINE_FUNCTION
  void join( volatile value_type & dst ,
             const volatile value_type & src ) const
  {
    dst.value[0] += src.value[0] ;
    dst.value[1] += src.value[1] ;
    dst.value[2] += src.value[2] ;
  }

  KOKKOS_INLINE_FUNCTION
  void operator()( size_type iwork , value_type & dst ) const
  {
    dst.value[0] += 1 ;
    dst.value[1] += iwork + 1 ;
    dst.value[2] += nwork - iwork ;
  }
};

template< class DeviceType >
class ReduceFunctorFinal : public ReduceFunctor< long , DeviceType > {
public:

  typedef typename ReduceFunctor< long , DeviceType >::value_type value_type ;

  ReduceFunctorFinal( const size_t n )
    : ReduceFunctor<long,DeviceType>(n)
    {}

  KOKKOS_INLINE_FUNCTION
  void final( value_type & dst ) const
  {
    dst.value[0] = - dst.value[0] ;
    dst.value[1] = - dst.value[1] ;
    dst.value[2] = - dst.value[2] ;
  }
};

template< typename ScalarType , class DeviceType >
class RuntimeReduceFunctor
{
public:
  // Required for functor:
  typedef DeviceType  device_type ;
  typedef ScalarType  value_type[] ;
  const unsigned      value_count ;


  // Unit test details:

  typedef typename device_type::size_type  size_type ;

  const size_type     nwork ;

  RuntimeReduceFunctor( const size_type arg_nwork ,
                        const size_type arg_count )
    : value_count( arg_count )
    , nwork( arg_nwork ) {}

  KOKKOS_INLINE_FUNCTION
  void init( value_type dst ) const
  {
    for ( unsigned i = 0 ; i < value_count ; ++i ) dst[i] = 0 ;
  }

  KOKKOS_INLINE_FUNCTION
  void join( volatile ScalarType dst[] ,
             const volatile ScalarType src[] ) const
  {
    for ( unsigned i = 0 ; i < value_count ; ++i ) dst[i] += src[i] ;
  }

  KOKKOS_INLINE_FUNCTION
  void operator()( size_type iwork , ScalarType dst[] ) const
  {
    const size_type tmp[3] = { 1 , iwork + 1 , nwork - iwork };

    for ( size_type i = 0 ; i < value_count ; ++i ) {
      dst[i] += tmp[ i % 3 ];
    }
  }
};

template< class DeviceType >
class RuntimeReduceFunctorFinal : public RuntimeReduceFunctor< long , DeviceType > {
public:

  typedef RuntimeReduceFunctor< long , DeviceType > base_type ;
  typedef typename base_type::value_type value_type ;
  typedef long scalar_type ;

  RuntimeReduceFunctorFinal( const size_t nwork , const size_t count ) : base_type(nwork,count) {}

  KOKKOS_INLINE_FUNCTION
  void final( value_type dst ) const
  {
    for ( unsigned i = 0 ; i < base_type::value_count ; ++i ) {
      dst[i] = - dst[i] ;
    }
  }
};
} // namespace Test

namespace {

template< typename ScalarType , class DeviceType >
class TestReduce
{
public:
  typedef DeviceType    device_type ;
  typedef typename device_type::size_type size_type ;

  //------------------------------------

  TestReduce( const size_type & nwork )
  {
    run_test(nwork);
    run_test_final(nwork);
  }

  void run_test( const size_type & nwork )
  {
    typedef Test::ReduceFunctor< ScalarType , device_type > functor_type ;
    typedef typename functor_type::value_type value_type ;

    enum { Count = 3 };
    enum { Repeat = 100 };

    value_type result[ Repeat ];

    const unsigned long nw   = nwork ;
    const unsigned long nsum = nw % 2 ? nw * (( nw + 1 )/2 )
                                      : (nw/2) * ( nw + 1 );

    for ( unsigned i = 0 ; i < Repeat ; ++i ) {
      Kokkos::parallel_reduce( nwork , functor_type(nwork) , result[i] );
    }

    for ( unsigned i = 0 ; i < Repeat ; ++i ) {
      for ( unsigned j = 0 ; j < Count ; ++j ) {
        const unsigned long correct = 0 == j % 3 ? nw : nsum ;
        ASSERT_EQ( (ScalarType) correct , result[i].value[j] );
      }
    }
  }

  void run_test_final( const size_type & nwork )
  {
    typedef Test::ReduceFunctorFinal< device_type > functor_type ;
    typedef typename functor_type::value_type value_type ;

    enum { Count = 3 };
    enum { Repeat = 100 };

    value_type result[ Repeat ];

    const unsigned long nw   = nwork ;
    const unsigned long nsum = nw % 2 ? nw * (( nw + 1 )/2 )
                                      : (nw/2) * ( nw + 1 );

    for ( unsigned i = 0 ; i < Repeat ; ++i ) {
      Kokkos::parallel_reduce( nwork , functor_type(nwork) , result[i] );
    }

    for ( unsigned i = 0 ; i < Repeat ; ++i ) {
      for ( unsigned j = 0 ; j < Count ; ++j ) {
        const unsigned long correct = 0 == j % 3 ? nw : nsum ;
        ASSERT_EQ( (ScalarType) correct , - result[i].value[j] );
      }
    }
  }
};

template< typename ScalarType , class DeviceType >
class TestReduceDynamic
{
public:
  typedef DeviceType    device_type ;
  typedef typename device_type::size_type size_type ;

  //------------------------------------

  TestReduceDynamic( const size_type nwork )
  {
    run_test_dynamic(nwork);
    run_test_dynamic_final(nwork);
  }

  void run_test_dynamic( const size_type nwork )
  {
    typedef Test::RuntimeReduceFunctor< ScalarType , device_type > functor_type ;

    enum { Count = 3 };
    enum { Repeat = 100 };

    ScalarType result[ Repeat ][ Count ] ;

    const unsigned long nw   = nwork ;
    const unsigned long nsum = nw % 2 ? nw * (( nw + 1 )/2 )
                                      : (nw/2) * ( nw + 1 );

    for ( unsigned i = 0 ; i < Repeat ; ++i ) {
      Kokkos::parallel_reduce( nwork , functor_type(nwork,Count) , result[i] );
    }

    for ( unsigned i = 0 ; i < Repeat ; ++i ) {
      for ( unsigned j = 0 ; j < Count ; ++j ) {
        const unsigned long correct = 0 == j % 3 ? nw : nsum ;
        ASSERT_EQ( (ScalarType) correct , result[i][j] );
      }
    }
  }

  void run_test_dynamic_final( const size_type nwork )
  {
    typedef Test::RuntimeReduceFunctorFinal< device_type > functor_type ;

    enum { Count = 3 };
    enum { Repeat = 100 };

    typename functor_type::scalar_type result[ Repeat ][ Count ] ;

    const unsigned long nw   = nwork ;
    const unsigned long nsum = nw % 2 ? nw * (( nw + 1 )/2 )
                                      : (nw/2) * ( nw + 1 );

    for ( unsigned i = 0 ; i < Repeat ; ++i ) {
      Kokkos::parallel_reduce( nwork , functor_type(nwork,Count) , result[i] );
    }

    for ( unsigned i = 0 ; i < Repeat ; ++i ) {
      for ( unsigned j = 0 ; j < Count ; ++j ) {
        const unsigned long correct = 0 == j % 3 ? nw : nsum ;
        ASSERT_EQ( (ScalarType) correct , - result[i][j] );
      }
    }
  }
};

template< typename ScalarType , class DeviceType >
class TestReduceDynamicView
{
public:
  typedef DeviceType    device_type ;
  typedef typename device_type::size_type size_type ;

  //------------------------------------

  TestReduceDynamicView( const size_type nwork )
  {
    run_test_dynamic_view(nwork);
  }

  void run_test_dynamic_view( const size_type nwork )
  {
    typedef Test::RuntimeReduceFunctor< ScalarType , device_type > functor_type ;

    typedef Kokkos::View< ScalarType* , DeviceType > result_type ;
    typedef typename result_type::HostMirror result_host_type ;

    const unsigned CountLimit = 23 ;

    const unsigned long nw   = nwork ;
    const unsigned long nsum = nw % 2 ? nw * (( nw + 1 )/2 )
                                      : (nw/2) * ( nw + 1 );

    for ( unsigned count = 0 ; count < CountLimit ; ++count ) {

      result_type result("result",count);
      result_host_type host_result = Kokkos::create_mirror( result );

      // Test result to host pointer:

      Kokkos::parallel_reduce( nw , functor_type(nw,count) , host_result.ptr_on_device() );

      for ( unsigned j = 0 ; j < count ; ++j ) {
        const unsigned long correct = 0 == j % 3 ? nw : nsum ;
        ASSERT_EQ( host_result(j), (ScalarType) correct );
        host_result(j) = 0 ;
      }
    }
  }
};

}

/*--------------------------------------------------------------------------*/

