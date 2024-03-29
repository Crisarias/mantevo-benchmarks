! ************************************************************************
!
!               miniGhost: stencil computations with boundary exchange.
!                 Copyright (2012) Sandia Corporation
!
! Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
! license for use of this work by or on behalf of the U.S. Government.
!
! This library is free software; you can redistribute it and/or modify
! it under the terms of the GNU Lesser General Public License as
! published by the Free Software Foundation; either version 2.1 of the
! License, or (at your option) any later version.
!
! This library is distributed in the hope that it will be useful, but
! WITHOUT ANY WARRANTY; without even the implied warranty of
! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
! Lesser General Public License for more details.
!
! You should have received a copy of the GNU Lesser General Public
! License along with this library; if not, write to the Free Software
! Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
! USA
! Questions? Contact Richard F. Barrett (rfbarre@sandia.gov) or
!                    Michael A. Heroux (maherou@sandia.gov)
!
! ************************************************************************

MODULE MG_ALLREDUCE_MOD

   ! Purpose
   ! =======
   ! Summation of the grid elements in GRID_IN.

   USE MG_CONSTANTS_MOD
   USE MG_UTILS_MOD
   USE MG_PROFILING_MOD

   IMPLICIT NONE

CONTAINS

   SUBROUTINE MG_ALLREDUCE_SUM ( IVAR, GRID_IN, GSUM, IERR )

      ! ---------------------
      ! Argument Declarations
      ! ---------------------

      INTEGER, INTENT(IN) :: &
         IVAR

      REAL(KIND=MG_REAL), DIMENSION(0:NX+1, 0:NY+1, 0:NZ+1), INTENT(INOUT) :: &
         GRID_IN

      REAL(KIND=MG_REAL), INTENT(OUT) :: &
         GSUM                                ! Global value.

      INTEGER, INTENT(OUT) :: &
         IERR           ! Return status

      ! ------------------
      ! Local Declarations
      ! ------------------

      REAL(KIND=MG_REAL) :: &
         LSUM                 ! Local GRID sum.
      REAL(KIND=MG_REAL) :: gflux

      REAL(KIND=MG_REAL8) ::      &
         TIME_START           ! Timer.

      INTEGER :: &
         I, J, K              ! Counters

      ! ---------------------
      ! Executable Statements
      ! ---------------------

      IERR = 0

      TIME_START = MG_TIMER ()
      LSUM = 0.0

#if defined _MG_OACC
!$acc data present_or_copyin ( LSUM )
!end data
!$acc data present ( GRID_IN )
!$acc parallel loop reduction ( +:LSUM )
#endif

      DO K = 1, NZ
         DO J = 1, NY
            DO I = 1, NX
               LSUM = LSUM + GRID_IN(I, J, K)
            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end parallel loop
!$acc end data   ! GRID_IN array.
!$acc update host ( LSUM )
!$acc end data    ! LSUM 
#endif

      LSUM = LSUM + FLUX_OUT(IVAR)

      MG_PERF%TIME_SUMGRID_COMP_PE = MG_PERF%TIME_SUMGRID_COMP_PE + MG_TIMER () - TIME_START

#if defined _MG_MPI
      TIME_START = MG_TIMER()
      CALL MPI_ALLREDUCE ( LSUM, GSUM,  1, MG_MPI_REAL, MPI_SUM, MPI_COMM_MG, IERR )

      MG_PERF%TIME_SUMGRID_COMM_PE = MG_PERF%TIME_SUMGRID_COMM_PE + MG_TIMER () - TIME_START

      MG_PERF%NUM_ALLREDUCES = MG_PERF%NUM_ALLREDUCES + 1
      MG_PERF%ALLREDUCE_COUNT = MG_PERF%ALLREDUCE_COUNT + 1

      MG_PERF%ALLREDUCE_COUNT_MAX = 1
      MG_PERF%ALLREDUCE_COUNT_MIN = 1
#else
      GSUM = LSUM
#endif

   END SUBROUTINE MG_ALLREDUCE_SUM

END MODULE MG_ALLREDUCE_MOD
