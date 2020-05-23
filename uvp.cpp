#include "uvp.hpp"
#include "helper.hpp"
#include "datastructures.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cfloat>
#include <algorithm>

// Determines the values of F and G
void calculate_fg(
        double Re,
        double beta,
        double GX,
        double GY,
        double alpha,
        double dt,
        double dx,
        double dy,
        int imax,
        int jmax,
        Grid& grid,
        matrix<double> &F,
        matrix<double> &G,
        matrix<double> &T)
{
    static matrix<double> u;
    static matrix<double> v;

    grid.velocity(u, velocity_type::U);
    grid.velocity(v, velocity_type::V);

    //boundary values will be set in boundary_val function
/*
    // ----- Boundary values for F and G ----- //
    // F[0, j] = u[0, j]        j = 1...jmax    LEFT
    // F[imax, j] = u[imax, j]  j = 1...jmax    RIGHT
    //
    // G[i, 0] = v[i, 0]        i = 1...imax    BOTTOM
    // G[i, jmax] = v[i, jmax]  i = 1...imax    TOP

    for (int j = 1; j <= jmax; j++) {
        F[0][j] = u[0][j];
        F[imax][j] = u[imax][j];
    }

    for (int i = 1; i <= imax; i++) {
        G[i][0] = v[i][0];
        G[i][jmax] = v[i][jmax];
    }

 */

    // ----- F function initialisation ----- //

    static double d2_u_dx2;
    static double d2_u_dy2;
    static double d_u2_dx;
    static double d_uv_dy;

    // ------ Discretisation of differential operators of G ----- //

    for (int i = 1; i < imax; i++) 
    {
        for (int j = 1; j <= jmax; j++)
        {
            //second derivative with respect to x
            d2_u_dx2 = 1 / (dx * dx) * (u[i + 1][j] - 2 * u[i][j] + u[i - 1][j]);
            //second derivative with respect to y
            d2_u_dy2 = 1 / (dy * dy) * (u[i][j + 1] - 2 * u[i][j] + u[i][j - 1]);
            //first derivative of (u*u) with respect to x
            d_u2_dx = 1 / (4 * dx) * (
                    (u[i][j] + u[i + 1][j]) * (u[i][j] + u[i + 1][j]) -
                    (u[i - 1][j] + u[i][j]) * (u[i - 1][j] + u[i][j])
            ) +

                      alpha / (4 * dx) * (
                              abs(u[i][j] + u[i + 1][j]) * (u[i][j] - u[i + 1][j]) -
                              abs(u[i - 1][j] + u[i][j]) * (u[i - 1][j] - u[i][j])
                      );
            //first derivative of (u*v) with respect to y
            d_uv_dy = 1 / (4 * dy) * (
                    (v[i][j] + v[i + 1][j]) * (u[i][j] + u[i][j + 1]) -
                    (v[i][j - 1] + v[i + 1][j - 1]) * (u[i][j - 1] + u[i][j])
            ) +

                      alpha / (4 * dy) * (
                              abs(v[i][j] + v[i + 1][j]) * (u[i][j] - u[i][j + 1]) -
                              abs(v[i][j - 1] + v[i + 1][j - 1]) * (u[i][j - 1] - u[i][j])
                      );

            // To check whether GX should be divided by density
            F.at(i).at(j) = u[i][j] + dt * (1 / Re * (d2_u_dx2 + d2_u_dy2) - d_u2_dx - d_uv_dy + GX);

            F.at(i).at(j) -= beta * dt/2 * (T.at(i).at(j) + T.at(i).at(j)) * GX;
        }
    }


    // ----- G function initialisation ----- //

    static double d2_v_dx2;
    static double d2_v_dy2;
    static double d_v2_dy;
    static double d_uv_dx;

    // ------ Discretisation of differential operators of F ----- //

    for (int i = 1; i <= imax; i++)
    {
        for (int j = 1; j < jmax; j++)
        {
            //second derivative of v with respect to x
            d2_v_dx2 = 1 / (dx * dx) * (v[i + 1][j] - 2 * v[i][j] + v[i - 1][j]);
            //second derivative of v with respect to y
            d2_v_dy2 = 1 / (dy * dy) * (v[i][j + 1] - 2 * v[i][j] + v[i][j - 1]);
            //first derivative of (v*v) with respect to y
            d_v2_dy = 1 / (4 * dy) * (
                    (v[i][j] + v[i][j + 1]) * (v[i][j] + v[i][j + 1]) -
                    (v[i][j - 1] + v[i][j]) * (v[i][j - 1] + v[i][j])
            ) +

                      alpha / (4 * dy) * (
                              abs(v[i][j] + v[i][j + 1]) * (v[i][j] - v[i][j + 1]) -
                              abs(v[i][j - 1] + v[i][j]) * (v[i][j - 1] - v[i][j])
                      );
            //first derivative of (u*v) with respect to x
            d_uv_dx = 1 / (4 * dx) * (
                    (u[i][j] + u[i][j + 1]) * (v[i][j] + v[i + 1][j]) -
                    (u[i - 1][j] + u[i - 1][j + 1]) * (v[i - 1][j] + v[i][j])
            ) +

                      alpha / (4 * dx) * (
                              abs(u[i][j] + u[i][j + 1]) * (v[i][j] - v[i + 1][j]) -
                              abs(u[i - 1][j] + u[i - 1][j + 1]) * (v[i - 1][j] - v[i][j])
                      );

            // To check whether GY should be divided by density
            G.at(i).at(j) = v[i][j] + dt * (1 / Re * (d2_v_dx2 + d2_v_dy2) - d_uv_dx - d_v2_dy + GY);

            G.at(i).at(j) -= beta * dt/2 * (T.at(i).at(j) + T.at(i).at(j)) * GY;
        }
    }
}

