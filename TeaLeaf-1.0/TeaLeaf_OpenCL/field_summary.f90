!Crown Copyright 2014 AWE.
!
! This file is part of TeaLeaf.
!
! TeaLeaf is free software: you can redistribute it and/or modify it under 
! the terms of the GNU General Public License as published by the 
! Free Software Foundation, either version 3 of the License, or (at your option) 
! any later version.
!
! TeaLeaf is distributed in the hope that it will be useful, but 
! WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
! FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
! details.
!
! You should have received a copy of the GNU General Public License along with 
! TeaLeaf. If not, see http://www.gnu.org/licenses/.

!>  @brief Driver for the field summary kernels
!>  @author David Beckingsale, Wayne Gaudin
!>  @details The user specified field summary kernel is invoked here. A summation
!>  across all mesh chunks is then performed and the information outputed.
!>  If the run is a test problem, the final result is compared with the expected
!>  result and the difference output.
!>  Note the reference solution is the value returned from an Intel compiler with
!>  ieee options set on a single core run.

SUBROUTINE field_summary()

  USE tea_module
  USE field_summary_kernel_module

  IMPLICIT NONE

  REAL(KIND=8) :: vol,mass,ie,temp
  REAL(KIND=8) :: qa_diff

!$ INTEGER :: OMP_GET_THREAD_NUM

  INTEGER      :: c

  REAL(KIND=8) :: kernel_time,timer

  IF(parallel%boss)THEN
    WRITE(g_out,*)
    WRITE(g_out,*) 'Time ',time
    WRITE(g_out,'(a13,5a16)')'           ','Volume','Mass','Density'       &
                                          ,'Energy','U'
  ENDIF

  IF(profiler_on) kernel_time=timer()
  IF(use_fortran_kernels)THEN
    DO c=1,chunks_per_task
      IF(chunks(c)%task.EQ.parallel%task) THEN
        CALL field_summary_kernel(chunks(c)%field%x_min,                   &
                                  chunks(c)%field%x_max,                   &
                                  chunks(c)%field%y_min,                   &
                                  chunks(c)%field%y_max,                   &
                                  chunks(c)%field%volume,                  &
                                  chunks(c)%field%density,                 &
                                  chunks(c)%field%energy0,                 &
                                  chunks(c)%field%u,                       &
                                  vol,mass,ie,temp                         )
      ENDIF
    ENDDO
  ELSEIF(use_opencl_kernels)THEN
    DO c=1,chunks_per_task
      IF(chunks(c)%task.EQ.parallel%task) THEN
        CALL field_summary_kernel_ocl(vol,mass,ie,temp)
      ENDIF
    ENDDO
  ENDIF

  ! For mpi I need a reduction here
  CALL tea_sum(vol)
  CALL tea_sum(mass)
  CALL tea_sum(ie)
  CALL tea_sum(temp)
  IF(profiler_on) profiler%summary=profiler%summary+(timer()-kernel_time)

  IF(parallel%boss) THEN
!$  IF(OMP_GET_THREAD_NUM().EQ.0) THEN
      WRITE(g_out,'(a6,i7,5e16.7)')' step:',step,vol,mass,mass/vol,ie,temp
      WRITE(g_out,*)
!$  ENDIF
  ENDIF

  !Check if this is the final call and if it is a test problem, check the result.
  IF(complete) THEN
    IF(parallel%boss) THEN
!$    IF(OMP_GET_THREAD_NUM().EQ.0) THEN
        IF(test_problem.GE.1) THEN
  ! Note that the "correct" solution is with IEEE switched on, 1 task, 1 thread, Intel compiler on Ivy Bridge
          IF(test_problem.EQ.1) qa_diff=ABS((100.0_8*(temp/157.550841832793_8))-100.0_8)
          IF(test_problem.EQ.2) qa_diff=ABS((100.0_8*(temp/116.067951160930_8))-100.0_8)
          IF(test_problem.EQ.3) qa_diff=ABS((100.0_8*(temp/95.4865103390698_8))-100.0_8)
          IF(test_problem.EQ.4) qa_diff=ABS((100.0_8*(temp/166.838315378708_8))-100.0_8)
          IF(test_problem.EQ.5) qa_diff=ABS((100.0_8*(temp/116.482111627676_8))-100.0_8)

          WRITE(*,'(a,i4,a,e16.7,a)')"Test problem", Test_problem," is within",qa_diff,"% of the expected solution"
          WRITE(g_out,'(a,i4,a,e16.7,a)')"Test problem", Test_problem," is within",qa_diff,"% of the expected solution"
          IF(qa_diff.LT.0.001) THEN
            WRITE(*,*)"This test is considered PASSED"
            WRITE(g_out,*)"This test is considered PASSED"
          ELSE
            WRITE(*,*)"This test is considered NOT PASSED"
            WRITE(g_out,*)"This is test is considered NOT PASSED"
          ENDIF
        ENDIF
!$    ENDIF
    ENDIF
  ENDIF

END SUBROUTINE field_summary
