#include "init.hpp"
#include <fstream>

int read_parameters(std::string szFileName,       /* name of the file */
	double* Re,                /* reynolds number   */
	double* UI,                /* velocity x-direction */
	double* VI,                /* velocity y-direction */
	double* PI,                /* pressure */
	double* GX,                /* gravitation x-direction */
	double* GY,                /* gravitation y-direction */
	double* t_end,             /* end time */
	double* xlength,           /* length of the domain x-dir.*/
	double* ylength,           /* length of the domain y-dir.*/
	double* dt,                /* time step */
	double* dx,                /* length of a cell x-dir. */
	double* dy,                /* length of a cell y-dir. */
	int* imax,                /* number of cells x-direction*/
	int* jmax,                /* number of cells y-direction*/
	double* alpha,             /* uppwind differencing factor*/
	double* omg,               /* relaxation factor */
	double* tau,               /* safety factor for time step*/
	int* itermax,             /* max. number of iterations  */
	double* eps,                 /* accuracy bound for pressure*/
	double* dt_value,            /* time for output */
	double* TI,                   /* Initial temperature */
	double* T_h,                  /* hot wall temperature */
	double* T_c,                  /* cold wall temperature */
	double* PR,                      /*Prandlt Number*/
	double* beta,
	double* v_inflow,          /* inflow y velocity */
	double* u_inflow,          /* inflow x velocity */
	double* kappa,				/* thermal conductivity */
	double* heat_flux,		 /* heat flux */
	double* CI_A,             /* initial concentration */
	double* CI_B,             /* initial concentration */
	double* CI_C,             /* initial concentration */
	double* CI_D,             /* initial concentration */
	double* C_inject_A,            /* source concentration*/
	double* C_inject_B,            /* source concentration*/
	double* C_inject_C,            /* source concentration*/
	double* C_inject_D,            /* source concentration*/
	double* Pr_diffusion_A,            /* Prandlt Number for chemical diffusion*/
	double* Pr_diffusion_B,            /* Prandlt Number for chemical diffusion*/
	double* Pr_diffusion_C,            /* Prandlt Number for chemical diffusion*/
	double* Pr_diffusion_D,           /* Prandlt Number for chemical diffusion*/
	double* SD_coeff                  /* surface development coefficient */
                     )                     
{
    // Reading Parameters
	std::ifstream file(szFileName);
	if (!file.is_open()) return -1;
	std::string var;
	while (!file.eof() && file.good()) {
		file >> var;
		if (var[0] == '#') {     /* ignore comment line*/
			file.ignore(MAX_LINE_LENGTH, '\n');
		}
		else {
			if (var == "xlength")  file >> *xlength;
			if (var == "ylength")  file >> *ylength;
			if (var == "Re")       file >> *Re;
			if (var == "t_end")    file >> *t_end;
			if (var == "dt")       file >> *dt;
			if (var == "omg")      file >> *omg;
			if (var == "eps")      file >> *eps;
			if (var == "tau")      file >> *tau;
			if (var == "alpha")    file >> *alpha;
			if (var == "dt_value") file >> *dt_value;
			if (var == "UI")       file >> *UI;
			if (var == "VI")       file >> *VI;
			if (var == "GX")       file >> *GX;
			if (var == "GY")       file >> *GY;
			if (var == "PI")       file >> *PI;
			if (var == "itermax")  file >> *itermax;
			if (var == "imax")     file >> *imax;
			if (var == "jmax")     file >> *jmax;
			if (var == "TI")       file >> *TI;
			if (var == "T_h")      file >> *T_h;
			if (var == "T_c")      file >> *T_c;
			if (var == "PR")       file >> *PR;
			if (var == "beta")     file >> *beta;
			if (var == "u_inflow") file >> *u_inflow;
			if (var == "v_inflow") file >> *v_inflow;
			if (var == "kappa")    file >> *kappa;
			if (var == "heat_flux")file >> *heat_flux;
			if (var == "CI_A")	   file >> *CI_A;
			if (var == "CI_B")	   file >> *CI_B;
			if (var == "CI_C")	   file >> *CI_C;
			if (var == "CI_D")	   file >> *CI_D;
			if (var == "C_inject_A")	   file >> *C_inject_A;
			if (var == "C_inject_B")	   file >> *C_inject_B;
			if (var == "C_inject_C")	   file >> *C_inject_C;
			if (var == "C_inject_D")	   file >> *C_inject_D;
			if (var == "Pr_diffusion_A")  file >> *Pr_diffusion_A;
			if (var == "Pr_diffusion_B")  file >> *Pr_diffusion_B;
			if (var == "Pr_diffusion_C")  file >> *Pr_diffusion_C;
			if (var == "Pr_diffusion_D")  file >> *Pr_diffusion_D;
			if (var == "SD_coeff")		  file >> *SD_coeff;
		}
	}

	*dx = *xlength / (double)(*imax);
	*dy = *ylength / (double)(*jmax);

	if (!file.good() && !file.eof()) return -1;

	return 1;
}