void calculate_temp(
    double PR,
    double alpha,
    double dt,
    double dx,
    double dy,
    int imax,
    int jmax,
    Grid& grid,
    matrix<double>& T) {

    static matrix<double> u;
    static matrix<double> v;

    grid.velocity(u, velocity_type::U);
    grid.velocity(v, velocity_type::V);

    static double duT_dx;
    static double dvT_dy;
    static double d2_T_dx2;
    static double d2_T_dy2;

    // temperature initial values (to be filled later)
    for (int j = 1; j <= jmax; j++) {
        T[0][j] =0;
        T[imax][j] = 0;
    }

    for (int i = 1; i <= imax; i++) {
        T[i][0] =0;
        T[i][jmax] = 0;
    }

    for (int i = 1; i < imax; i++)
    {
        for (int j = 1; j <= jmax; j++)
        {
            duT_dx = (u[i][j] * (T[i][j] + T[i + 1][j]) - u[i - 1][j] * (T[i - 1][j] + T[i][j]))
                + (alpha * std::abs(u[i][j]) * (T[i][j] - T[i + 1][j])) - (alpha * std::abs(u[i - 1][j]) * (T[i - 1][j] - T[i][j]));
            duT_dx *= 0.5 / dx;

            dvT_dy = (v[i][j] * (T[i][j] + T[i][j + 1]) - v[i][j - 1] * (T[i][j - 1] + T[i][j]))
                + (alpha * std::abs(v[i][j]) * (T[i][j] - T[i][j + 1])) - (alpha * std::abs(v[i][j - 1]) * (T[i][j - 1] - T[i][j]));
            dvT_dy *= 0.5 / dy;

            d2_T_dx2 = (T[i + 1][j] - 2 * T[i][j] + T[i - 1][j]) / (dx * dx);

            d2_T_dy2 = (T[i][j + 1] - 2 * T[i][j] + T[i][j - 1]) / (dy * dy);

            // Explicit euler to solve the temperature equation

            T[i][j] = T[i][j] + dt * ((1 / PR) * (d2_T_dx2 - d2_T_dy2) - duT_dx - dvT_dy);
        }
    }


}

