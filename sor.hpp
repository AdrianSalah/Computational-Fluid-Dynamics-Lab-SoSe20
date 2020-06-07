#ifndef __SOR_H_
#define __SOR_H_

#include "datastructures.hpp"
#include "grid.hpp"
#include "boundary_val.hpp"

/**
 * One GS iteration for the pressure Poisson equation. Besides, the routine must 
 * also set the boundary values for P according to the specification. The 
 * residual for the termination criteria has to be stored in res.
 * 
 * An \omega = 1 GS - implementation is given within sor.c.
 */
/*void sor(
        double omg,
        double dx,
        double dy,
        int    imax,
        int    jmax,
        matrix &P,
        matrix &RS,
        double *res
);*/


void sor(
        double omg,
        double dx,
        double dy,
        int    imax,
        int    jmax,
        Grid &grid,
        matrix<double> &RS,
        double *res,
        int il,
        int ir,
        int jb,
        int jt,
        int my_rank
);


#endif
