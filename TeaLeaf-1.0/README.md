# TeaLeaf ![image](Tea_alpha_small.png "TeaLeaf")

TeaLeaf is a mini-app that solves the linear heat conduction equation on a
spatially decomposed regularly grid using a 5 point stencil with implicit
solvers. TeaLeaf currently solves the equations in two dimensions, but three
dimensional support is in beta.

In TeaLeaf temperatures are stored at the cell centres. A conduction
coefficient is calculated that is equal to the cell centred density or the
reciprocal of the density. This is then averaged to each face of the cell for
use in the solution. The solve is carried out using an implicit method due to
the severe timestep limitations imposed by the stability criteria of an
explicit solution for a parabolic partial differential equation. The implicit
method requires the solution of a system of linear equations which form a
regular sparse matrix with a well defined structure.

The computation in TeaLeaf has been broken down into "kernels", low level
building blocks with minimal complexity. Each kernel loops over the entire grid
and updates the relevant mesh variables. Control logic within each kernel is
kept to a minimum, allowing maximum optimisation by the compiler. Memory is
sacrificed in order to increase performance, and any updates to variables that
would introduce dependencies between loop iterations are written into copies of
the mesh.

Normally, third party solvers are used to invert the system of linear
equations, because of the complexity of state of the art methods. For reference
the simplest iterative method, the Jacobi method, has been included in the
reference version as the default as well the option to use a Conjugate Gradient or a Chebyshev solver.
These does have the advantage of being matrix free and independent of library dependencies.
By deault a pre-conditioner is not invoked, but a simple pre-conditioner is available as an
option which just uses diagonal scaling. Note that this simple method will not always speed up the solve time.

The solvers have been written in Fortran with OpenMP and MPI and they have also
been ported to OpenCL to provide an accelerated capability.

Other versions invoke third party linear solvers and currently include
Petsc, Trilinos and Hypre, which are in beta release. For each of these version there are instructions on
how to download, build and link in the relevant library.

## Current Implementations:

- Fortran with OpenMP and MPI
- OpenCL
- Petsc
- Trilinos
- Hypre

## Coming Soon:

- CUDA and AMGX

## TeaLeaf Build Procedure

Dependencies in TeaLeaf have been kept to a minimum to ease the build 
process across multiple platforms and environments. There is a single makefile 
for all compilers and adding a new compiler should be straight forward.

In many case just typing `make` in the required software directory will work. 
This is the case if the mpif90 and mpicc wrappers are available on the system. 

If the MPI compilers have different names then the build process needs to 
notified of this by defining two environment variables, `MPI_COMPILER` and 
`C_MPI_COMPILER`. 

For example on some Intel systems:

`make MPI_COMPILER=mpiifort C_MPI_COMPILER=mpiicc`

Or on Cray systems:

`make MPI_COMPILER=ftn C_MPI_COMPILER=cc`

### OpenMP Build

All compilers use different arguments to invoke OpenMP compilation. A simple 
call to make will invoke the compiler with -O3. This does not usually include 
OpenMP by default. To build for OpenMP for a specific compiler a further 
variable must be defined, `COMPILER` that will then select the correct option 
for OpenMP compilation. 

For example with the Intel compiler:

`make COMPILER=INTEL`

Which then append the -openmp to the build flags.

Other supported compiler that will be recognised are:-

* CRAY
* SUN
* GNU
* XL
* PATHSCALE
* PGI

The default flags for each of these is show below:-

* INTEL: -O3 -ipo
* SUN: -fast
* GNU: -ipo
* XL: -O5
* PATHSCLE: -O3
* PGI: -O3 -Minline
* CRAY: -em  _Note: that by default the Cray compiler with pick the optimum 
options for performance._

### Other Flags

The default compilation with the COMPILER flag set chooses the optimal 
performing set of flags for the specified compiler, but with no hardware 
specific options or IEEE compatability.

To produce a version that has IEEE compatiblity a further flag has to be set on 
the compiler line.

`make COMPILER=INTEL IEEE=1`

This flag has no effect if the compiler flag is not set because IEEE options 
are always compiler specific.

For each compiler the flags associated with IEEE are shown below:-

* INTEL: -fp-model strict –fp-model source –prec-div –prec-sqrt
* CRAY: -hpflex_mp=intolerant
* SUN: -fsimple=0 –fns=no
* GNU: -ffloat-store
* PGI: -Kieee
* PATHSCALE: -mieee-fp
* XL: -qstrict –qfloat=nomaf

