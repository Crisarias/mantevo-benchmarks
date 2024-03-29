INCLUDE(TribitsPackageMacros)

#
# A) Forward delcare the package so that certain options are also defined for
# subpackages
#

TRIBITS_PACKAGE_DECL(Kokkos) # ENABLE_SHADOWING_WARNINGS)

#------------------------------------------------------------------------------
#
# B) Define the common options for Kokkos first so they can be used by
# subpackages as well.
#

TRIBITS_ADD_DEBUG_OPTION()

TRIBITS_ADD_OPTION_AND_DEFINE(
  Kokkos_ENABLE_Cuda
  KOKKOS_HAVE_CUDA
  "Enable CUDA support in Kokkos (new)."
  "${TPL_ENABLE_CUDA}"
  )

TRIBITS_ADD_OPTION_AND_DEFINE(
  Kokkos_ENABLE_Pthread
  KOKKOS_HAVE_PTHREAD
  "Enable Pthread support in Kokkos."
  "${TPL_ENABLE_Pthread}"
  )

TRIBITS_ADD_OPTION_AND_DEFINE(
  Kokkos_ENABLE_OpenMP
  KOKKOS_HAVE_OPENMP
  "Enable OpenMP support in Kokkos."
  "${Trilinos_ENABLE_OpenMP}"
  )

TRIBITS_ADD_OPTION_AND_DEFINE(
  Kokkos_ENABLE_HWLOC
  KOKKOS_HAVE_HWLOC
  "Enable HWLOC support in Kokkos."
  "${TPL_ENABLE_HWLOC}"
  )

TRIBITS_ADD_OPTION_AND_DEFINE(
  Kokkos_ENABLE_MPI
  KOKKOS_HAVE_MPI
  "Enable MPI support in Kokkos."
  "${TPL_ENABLE_MPI}"
  )

TRIBITS_ADD_OPTION_AND_DEFINE(
  Kokkos_ENABLE_BOUNDS_CHECK
  KOKKOS_EXPRESSION_CHECK
  "Enable bounds checking support in Kokkos."
  OFF
  )

#------------------------------------------------------------------------------
#
# C) Process the subpackages for Kokkos
#

TRIBITS_PROCESS_SUBPACKAGES()

#
# D) If Kokkos itself is enabled, process the Kokkos package
#

TRIBITS_PACKAGE_DEF()

TRIBITS_EXCLUDE_AUTOTOOLS_FILES()

TRIBITS_EXCLUDE_FILES(
  classic/doc
  classic/DistTSQR/doc
  classic/LinAlg/doc/CrsRefactorNotesMay2012
  )

TRIBITS_PACKAGE_POSTPROCESS()

