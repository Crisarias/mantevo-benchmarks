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

MODULE MG_STENCIL_COMPS_MOD

   USE MG_UTILS_MOD
   USE MG_FLUX_ACCUMULATE_MOD
   USE MG_PROFILING_MOD

   IMPLICIT NONE

   ! Stencil computations for a single variable.
   ! WORK = F(GRID), then GRID = WORK. WORK array globally accessible.
   !
   ! MG_STENCIL_2D5PT
   ! MG_STENCIL_2D9PT
   ! MG_STENCIL_3D7PT
   ! MG_STENCIL_3D27PT

   ! Note: Using 0-based indexing on arrays.

CONTAINS
   
   SUBROUTINE MG_STENCIL_2D5PT ( IVAR, GRID, IERR )
   
      ! ---------------------
      ! Argument Declarations
      ! ---------------------

      INTEGER, INTENT(IN) :: &
         IVAR

      INTEGER, INTENT(OUT) :: &
         IERR           ! Return status

      REAL(KIND=MG_REAL), DIMENSION(0:NX+1, 0:NY+1, 0:NZ+1), INTENT(INOUT) :: &
         GRID

      ! ------------------
      ! Local Declarations
      ! ------------------

      INTEGER ::         &
         I, J, K,        &  ! Counters
         NUM_GRID_PTS

      ! ---------------------
      ! Executable Statements
      ! ---------------------
   
#if defined _MG_OACC
!$acc data present ( WORK )
#endif

      IERR = 0

      ! Accumulate flux out of physical domain.

      CALL MG_FLUX_ACCUMULATE ( IVAR, GRID, IERR )

#if defined _MG_OACC
!$acc parallel
!$acc loop
#endif

      DO K = 1, NZ
         DO J = 1, NY
            DO I = 1, NX
               WORK(I,J,K) =               ( GRID(I-1,J,K) +                      &
                             GRID(I,J-1,K) + GRID(I  ,J,K) + GRID(I,J+1,K) +      &
                                             GRID(I+1,J,K)                      ) &
                           / 5.0
            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end loop

!$acc loop
#endif

      DO K = 1, NZ
         DO J = 1, NY
            DO I = 1, NX
               GRID(I,J,K) = WORK(I,J,K)
            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end loop
!$acc end parallel
!$acc end data   ! WORK array.
#endif

      NUM_GRID_PTS = NX*NY*NZ
      MG_PERF%NUM_ADDS    = MG_PERF%NUM_ADDS  + 4*(NUM_GRID_PTS)
      MG_PERF%NUM_DIVIDES = MG_PERF%NUM_DIVIDES + NUM_GRID_PTS 
      MG_PERF%NUM_COPY    = MG_PERF%NUM_COPY + NUM_GRID_PTS

   END SUBROUTINE MG_STENCIL_2D5PT
   
   SUBROUTINE MG_STENCIL_2D9PT ( IVAR, GRID, IERR )
   
      ! ---------------------
      ! Argument Declarations
      ! ---------------------

      INTEGER, INTENT(IN) :: &
         IVAR

      INTEGER, INTENT(OUT) :: &
         IERR           ! Return status

      REAL(KIND=MG_REAL), DIMENSION(0:NX+1, 0:NY+1, 0:NZ+1), INTENT(INOUT) :: &
         GRID

      ! ------------------
      ! Local Declarations
      ! ------------------

      INTEGER :: &
         I, J, K,        &  ! Counters
         NUM_GRID_PTS

      ! ---------------------
      ! Executable Statements
      ! ---------------------
   
#if defined _MG_OACC
!$acc data present ( WORK )
#endif

      IERR = 0

#if defined _MG_OACC
!$acc parallel
!$acc loop
#endif

      DO K = 1, NZ 
         DO J = 1, NY
            DO I = 1, NX
               WORK(I,J,K) = ( GRID(I-1,J-1,K) + GRID(I-1,J,K) + GRID(I-1,J+1,K) +  &
                               GRID(I  ,J-1,K) + GRID(I  ,J,K) + GRID(I  ,J+1,K) +  &
                               GRID(I+1,J-1,K) + GRID(I+1,J,K) + GRID(I+1,J+1,K) )  &
                           / 9.0
            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end loop

!$acc loop
#endif

      DO K = 1, NZ
         DO J = 1, NY
            DO I = 1, NX
               GRID(I,J,K) = WORK(I,J,K)
            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end loop
!$acc end parallel
!$acc end data   ! WORK array.
#endif

      ! Accumulate flux out of physical domain.

      CALL MG_FLUX_ACCUMULATE ( IVAR, GRID, IERR )

      NUM_GRID_PTS = NX*NY*NZ
      MG_PERF%NUM_ADDS    = MG_PERF%NUM_ADDS  + 8*(NUM_GRID_PTS)
      MG_PERF%NUM_DIVIDES = MG_PERF%NUM_DIVIDES + NUM_GRID_PTS
      MG_PERF%NUM_COPY    = MG_PERF%NUM_COPY + NUM_GRID_PTS

   END SUBROUTINE MG_STENCIL_2D9PT
   
   SUBROUTINE MG_STENCIL_3D7PT ( IVAR, GRID, IERR )
   
      ! ---------------------
      ! Argument Declarations
      ! ---------------------

      INTEGER, INTENT(IN) :: &
         IVAR

      INTEGER, INTENT(OUT) :: &
         IERR           ! Return status

      REAL(KIND=MG_REAL), DIMENSION(0:NX+1, 0:NY+1, 0:NZ+1), INTENT(INOUT) :: &
         GRID
      ! ------------------
      ! Local Declarations
      ! ------------------

      INTEGER :: &
         I, J, K,        &  ! Counters
         NUM_GRID_PTS

      REAL(KIND=MG_REAL) :: &
         SLICE_MINE               ! Temporary

      ! ---------------------
      ! Executable Statements
      ! ---------------------
   
