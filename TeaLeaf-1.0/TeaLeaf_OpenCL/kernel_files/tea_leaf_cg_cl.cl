#include <kernel_files/macros_cl.cl>

/*
 *  Kernels used for conjugate gradient method
 */

#define CONDUCTIVITY 1
#define RECIP_CONDUCTIVITY 2

__kernel void tea_leaf_cg_init_u
(__global const double * __restrict const density1,
 __global const double * __restrict const energy1,
 __global       double * __restrict const u,
 __global       double * __restrict const p,
 __global       double * __restrict const r,
 __global       double * __restrict const d,
 const int coefficient)
{
    __kernel_indexes;

    if (/*row >= (y_min + 1) - 2 &&*/ row <= (y_max + 1) + 2
    && /*column >= (x_min + 1) - 2 &&*/ column <= (x_max + 1) + 2)
    {
        p[THARR2D(0, 0, 0)] = 0.0;
        r[THARR2D(0, 0, 0)] = 0.0;

        u[THARR2D(0, 0, 0)] = energy1[THARR2D(0, 0, 0)]*density1[THARR2D(0, 0, 0)];

        if (CONDUCTIVITY == coefficient)
        {
            d[THARR2D(0, 0, 0)] = density1[THARR2D(0, 0, 0)];
        }
        else
        {
            d[THARR2D(0, 0, 0)] = 1.0/density1[THARR2D(0, 0, 0)];
        }
    }
}

__kernel void tea_leaf_cg_init_directions
(__global const double * __restrict const d,
 __global       double * __restrict const Kx,
 __global       double * __restrict const Ky)
{
    __kernel_indexes;

    if (/*row >= (y_min + 1) - 0 &&*/ row <= (y_max + 1) + 1
    && /*column >= (x_min + 1) - 0 &&*/ column <= (x_max + 1) + 1)
    {
        Kx[THARR2D(0, 0, 0)] = (d[THARR2D(-1, 0, 0)] + d[THARR2D(0, 0, 0)])
            /(2.0*d[THARR2D(-1, 0, 0)]*d[THARR2D(0, 0, 0)]);
        Ky[THARR2D(0, 0, 0)] = (d[THARR2D(0, -1, 0)] + d[THARR2D(0, 0, 0)])
            /(2.0*d[THARR2D(0, -1, 0)]*d[THARR2D(0, 0, 0)]);
    }
}

__kernel void tea_leaf_cg_init_others
(__global       double * __restrict const rro,
 __global const double * __restrict const u,
 __global       double * __restrict const p,
 __global       double * __restrict const r,
 __global       double * __restrict const w,
 __global       double * __restrict const Mi,
 __global const double * __restrict const Kx,
 __global const double * __restrict const Ky,
 double rx, double ry,
 __global       double * __restrict const z)
{
    __kernel_indexes;

    __local double rro_shared[BLOCK_SZ];
    rro_shared[lid] = 0.0;

    // used to make ifdefs for reductions less messy
    double rro_val;

    if (/*row >= (y_min + 1) - 0 &&*/ row <= (y_max + 1) + 0
    && /*column >= (x_min + 1) - 0 &&*/ column <= (x_max + 1) + 0)
    {
        w[THARR2D(0, 0, 0)] = (1.0
            + (Ky[THARR2D(0, 1, 0)] + Ky[THARR2D(0, 0, 0)])
            + (Kx[THARR2D(1, 0, 0)] + Kx[THARR2D(0, 0, 0)]))*u[THARR2D(0, 0, 0)]
            - (Ky[THARR2D(0, 1, 0)]*u[THARR2D(0, 1, 0)] + Ky[THARR2D(0, 0, 0)]*u[THARR2D(0, -1, 0)])
            - (Kx[THARR2D(1, 0, 0)]*u[THARR2D(1, 0, 0)] + Kx[THARR2D(0, 0, 0)]*u[THARR2D(-1, 0, 0)]);

        r[THARR2D(0, 0, 0)] = u[THARR2D(0, 0, 0)] - w[THARR2D(0, 0, 0)];

#if defined(USE_PRECONDITIONER)
        Mi[THARR2D(0, 0, 0)] = (1.0
            + (Ky[THARR2D(0, 1, 0)] + Ky[THARR2D(0, 0, 0)])
            + (Kx[THARR2D(1, 0, 0)] + Kx[THARR2D(0, 0, 0)]));
        Mi[THARR2D(0, 0, 0)] = 1.0/Mi[THARR2D(0, 0, 0)];

        z[THARR2D(0, 0, 0)] = Mi[THARR2D(0, 0, 0)]*r[THARR2D(0, 0, 0)];
        p[THARR2D(0, 0, 0)] = z[THARR2D(0, 0, 0)];
        rro_val = r[THARR2D(0, 0, 0)]*z[THARR2D(0, 0, 0)];
#else
        p[THARR2D(0, 0, 0)] = r[THARR2D(0, 0, 0)];
        rro_val = r[THARR2D(0, 0, 0)]*r[THARR2D(0, 0, 0)];
#endif

        rro_shared[lid] = rro_val;
    }

    REDUCTION(rro_shared, rro, SUM)
}

