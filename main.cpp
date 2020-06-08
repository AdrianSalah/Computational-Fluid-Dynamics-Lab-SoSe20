#include "helper.hpp"
#include "visual.hpp"
#include "init.hpp"
#include "sor.hpp"
#include "uvp.hpp"
#include <cstdio>
#include <iostream>
#include <sstream>
#include "boundary_val.hpp"
#include "Timer.h"
#include "parallel.hpp"
#include <mpi.h>

#define BOUNDARY_SIZE 1


// NOT USED in ws3: specify Scenario with input from console
/*
int scenarioSpec;
std::string SCENARIO_NAME;
std::string SCENARIO_DAT_FILE;
std::string SCENARIO_PGM_FILE;
*/


// USED in ws3: specify Scenario using macros
#define SCENARIO_NAME "lid_driven_cavity"
#define SCENARIO_DAT_FILE "../parameters/lid_driven_cavity.dat"
#define SCENARIO_PGM_FILE "../geometry/lid_driven_cavity.pgm"

int main(int argn, char** args) {

    MPI_Init(&argn, &args);

    int num_proc;

    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    int myrank;

    int chunk;

    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);




    // NOT USED in ws3: specify Scenario with input from console
    /*
    if (argn == 1)
        scenarioSpec = 1;

    else if (argn == 2) {

        scenarioSpec = atoi(args[1]);
    }
    else {std::cout << "more arguments given" << std::endl;
        exit(EXIT_FAILURE);}
    switch(scenarioSpec)
    {
        case 1:
            printf("lid driven cavity \n");
            SCENARIO_NAME = "lid_driven_cavity";
            SCENARIO_DAT_FILE = "../parameters/lid_driven_cavity.dat";
            SCENARIO_PGM_FILE = "../geometry/lid_driven_cavity.pgm";
            break;

        case 2:
            printf("plane shear \n");
            SCENARIO_NAME = "plane_shear";
            SCENARIO_DAT_FILE = "../parameters/plane_shear.dat";
            SCENARIO_PGM_FILE = "../geometry/plane_shear.pgm";
            break;

        case 3:
            printf("karman vortex street \n");
            SCENARIO_NAME = "karman_vortex_street";
            SCENARIO_DAT_FILE = "../parameters/karman_vortex_street.dat";
            SCENARIO_PGM_FILE = "../geometry/karman_vortex_street.pgm";
            break;

        case 4:
            printf("flow over step \n");
            SCENARIO_NAME = "flow_over_step";
            SCENARIO_DAT_FILE = "../parameters/flow_over_step.dat";
            SCENARIO_PGM_FILE = "../geometry/flow_over_step.pgm";
            break;

        case 5:
            printf("natural convection \n");
            SCENARIO_NAME = "natural_convection";
            SCENARIO_DAT_FILE = "../parameters/natural_convection.dat";
            SCENARIO_PGM_FILE = "../geometry/natural_convection.pgm";
            break;

        case 6:
            printf("fluid trap \n");
            SCENARIO_NAME = "fluid_trap";
            SCENARIO_DAT_FILE = "../parameters/fluid_trap.dat";
            SCENARIO_PGM_FILE = "../geometry/fluid_trap.pgm";
            break;

        case 7:
            printf("rayleigh benard convection \n");
            SCENARIO_NAME = "rayleigh_benard_convection";
            SCENARIO_DAT_FILE = "../parameters/rayleigh_benard_convection.dat";
            SCENARIO_PGM_FILE = "../geometry/rayleigh_benard_convection.pgm";
            break;

        default:
            printf("Couldn't select any secnario \n");
            exit(EXIT_FAILURE);

    }
     */



    //initialize all relevant parameters
    double* Re = new double;                /* reynolds number   */
    double* UI = new double;                /* velocity x-direction */
    double* VI = new double;                /* velocity y-direction */
    double* PI = new double;                /* pressure */
    double* GX = new double;                /* gravitation x-direction */
    double* GY = new double;                /* gravitation y-direction */
    double* t_end = new double;             /* end time */
    double* xlength = new double;           /* length of the domain x-dir.*/
    double* ylength = new double;           /* length of the domain y-dir.*/
    double* dt = new double;                /* time step */
    double* dx = new double;                /* length of a cell x-dir. */
    double* dy = new double;                /* length of a cell y-dir. */
    int* imax = new int;                    /* number of cells x-direction*/
    int* jmax = new int;                    /* number of cells y-direction*/
    double* alpha = new double;             /* uppwind differencing factor*/
    double* omg = new double;               /* relaxation factor */
    double* tau = new double;               /* safety factor for time step*/
    int* itermax = new int;                 /* max. number of iterations  */
    double* eps = new double;               /* accuracy bound for pressure*/
    double* dt_value = new double;          /* time for output */
    double* TI = new double;                /* Initial Temperature*/
    double* T_h = new double;               /* Temperature of hot wall*/
    double* T_c = new double;               /* Temperature of cold wall*/
    double* Pr = new double;                /* Prandlt Number*/
    double* res = new double;               /* residual for SOR*/
    double* beta= new double;               /* beta for fg calculation*/
    double* v_inflow = new double;          /* boundary value for inflow BC */
    double* u_inflow = new double;          /* boundary value for inflow BC */
    double* kappa = new double;             /* thermal conductivity */
    double* heat_flux = new double;         /* heat flux */
    int **cell_array = new int *;           /* array of geometry */
    int* iproc = new int;
    int* jproc = new int;
    int *il = new int;
    int *ir = new int;
    int *jb = new int;
    int *jt = new int;
    int *rank_l = new int;
    int *rank_r = new int;
    int *rank_b = new int;
    int *rank_t = new int;
    int *omg_i = new int;
    int *omg_j = new int;
    double* bufSend = new double[50];
    double* bufRecv = new double[50];





    //check if directory "output" exists, if not creates directory "output"
    check_dir_exists(SCENARIO_NAME);

    FILE *parameterFile;
    FILE *geometryFile;

    const char *input_parameter_file_path = SCENARIO_DAT_FILE; //.c_str(); ---> used when we specify with input from console
    const char *input_geometry_file_path = SCENARIO_PGM_FILE; //.c_str(); ---> used when we specify with input from console

    parameterFile = fopen(input_parameter_file_path, "r");
    geometryFile = fopen(input_parameter_file_path, "r");

    //check whether plane_shear.dat exists or not
    if (parameterFile == NULL) {
        printf("Error opening parameter-file");
        exit(EXIT_FAILURE);
    } 
    else if (geometryFile == NULL) {
        printf("Error opening geometry-file");
        exit(EXIT_FAILURE);
    }
    else {
        std::string parameterFile{input_parameter_file_path}; //relative path to plane_shear.dat file
        //ready parameters from plane_shear.dat file and assign values to initalized parameters
        read_parameters(parameterFile, Re, UI, VI, PI, GX, GY, t_end, xlength, ylength, dt, dx, dy, imax, jmax, alpha, omg,
                        tau, itermax, eps, dt_value, TI, T_h, T_c, Pr, beta, v_inflow, u_inflow, kappa, heat_flux, iproc, jproc);
    }
    // asserting iproc and jproc values

    if ((*iproc) * (*jproc) > num_proc) {
        std::cout << "Too many subdomains. ";
        *jproc = std::floor(std::sqrt(num_proc *(*jmax)/(*imax)));
        *iproc = num_proc / (*jproc);
        std::cout << "new iproc = " << * iproc << "  new jproc = " << *jproc << std::endl;
    }

 
    cell_array = read_pgm(input_geometry_file_path);


    // Set up grid
    Grid grid(*imax, *jmax, BOUNDARY_SIZE, *PI, *UI, *VI, *TI);

    if (*iproc == 1 and *jproc == 1) {
        *il = 0;
        *ir = grid.imaxb() - 1;
        *jb = 0;
        *jt= grid.jmaxb() - 1;
    }

    // NOT USED in ws3!
    /*
    if (!assert_problem_solvability(cell_array, grid)) {
        printf("PGM file is not solvable");
        exit(EXIT_FAILURE);
    }
     */


    init_parallel(*iproc, *jproc, *imax, *jmax, &myrank, il, ir, jb, jt, rank_l, rank_r, rank_b, rank_t, omg_i, omg_j, num_proc);
    //std::cout << "myrank: " << myrank << std::endl;

    //std::cout << *rank_l << " " << *rank_r << " " << *rank_b << " " << *rank_t << std::endl;
    //for debugging
    //std::cout << "myrank: " << myrank << ", omg_i: " << *omg_i << ", omg_j: " << *omg_j << std::endl;
    std::cout << "myrank: " << myrank << ", il: " << *il << ", ir: " << *ir << std::endl;
    //std::cout << "myrank: " << myrank << ", jb: " << *jb << ", jt: " << *jt << std::endl;

 
    //for output to vtk-file
    VTKHelper vtkOutput;


    // NOT USED in ws3!
    //TO DO: check wheather imax and jmax same as grid size in geometry file
    /*
    for (int j = grid.jmaxb() - 1; j >= 0; j--){
        for (int i = 0; i < grid.imaxb(); i++){
            //assign cell type
            if (cell_array[i][j] == 0){grid.cell(i,j)._cellType = NOSLIP;}
            else if(cell_array[i][j] == 4){grid.cell(i,j)._cellType = FLUID;}
            else if(cell_array[i][j] == 3){grid.cell(i,j)._cellType = OUTFLOW;}
            else if(cell_array[i][j] == 2){grid.cell(i,j)._cellType = INFLOW;}
            else if(cell_array[i][j] == 1){grid.cell(i,j)._cellType = FREESLIP;}
            else{
                printf("Error: wrong grey value in geometry-file ");
                exit(EXIT_FAILURE);
            }
            //for debugging
            //std::cout << cell_array[i][j] << " ";
        }
        //for debugging
        //std::cout << std::endl;
    }

    assign_ptr_nbcells(grid);
    */



    // Initializing variables
    double time = 0;                        // time
    int timesteps_total = 0;                // # of iterations for main loop
    int current_timestep_iteration;         // # of iterations for SOR
    double visualization_time_accumulator = 0.0;        // signals when it's time to visualize within the main loop
    int count_failed_SOR = 0;               //# of failed SOR iterations

    //initialize matrices U, V, P, T
    matrix<double> U, V, P, T;
    init_uvpt(*imax, *jmax, U, V, P, T, *UI, *VI, *PI, *TI, grid, *il, *ir, *jb, *jt);
    //initialize matrices F, G and RS
    matrix<double> F, G, RS;

    //assign initial values FI, GI and RSI on the hole domain for F, G and RS
    init_fgrs(*imax, *jmax, F, G, RS, 0, 0, 0, grid, *il, *ir, *jb, *jt);


    //vtkOutput.printVTKFile(grid, *dx, *dy, SCENARIO_NAME, SCENARIO_NAME, timesteps_total);



    // Initialize timer to measure performance
    Timer runtime;
    *dt = 0.005;
    while (time < *t_end) {
   
        //here we set time steps manually
        //calculate_dt(*Re, *Pr, *tau, dt, *dx, *dy, *imax, *jmax, grid, il, ir, jb, jt);
        //boundaryvalues(*imax, *jmax, grid, *v_inflow, *u_inflow, F, G, *T_h, *T_c, *dx, *dy, *kappa, *heat_flux, *beta, *dt, *GX, *GY, scenarioSpec);
        spec_boundary_val(grid, *u_inflow, *v_inflow, *T_c, *T_h, *kappa, *heat_flux, U, V, P, T, F, G, *il, *ir, *jb, *jt);
        //calculate_temp(*Re, *Pr, *alpha, *dt, *dx, *dy, *imax, *jmax, grid, il, ir, jb, jt);
        calculate_fg(*Re, *beta, *GX, *GY, *alpha, *dt, *dx, *dy, *imax, *jmax, grid, F, G, *il, *ir, *jb, *jt);
        calculate_rs(*dt, *dx, *dy, *imax, *jmax, F, G, RS, grid, *il, *ir, *jb, *jt);

        //reset current number of iterations for SOR
        current_timestep_iteration = 0;

        //reset residual before new SOR iteration
        *res = INFINITY;

        while ((*res > *eps) && (current_timestep_iteration <= *itermax)) {
            sor(*omg, *dx, *dy, *imax, *jmax, grid, RS, res, *il, *ir, *jb, *jt, myrank);
            current_timestep_iteration++;
        }

        //MPI_Status status;
        //pressure_comm(P, *il, *ir, *jb, *jt, *rank_l, *rank_r, *rank_b, *rank_t, bufSend, bufRecv, MPI_STATUS_IGNORE, myrank);



        //double summ = 0;
        //grid.pressure(P, il, ir, jb, jt);
        //for (int i = 0; i < 54; i++) {
        //    for (int j = 0; j < 54; j++) {
        //        summ += P[i][j];
        //    }
        //}
        //std::cout << summ / ((double)54 * 54) << std:: endl;
        //count number of failed SOR iterations
        if(*res > *eps){
            //print warning message after failed SOR iteration
            count_failed_SOR++;
        }

        calculate_uv(*dt, *dx, *dy, *imax, *jmax, grid, F, G, *il, *ir, *jb, *jt);
        visualization_time_accumulator += * dt;
        timesteps_total++;
        time += *dt;
        
        // Visualize u v p
        if (visualization_time_accumulator >= *dt_value) {
            grid.velocity(U, velocity_type::U, *il, *ir, *jb, *jt);
            grid.velocity(V, velocity_type::V, *il, *ir, *jb, *jt);
            grid.pressure(P, *il, *ir, *jb, *jt);
            grid.temperature(T, *il, *ir, *jb, *jt);
            //write_vtkFile(SCENARIO_NAME, timesteps_total, *xlength, *ylength, *imax, *jmax, *dx, *dy, U, V, P, T);
            vtkOutput.printVTKFile(grid, *dx, *dy, SCENARIO_NAME, SCENARIO_NAME, timesteps_total);
            //output_uvp_parallel(U,V, P, *il, *ir, *jb, *jt, *omg_i, *omg_j,  )


            if(myrank==0)
            solutionProgress(time, *t_end); // Print out total progress with respect to the simulation timerange
            visualization_time_accumulator -= *dt_value;
        }
    }

    grid.velocity(U, velocity_type::U, *il, *ir, *jb, *jt);
    grid.velocity(V, velocity_type::V, *il, *ir, *jb, *jt);
    grid.pressure(P, *il, *ir, *jb, *jt);
    grid.temperature(T, *il, *ir, *jb, *jt);

    //write_vtkFile(SCENARIO_NAME, timesteps_total, *xlength, *ylength, *imax, *jmax, *dx, *dy, U, V, P, T);
    vtkOutput.printVTKFile(grid, *dx, *dy, SCENARIO_NAME, SCENARIO_NAME, timesteps_total);
          
    // Print out the total time required for the solution
    runtime.printTimer();

    //Print total number of timesteps and number of failed SOR iterations
    std::cout << "#total of timesteps: " << timesteps_total << " #failed SOR iterations: " << count_failed_SOR << std::endl;


    //close input file
    fclose(parameterFile);
    fclose(geometryFile);
   
    // Free dynamically allocated memory
    delete[] cell_array;
    delete Re;
    delete UI;
    delete VI;
    delete PI;
    delete GX;
    delete GY;
    delete t_end;
    delete xlength;
    delete ylength;
    delete dt;
    delete dx;
    delete dy;
    delete imax;
    delete jmax;
    delete alpha;
    delete omg;
    delete tau;
    delete itermax;
    delete eps;
    delete dt_value;
    delete TI;
    delete T_h;
    delete T_c;
    delete Pr;
    delete res;
    delete beta;
    delete v_inflow;
    delete u_inflow;
    delete kappa;
    delete heat_flux;
    delete iproc;
    delete jproc;
    delete il;
    delete ir;
    delete jb;
    delete jt;
    delete rank_l;
    delete rank_r;
    delete rank_b;
    delete rank_t;
    delete omg_i;
    delete omg_j;
    delete[] bufSend;
    delete[] bufRecv;

    MPI_Finalize();

    return 0;
}
