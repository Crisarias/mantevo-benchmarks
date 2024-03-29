INCLUDE(TribitsAddExecutableAndTest)   

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(SOURCES
  UnitTestMain.cpp 
  TestSerial.cpp 
  TestThreads.cpp 
  TestHWLOC.cpp 
  )

IF(Kokkos_ENABLE_OpenMP)
  LIST( APPEND SOURCES TestOpenMP.cpp )
ENDIF()

IF(Kokkos_ENABLE_Cuda)

  # Add subpackage include path dependencies for CUDA_COMPILE().
  CUDA_INCLUDE_DIRECTORIES( ${KokkosCore_INCLUDE_DIRS} )
  CUDA_COMPILE(OBJECTS_CUDA TestCudaFunctions.cu)

  LIST( APPEND SOURCES TestCuda.cpp ${OBJECTS_CUDA} )

ENDIF()

TRIBITS_ADD_EXECUTABLE_AND_TEST(
  ArrayUnitTest
  SOURCES ${SOURCES}
  COMM serial mpi
  NUM_MPI_PROCS 1
  FAIL_REGULAR_EXPRESSION "  FAILED  "
  DEPLIBS kokkoscore
  )
  