/* reduce rro */

__kernel void tea_leaf_cg_solve_calc_w
(__global       double * __restrict const pw,
 __global const double * __restrict const p,
 __global       double * __restrict const w,
 __global const double * __restrict const Kx,
 __global const double * __restrict const Ky,
 double rx, double ry)
{
    __kernel_indexes;

    __local double pw_shared[BLOCK_SZ];
    pw_shared[lid] = 0.0;

    if (/*row >= (y_min + 1) - 0 &&*/ row <= (y_max + 1) + 0
    && /*column >= (x_min + 1) - 0 &&*/ column <= (x_max + 1) + 0)
    {
        w[THARR2D(0, 0, 0)] = (1.0
            + (Ky[THARR2D(0, 1, 0)] + Ky[THARR2D(0, 0, 0)])
            + (Kx[THARR2D(1, 0, 0)] + Kx[THARR2D(0, 0, 0)]))*p[THARR2D(0, 0, 0)]
            - (Ky[THARR2D(0, 1, 0)]*p[THARR2D(0, 1, 0)] + Ky[THARR2D(0, 0, 0)]*p[THARR2D(0, -1, 0)])
            - (Kx[THARR2D(1, 0, 0)]*p[THARR2D(1, 0, 0)] + Kx[THARR2D(0, 0, 0)]*p[THARR2D(-1, 0, 0)]);
        
        pw_shared[lid] = p[THARR2D(0, 0, 0)]*w[THARR2D(0, 0, 0)];
    }

    REDUCTION(pw_shared, pw, SUM);
}

/* reduce pw */

__kernel void tea_leaf_cg_solve_calc_ur
(double alpha,
 __global       double * __restrict const rrn,
 __global       double * __restrict const u,
 __global const double * __restrict const p,
 __global       double * __restrict const r,
 __global const double * __restrict const w,
 __global       double * __restrict const z,
 __global const double * __restrict const Mi)
{
    __kernel_indexes;

    __local double rrn_shared[BLOCK_SZ];
    rrn_shared[lid] = 0.0;

    // as above
    double rrn_val;

    if (/*row >= (y_min + 1) - 0 &&*/ row <= (y_max + 1) + 0
    && /*column >= (x_min + 1) - 0 &&*/ column <= (x_max + 1) + 0)
    {
        u[THARR2D(0, 0, 0)] += alpha*p[THARR2D(0, 0, 0)];
        r[THARR2D(0, 0, 0)] -= alpha*w[THARR2D(0, 0, 0)];
#if defined(USE_PRECONDITIONER)
        z[THARR2D(0, 0, 0)] = Mi[THARR2D(0, 0, 0)]*r[THARR2D(0, 0, 0)];
        rrn_val = r[THARR2D(0, 0, 0)]*z[THARR2D(0, 0, 0)];
#else
        rrn_val = r[THARR2D(0, 0, 0)]*r[THARR2D(0, 0, 0)];
#endif

        rrn_shared[lid] = rrn_val;
    }

    REDUCTION(rrn_shared, rrn, SUM);
}

/* reduce rrn */

__kernel void tea_leaf_cg_solve_calc_p
(double beta,
 __global       double * __restrict const p,
 __global const double * __restrict const r,
 __global const double * __restrict const z)
{
    __kernel_indexes;

    if (/*row >= (y_min + 1) - 0 &&*/ row <= (y_max + 1) + 0
    && /*column >= (x_min + 1) - 0 &&*/ column <= (x_max + 1) + 0)
    {
#if defined(USE_PRECONDITIONER)
        p[THARR2D(0, 0, 0)] = z[THARR2D(0, 0, 0)] + beta*p[THARR2D(0, 0, 0)];
#else
        p[THARR2D(0, 0, 0)] = r[THARR2D(0, 0, 0)] + beta*p[THARR2D(0, 0, 0)];
#endif
    }
}