#if defined _MG_OACC
!$acc data present ( WORK )
#endif

      IERR = 0

#if defined _MG_OACC
!$acc parallel
!$acc loop
#endif

      DO K = 1, NZ 
         DO J = 1, NY
            DO I = 1, NX
               SLICE_MINE =                  GRID(I-1,J,K) +                 &
                             GRID(I,J-1,K) + GRID(I  ,J,K) + GRID(I,J+1,K)   &
                                           + GRID(I+1,J,K)

               WORK(I,J,K) = ( GRID(I,J,K-1)  + SLICE_MINE + GRID(I,J,K+1) ) / 7.0

            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end loop

!$acc loop
#endif

      DO K = 1, NZ
         DO J = 1, NY
            DO I = 1, NX
               GRID(I,J,K) = WORK(I,J,K)
            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end loop
!$acc end parallel
!$acc end data   ! WORK array.
#endif

      ! Accumulate flux out of physical domain.

      CALL MG_FLUX_ACCUMULATE ( IVAR, GRID, IERR )

      NUM_GRID_PTS = NX*NY*NZ
      MG_PERF%NUM_ADDS    = MG_PERF%NUM_ADDS  + 6*(NUM_GRID_PTS) 
      MG_PERF%NUM_DIVIDES = MG_PERF%NUM_DIVIDES + NUM_GRID_PTS 
      MG_PERF%NUM_COPY    = MG_PERF%NUM_COPY + NUM_GRID_PTS

   END SUBROUTINE MG_STENCIL_3D7PT

   SUBROUTINE MG_STENCIL_3D27PT ( IVAR, GRID, IERR )
   
      ! ---------------------
      ! Argument Declarations
      ! ---------------------

      INTEGER, INTENT(IN) :: &
         IVAR

      INTEGER, INTENT(OUT) :: &
         IERR           ! Return status

      REAL(KIND=MG_REAL), DIMENSION(0:NX+1, 0:NY+1, 0:NZ+1), INTENT(INOUT) :: &
         GRID
      ! ------------------
      ! Local Declarations
      ! ------------------

      INTEGER :: &
         I, J, K,        &  ! Counters
         NUM_GRID_PTS

      REAL(KIND=MG_REAL) :: &
         SLICE_BACK,          &   ! Temporary
         SLICE_MINE,          &   ! Temporary
         SLICE_FRONT              ! Temporary

      ! ---------------------
      ! Executable Statements
      ! ---------------------
   
#if defined _MG_OACC
!$acc data present ( WORK )
#endif

      IERR = 0

#if defined _MG_OACC
!$acc parallel
!$acc loop
#endif

      DO K = 1, NZ 
         DO J = 1, NY
            DO I = 1, NX

               SLICE_BACK =  GRID(I-1,J-1,K-1) + GRID(I-1,J,K-1) + GRID(I-1,J+1,K-1) + &
                             GRID(I  ,J-1,K-1) + GRID(I  ,J,K-1) + GRID(I  ,J+1,K-1) + &
                             GRID(I+1,J-1,K-1) + GRID(I+1,J,K-1) + GRID(I+1,J+1,K-1) 

               SLICE_MINE =  GRID(I-1,J-1,K)   + GRID(I-1,J,K)   + GRID(I-1,J+1,K) + &
                             GRID(I  ,J-1,K)   + GRID(I  ,J,K)   + GRID(I  ,J+1,K) + &
                             GRID(I+1,J-1,K)   + GRID(I+1,J,K)   + GRID(I+1,J+1,K)

               SLICE_FRONT = GRID(I-1,J-1,K+1) + GRID(I-1,J,K+1) + GRID(I-1,J+1,K+1) + &
                             GRID(I  ,J-1,K+1) + GRID(I  ,J,K+1) + GRID(I  ,J+1,K+1) + &
                             GRID(I+1,J-1,K+1) + GRID(I+1,J,K+1) + GRID(I+1,J+1,K+1)

               WORK(I,J,K) = ( SLICE_BACK + SLICE_MINE + SLICE_FRONT ) / 27.0

            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end loop

!$acc loop
#endif

      DO K = 1, NZ
         DO J = 1, NY
            DO I = 1, NX
               GRID(I,J,K) = WORK(I,J,K)
            END DO
         END DO
      END DO

#if defined _MG_OACC
!$acc end loop
!$acc end parallel
!$acc end data   ! WORK array.
#endif

      ! Accumulate flux out of physical domain.

      CALL MG_FLUX_ACCUMULATE ( IVAR, GRID, IERR )

      NUM_GRID_PTS = NX*NY*NZ
      MG_PERF%NUM_ADDS    = MG_PERF%NUM_ADDS  + 26*(NUM_GRID_PTS)  
      MG_PERF%NUM_DIVIDES = MG_PERF%NUM_DIVIDES + NUM_GRID_PTS
      MG_PERF%NUM_COPY    = MG_PERF%NUM_COPY + NUM_GRID_PTS

   END SUBROUTINE MG_STENCIL_3D27PT
   
END MODULE MG_STENCIL_COMPS_MOD
