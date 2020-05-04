#include "uvp.hpp"
#include "helper.hpp"
#include "datastructures.hpp"
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cfloat>

//my includes
#include <algorithm>
#include <iostream>

// Determines the value of F and G
void calculate_fg(
        double Re,
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
        matrix<double> &G)
{
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
}

//used in max_abs_velocity function to get index of absolute maximum value
static bool abs_compare(int a, int b)
{
    return (std::abs(a) < std::abs(b));
}


// Changed the function signature: removed imax and jmax - they are already contained in Grid
// and can be acessed with imax() and jmax() methods. And jmax value is not used here at all.
// Used *pointer to access the abs_max_value directly
// Avoided creating additional vectors in the loop to improve performance and make code more readable
// [Oleg]

double max_abs_velocity(Grid& grid, velocity_type type) {
    matrix<double> current_velocity; //matrix of current velocity U or V on grid
    grid.velocity(current_velocity, type); //assigns velocity U or V to current_velocity
    
    // Vector of maximum velocity values in every row (including boundaries, i.e. imaxb):
    std::vector<double> max_abs_value_per_row(grid.imaxb(), 0);

    for (int i = 0; i < grid.imaxb(); ++i)
        max_abs_value_per_row.at(i) = *std::max_element(current_velocity.at(i).begin(), current_velocity.at(i).end(), abs_compare);

    return *std::max_element(max_abs_value_per_row.begin(), max_abs_value_per_row.end());
}


double max_abs_velocity(int imax, int jmax, Grid &grid, velocity_type type){
    matrix<double> current_velocity; //matrix of current velocity U or V on grid
    grid.velocity(current_velocity, type); //assigns velocity U or V to current_velocity

    std::vector<double> max_values_columns; //vector of maximum velocity values in every column
    int j_abs_max_value; //j index of maximum absolute velocity value

    for(int i = 0; i < imax+2; i++){
        std::vector<double> current_column = current_velocity.at(i);
        j_abs_max_value = std::distance(current_column.begin(), std::max_element(current_column.begin(), current_column.end(), abs_compare));
        max_values_columns.push_back(abs(current_column.at(j_abs_max_value)));
    }
    //index of maximum absolute value in max_values_columns
    int max_index = std::distance(max_values_columns.begin(), std::max_element(max_values_columns.begin(), max_values_columns.end()));

    return max_values_columns.at(max_index);

}

// Determines the maximal time step size
void calculate_dt(double Re, double tau, double *dt, double dx, double dy, int imax, int jmax, Grid &grid) {

    //maximum absolute values for U, V on grid for current time step
    double max_abs_U = max_abs_velocity(imax, jmax, grid, velocity_type::U);
    double max_abs_V = max_abs_velocity(imax, jmax, grid, velocity_type::V);

    //first stability conditon

    double condition1 = 0.5 * Re * (dx * dx) * (dy * dy) / ((dx * dx) + (dy * dy));  


    //check if CFL stability conditions are too small, then just use first stability condition
    //I am not sure if that's the best way to do it
    
    // Decreased it a bit and replace logical OR operator from "or" to "||" to be consisntent
    // with the project code, and avoid the issue I have with my configuration (it shows an error with "or")
    // [Oleg]
    
    if (max_abs_V < 1e-06 || max_abs_U < 1e-06) {
        *dt = condition1;
    }

    else {

    //CFL stability conditions
    double CFL1 = dx / max_abs_U;
    double CFL2 = dy / max_abs_V;

    double min_condition = std::min(condition1, std::min(CFL1, CFL2));

    *dt = tau * min_condition;
    }
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
}