Note that the MPI communications have been written to ensure bitwise identical 
answers independent of core count. However under some compilers this is not 
true unless the IEEE flags is set to be true. This is certainly true of the 
Intel and Cray compiler. Even with the IEEE options set, this is not guarantee 
that different compilers or platforms will produce the same answers. Indeed a 
Fortran run can give different answers from a C run with the same compiler, 
same options and same hardware.

Extra options can be added without modifying the makefile by adding two further 
flags, `OPTIONS` and `C_OPTIONS`, one for the Fortran and one for the C options.

`make COMPILER=INTEL OPTIONS=-xavx C_OPTIONS=-xavx`

A build for a Xeon Phi would just need the -xavx option above replaced by -mmic.

Finally, a `DEBUG` flag can be set to use debug options for a specific compiler.

`make COMPILER=PGI DEBUG=1`

These flags are also compiler specific, and so will depend on the `COMPILER` 
environment variable.

So on a system without the standard MPI wrappers, for a build that requires 
OpenMP, IEEE and AVX this would look like so:-

```
make COMPILER=INTEL MPI_COMPILER=mpiifort C_MPI_COMPILER=mpiicc IEEE=1 \
OPTIONS="-xavx" C_OPTIONS="-xavx"
```

## Running the Code

TeaLeaf takes no command line arguments. It expects to find a file called 
`tea.in` in the directory it is running in.

There are a number of input files that come with the code. To use any of these 
they simply need to be copied to `tea.in` in the run directory and 
TeaLeaf invoked. The invocation is system dependent. 

For example for a hybrid run:

```
export OMP_NUM_THREADS=4

mpirun -np 8 tea_leaf
```

### Weak and Strong Scaling

Note that with strong scaling, as the task count increases for the same size 
global problem, the memory use of each task decreases. Eventually, the mesh 
data starts to fit into the various levels of cache. So even though the 
communications overhead is increasing, super-scalar leaps in performance can be 
seen as task count increases. Eventually all cache benefits are gained and the 
communications dominate. 

For weak scaling, memory use stays close to constant and these super-scalar 
increases aren't seen but the communications overhead stays constant relative 
to the computational overhead, and scaling remains good.

### Other Issues to Consider

System libraries and settings can also have a significant effect on performance. 

The use of the `HugePage` library can make memory access more efficient. The 
implementation of this is very system specific and the details will not be 
expanded here. 

Variation in clock speed, such as `SpeedStep` or `Turbo Boost`, is also 
available on some hardware and care needs to be taken that the settings are 
known. 

Many systems also allow some level of hyperthreading at a core level. These 
usually share floating point units and for a code like TeaLeaf, which is 
floating point intensive and light on integer operations, are unlikely to 
produce a benefit and more likely to reduce performance.

TeaLeaf is considered a memory bound code. Most data does not stay in cache 
very long before it is replaced. For this reason, memory speed can have a 
significant effect on performance. For the same reason, the same is true of 
hardware caches.

## Testing the Results

Even though bitwise answers cannot be expected across systems, answers should be 
very close, though the iterative nature of the solves and the dependence on reduction in the MPI
can affect this. A summary print of state variables is printed out by default every 
ten diffusion steps and then at the end of the run. This print gives average 
value of the volume, mass, density, internal energy and temperature. Only temperature shoul vary with step count
because all boundaries are zero flux and there is no material motion.
If mass, volume or enegry do not stay constant through a run, then 
something is seriously wrong.

There is a very simple, small self test include in the TeaLeaf. If the code is
invoked no tean.in input file present, this test will be run and the answer tested
against a "known" solution.

There are four standard input files that are recommended for testing. 
Initially it is suggested than `tea_bm_short.in` is run. This will self test.
It is quick to run, even on a single core, and should stop after 87 steps.

The second test to try is `tea_bm.in`. This runs for 1000 timesteps and is 
more sensitive than the first test. Through this simulation the whole 
computational mesh in traversed by the heat front and so it is a good test of the 
parallel implementation because all internal boundaries will be crossed during 
the course of the simulation. This is also self testing.

The third test to try is `tea_bm16_short.in`. This is the "socket" test and 
has a much larger mesh size and therefore, memory footprint. It will self test.

The last test to run for validation purposes is `tea_bm16.in`. This is a 
fairly long, large mesh run and will self test.