// This operation computes the right hand side of the pressure poisson equation.
void calculate_rs(
        double dt,
        double dx,
        double dy,
        int imax,
        int jmax,
        matrix<double> &F,
        matrix<double> &G,
        matrix<double> &RS)
{
    for (int i = 1; i <= imax; i++)
    {
        for (int j = 1; j <= jmax; j++)
        {
            RS.at(i).at(j) = 1 / dt * (
                    (F.at(i).at(j) - F.at(i - 1).at(j)) / dx +
                    (G.at(i).at(j) - G.at(i).at(j - 1)) / dy
            );
        }
    }
}



static bool abs_compare(int a, int b)
{
    return (std::abs(a) < std::abs(b));
}



double max_abs_velocity(int imax, int jmax, Grid& grid, velocity_type type) {
    static matrix<double> current_velocity; //matrix of current velocity U or V on grid
    grid.velocity(current_velocity, type); //assigns velocity U or V to current_velocity

    // Vector of maximum velocity values in every row (including boundaries, i.e. imaxb):
    static std::vector<double> max_abs_value_per_row(grid.imaxb(), 0);

    // Resetting the values to zeros
    std::fill(max_abs_value_per_row.begin(), max_abs_value_per_row.end(), 0);


    for (int i = 0; i < grid.imaxb(); ++i) {
        max_abs_value_per_row.at(i) = *std::max_element(current_velocity.at(i).begin(), current_velocity.at(i).end(),
                                                        abs_compare);
    }
    //maximum velocity value in grid
    return abs(*std::max_element(max_abs_value_per_row.begin(), max_abs_value_per_row.end(), abs_compare));
}



void calculate_dt(double Re,double PR, double tau, double* dt, double dx, double dy, int imax, int jmax, Grid& grid) {

    //maximum absolute values for U, V on grid for current time step
    static double max_abs_U;
    max_abs_U = max_abs_velocity(imax, jmax, grid, velocity_type::U);

    static double max_abs_V;
    max_abs_V = max_abs_velocity(imax, jmax, grid, velocity_type::V);

    //explicit time-steooing stability condition
    static double condition12;
    // PR=nu/alpha so Re*PR= alpha
    condition12 = 0.5 * std::min(PR, 1.0) * Re * (dx * dx) * (dy * dy) / ((dx * dx) + (dy * dy));


    if (max_abs_V < 1e-06 && max_abs_U < 1e-06)
        *dt = tau * condition12;
    else
        *dt = tau * std::min(condition12,
                             std::min((dx / max_abs_U), (dy / max_abs_V)));
}


void calculate_uv(
        double dt,
        double dx,
        double dy,
        int imax,
        int jmax,
        Grid& grid,
        matrix<double> &F,
        matrix<double> &G)
{
    static matrix<double> u_velocity;
    static matrix<double> v_velocity;
    static matrix<double> pressure;

    grid.velocity(u_velocity, velocity_type::U);
    grid.velocity(v_velocity, velocity_type::V);
    grid.pressure(pressure);


    for(int i = 1; i <= imax - 1; i++){
        for(int j = 1; j <= jmax; j++){
            u_velocity.at(i).at(j) = F.at(i).at(j) - dt/ dx * (pressure.at(i+1).at(j) - pressure.at(i).at(j));
        }
    }

    for (int i = 1; i <= imax; i++){
        for(int j = 1; j <= jmax - 1; j++){
            v_velocity.at(i).at(j) = G.at(i).at(j) - dt/ dy * (pressure.at(i).at(j+1) - pressure.at(i).at(j));
        }
    }

    grid.set_velocity(u_velocity, velocity_type::U);
    grid.set_velocity(v_velocity, velocity_type::V);

}

void init_fgrs(int imax,
            int jmax,
            matrix<double> &F,
            matrix<double> &G,
            matrix<double> &RS,
            double FI,
            double GI,
            double RSI
  ){
    F.resize(imax + 2);
    G.resize(imax + 2);
    RS.resize(imax + 2);

    for (int i = 0; i < imax + 2; i++) {

        F.at(i).resize(jmax + 2, FI);
        G.at(i).resize(jmax + 2, GI);
        RS.at(i).resize(jmax + 2, RSI);
    }
}