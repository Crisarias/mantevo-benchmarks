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


template <class Scalar , class DeviceType >
struct Transform; 

template<class Scalar >
struct Transform<Scalar , KOKKOS_MACRO_DEVICE >
{
	typedef KOKKOS_MACRO_DEVICE 		device_type;
	typedef device_type::size_type 		size_type;
	
	typedef typename Kokkos::MDArrayView<Scalar,device_type> array_type;
	typedef typename Kokkos::MDArrayView<Scalar,Kokkos::DeviceHost> host_array;
	
  private:
	
	array_type output;
	array_type input;
	array_type fields;  
	array_type basisGrads;
	int data_rank;
	int numDataPts;
	int in_rank;
	int numCells;
	int numFields;
	int numPoints;
	int dim;
	
  public:
  
	Transform(	array_type 			& arg_output , 
				const array_type	& arg_input ,
				const array_type	& arg_fields  ) : output(arg_output) , input(arg_input) , fields(arg_fields)
	{
		data_rank = input.rank();
		numDataPts = input.dimension(1);
		in_rank = fields.rank();
		numCells = output.dimension(0);
		numFields = output.dimension(1);
		numPoints = output.dimension(2);
		dim = output.dimension(3);		
	}
  	
  	//Assume fields is rank 3, input is rank 4 and transpose is on
  	KOKKOS_MACRO_DEVICE_FUNCTION
  	void operator()(size_type ielem) const 
  	{
		for(int field = 0; field < numFields; field++){
          for(int point = 0; point < numPoints; point++){
            for(int row = 0; row < dim; row++){
              output(ielem, field, point, row) = 0.0;
              for(int col = 0; col < dim; col++){
                output(ielem, field, point, row) += \
                  input(ielem, point, col, row)*fields(field, point, col);
              }// col
            } //row
          }// point
        }// field
  	}

};




} // namespace Test
