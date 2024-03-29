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

namespace Test {

template< typename Scalar , class DeviceType>
struct computeCellMeasure;

template<typename Scalar>
struct computeCellMeasure<Scalar, KOKKOS_MACRO_DEVICE>{

	typedef KOKKOS_MACRO_DEVICE     							device_type ;
  	typedef typename Kokkos::MDArrayView<Scalar,device_type> 	array_type ;

	array_type inDet;
	array_type inWeights;
	array_type outVals;

  	computeCellMeasure(	array_type & arg_outVals,
						array_type & arg_inDet, 
						array_type & arg_inWeights):
		inDet		(arg_inDet), 
		inWeights	(arg_inWeights), 
		outVals		(arg_outVals)
	{}


	KOKKOS_MACRO_DEVICE_FUNCTION
  	void operator()( int ielem )const {

		for(unsigned int point = 0; point < outVals.dimension(1); point++){

			outVals(ielem, point) = inDet(ielem, point) * inWeights(point);
			
		}// for, cubature

		if(inDet(ielem, 0) < 0.0){			

			for(unsigned int point = 0; point < outVals.dimension(1); point++){

				outVals(ielem, point) *= -1;
		
			}// for, point

		}// if

	}

}; // struct

} // namespace

