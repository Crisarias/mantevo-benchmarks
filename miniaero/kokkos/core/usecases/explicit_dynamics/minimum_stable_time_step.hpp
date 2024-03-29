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

template<typename Scalar , class DeviceType>
struct minimum_stable_time_step;

template<typename Scalar>
struct minimum_stable_time_step<Scalar ,KOKKOS_MACRO_DEVICE>{

  typedef KOKKOS_MACRO_DEVICE       device_type;
  typedef device_type::size_type    size_type;

  typedef Scalar value_type;

  typedef Region<Scalar,device_type> MyRegion;

    minimum_stable_time_step( const MyRegion  & arg_region)
       : region(arg_region)
      {}


    KOKKOS_INLINE_FUNCTION
    static void init(value_type &update) {
      update = 1.0e32;
    }

    KOKKOS_INLINE_FUNCTION
    static void join(volatile value_type &update, const volatile value_type & source) {
      update = update < source ? update : source;
    }


    KOKKOS_INLINE_FUNCTION
    void operator()(int ielem, value_type & update) const {
      value_type source = region.elem_t_step(ielem);
      update = update < source ? update : source;
    }

    MyRegion   region;

}; //minimum_stable_time_step




template<typename Scalar , class DeviceType>
struct set_next_time_step;

template<typename Scalar>
struct set_next_time_step<Scalar ,KOKKOS_MACRO_DEVICE>{

  typedef KOKKOS_MACRO_DEVICE       device_type;
  typedef device_type::size_type    size_type;

  typedef Scalar value_type;

  typedef Region<Scalar,device_type> MyRegion;

    set_next_time_step(
                const MyRegion  & arg_region,
                const int       arg_next_state)
       : region(arg_region)
       , next_state(arg_next_state)
      {}


    KOKKOS_INLINE_FUNCTION
    void operator()(Scalar & result) const {
      region.delta_t(next_state) = result;
    }

    MyRegion   region;
    const int  next_state;

}; //minimum_stable_time_step

