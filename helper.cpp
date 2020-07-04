#include <ctype.h>
#include <cerrno>
#include <cstdio>
#include "helper.hpp"
#include <fstream>
#include "regex"
#include "iostream"
#include "enums.hpp"
#include "grid.hpp"
#ifdef gpp9
// gcc Version >= 9
#include "filesystem"
namespace filesystem = std::filesystem;
#else
// gcc Version < 9
#include "experimental/filesystem"
namespace filesystem = std::experimental::filesystem;
#endif // DEBUG


/* ----------------------------------------------------------------------- */
/*                         local auxiliary functions                       */
/* ----------------------------------------------------------------------- */

clock_t last_timer_reset;

int min_int( const int n1, const int n2 )
{
    if( n1 < n2 ) return n1;
    return n2;
}

/* ----------------------------------------------------------------------- */
/*                             read datafile                               */
/* ----------------------------------------------------------------------- */

void errhandler( int nLine, const char *szFile, const char *szString )
{
    int err = errno;

    fprintf( ERROUT, "%s:%d Error : %s", szFile, nLine, szString );
    fprintf( ERROUT, "\n" );
    
    /* if an error within the c-library occured, an error code can be   */
    /* found in the global variable err                                 */
    if( err != 0 )
    {
	fprintf( ERROUT, "C-Lib   errno    = %d\n", err);
	fprintf( ERROUT, "C-Lib   strerror = %s\n", strerror( err ) );
    }
    exit(1);
}


/*  for comfort */
#define READ_ERROR(szMessage, szVarName, szFileName, nLine) \
  { char szTmp[80]; \
    if( nLine ) \
	sprintf( szTmp, " %s  File: %s   Variable: %s  Line: %d", szMessage, szFileName, szVarName, nLine ); \
    else \
	sprintf( szTmp, " %s  File: %s   Variable: %s ", szMessage, szFileName, szVarName); \
    ERROR( szTmp ); \
  }
    

/* --------------------------------------------------------------------------*/
/* The function searches the datafile fh for the line defining the variable  */
/* szVarName and returns the respctive string including the value of the     */
/* variable. If there's no appropriate line within the datafile, the program */
/* stops with an error messsage.                                             */
/* ATTENTION: The pointer returned refers to a static variable within the    */
/* function. To maintain the string over several program calls, it has to be */
/* copied!!!                                                                 */
/*                                                                           */
char* find_string( const char* szFileName, const char *szVarName )
{ 
    int nLine = 0;
    size_t i;
    FILE *fh = NULL;
    
    static char szBuffer[MAX_LINE_LENGTH];	/* containes the line read  */
                                               /* from the datafile        */

    char* szLine = szBuffer;
    char* szValue = NULL;
    char* szName = NULL;

    /* open file */
    fh = fopen( szFileName, "rt" );
    if( fh == 0 ) 
	READ_ERROR("Could not open file", szVarName, szFileName, 0);

    /* searching */
    while( ! feof(fh) )
    {
	fgets( szLine, MAX_LINE_LENGTH, fh );
	++nLine;

	/* remove comments */
	for( i = 0; i < std::strlen(szLine); i++)
	    if( szLine[i] == '#' )
	    {
		szLine[i] = '\0'; /* Stringende setzen */
		break;
	    }

	/* remove empty lines */
	while( isspace( (int)*szLine ) && *szLine) ++szLine;
	if( strlen( szLine ) == 0) continue; 

	/* now, the name can be extracted */
	szName = szLine;
	szValue = szLine;
	while( (isalnum( (int)*szValue ) || *szValue == '_') && *szValue) ++szValue;
	
	/* is the value for the respective name missing? */
	if( *szValue == '\n' || strlen( szValue) == 0)  
	    READ_ERROR("wrong format", szName, szFileName, nLine);
	
	*szValue = 0;		/* complete szName! at the right place */
	++szValue;
        
	/* read next line if the correct name wasn't found */
	if( strcmp( szVarName, szName)) continue;

	/* remove all leading blnkets and tabs from the value string  */
	while( isspace( (int)*szValue) ) ++szValue;
	if( *szValue == '\n' || strlen( szValue) == 0)  
	    READ_ERROR("wrong format", szName, szFileName, nLine);
	
	fclose(fh);
	return szValue;
    }  
   
    READ_ERROR("variable not found", szVarName, szFileName, nLine);
    
    return NULL;		/* dummy to satisfy the compiler  */
} 

void read_string( const char* szFileName, const char* szVarName, char*   pVariable)
{
    char* szValue = NULL;	/* string containg the read variable value */

    if( szVarName  == 0 )  ERROR("null pointer given as variable name" );
    if( szFileName == 0 )  ERROR("null pointer given as filename" );
    if( pVariable  == 0 )  ERROR("null pointer given as variable" );

    if( szVarName[0] == '*' )
	szValue = find_string( szFileName, szVarName +1 );
    else
	szValue = find_string( szFileName, szVarName );
    
    if( sscanf( szValue, "%s", pVariable) == 0)
	READ_ERROR("wrong format", szVarName, szFileName,0);

    printf( "File: %s\t\t%s%s= %s\n", szFileName, 
	                              szVarName,
	                              &("               "[min_int( strlen(szVarName), 15)]), 
	                              pVariable );
}

void read_int( const char* szFileName, const char* szVarName, int* pVariable)
{
    char* szValue = NULL;	/* string containing the read variable value */

    if( szVarName  == 0 )  ERROR("null pointer given as varable name" );
    if( szFileName == 0 )  ERROR("null pointer given as filename" );
    if( pVariable  == 0 )  ERROR("null pointer given as variable" );

    if( szVarName[0] == '*' )
	szValue = find_string( szFileName, szVarName +1 );
    else
	szValue = find_string( szFileName, szVarName );
    
    if( sscanf( szValue, "%d", pVariable) == 0)
	READ_ERROR("wrong format", szVarName, szFileName, 0);

    printf( "File: %s\t\t%s%s= %d\n", szFileName, 
	                              szVarName,
	                              &("               "[min_int( strlen(szVarName), 15)]), 
	                              *pVariable );
}

void read_double( const char* szFileName, const char* szVarName, double* pVariable)
{
    char* szValue = NULL;	/* String mit dem eingelesenen Variablenwert */

    if( szVarName  == 0 )  ERROR("null pointer given as varable name" );
    if( szFileName == 0 )  ERROR("null pointer given as filename" );
    if( pVariable  == 0 )  ERROR("null pointer given as variable" );

    if( szVarName[0] == '*' )
	szValue = find_string( szFileName, szVarName +1 );
    else
	szValue = find_string( szFileName, szVarName );
    
    if( sscanf( szValue, "%lf", pVariable) == 0)
	READ_ERROR("wrong format", szVarName, szFileName, 0);

    printf( "File: %s\t\t%s%s= %f\n", szFileName, 
	                              szVarName,
	                              &("               "[min_int( strlen(szVarName), 15)]), 
	                              *pVariable );
}

/* ----------------------------------------------------------------------- */
/*                   write matrices to a file                              */
/* ----------------------------------------------------------------------- */

void write_matrix( const char* szFileName,       /* filename */
		   double **m,		       /* matrix */
		   int nrl,		       /* first column */
		   int nrh,		       /* last column */
		   int ncl,		       /* first row */
		   int nch,		       /* last row */
		 double xlength,	       /* size of the geometry in */
                                               /* x-direction */
		 double ylength,	       /* size of the geometry in */
                                               /* y-direction  */
		   int fFirst ) 	       /* 0 == append, else overwrite*/
{
   int i, j;
   FILE * fh = 0;
   int nSize = (nrh-nrl+1) * (nch-ncl+1);
   float *tmp = new float[nSize];
   int k = 0;

   if( fFirst )				/* first call of the function ? */
   {
       fh = fopen( szFileName, "w");	/* overwrite file/write new file */
       if( fh == NULL )			/* opening failed ? */
       {
	   char szBuff[80];
	   sprintf( szBuff, "Outputfile %s cannot be created", szFileName );
	   ERROR( szBuff );
       }
       
/*       fprintf( fh,"%f\n%f\n%d\n%d\n%d\n%d\n", xlength, ylength, nrl, nrh, ncl, nch ); */
   }
   else
   {
       fh = fopen( szFileName ,"a");	/* append to the file */
       if( fh == NULL )			/* opening failed ? */
       {
	   char szBuff[80];
	   sprintf( szBuff, "Outputfile %s cannot be opened", szFileName );
	   ERROR( szBuff );
       }
   } 

   for( j = ncl; j <= nch; j++)
       for( i = nrl; i <= nrh; i++)
	   tmp[k++] = (float)m[i][j];

   fwrite( tmp, sizeof(float), nSize, fh);

   if( fclose(fh) )
   {
       char szBuff[80];
       sprintf( szBuff, "Outputfile %s cannot be closed", szFileName );
       ERROR( szBuff );
   };

   free( tmp );
}


void read_matrix( const char* szFileName,       /* filename */
		   double **m,		       /* matrix */
		   int nrl,		       /* first column */
		   int nrh,		       /* last column */
		   int ncl,		       /* first row */
		   int nch		       /* last row */
                  ) 	  
{
   int i, j;
   FILE * fh = 0;
   int nSize = (nrh-nrl+1) * (nch-ncl+1);
   float *tmp = new float[nSize];
   int k = 0;

       fh = fopen( szFileName, "r");	/* overwrite file/write new file */
       if( fh == NULL )			/* opening failed ? */
       {
	   char szBuff[80];
	   sprintf( szBuff, "Can not read file %s !!!", szFileName );
	   ERROR( szBuff );
       }


   fread( tmp, sizeof(float), nSize, fh);

   for( j = ncl; j <= nch; j++)
       for( i = nrl; i <= nrh; i++)
	   m[i][j]=tmp[k++];

   if( fclose(fh) )
   {
       char szBuff[80];
       /*orig bug:
       sscanf( szBuff, "Inputfile %s cannot be closed", szFileName );*/
       sprintf( szBuff, "Inputfile %s cannot be closed", szFileName );
       ERROR( szBuff );
   };

   free( tmp );
}


/* ----------------------------------------------------------------------- */
/*                      general matrix functions                           */
/* ----------------------------------------------------------------------- */

/*  allocates storage for a matrix                                         */
/*double **matrix( int nrl, int nrh, int ncl, int nch )
{
   int i;
   int nrow = nrh - nrl + 1;	*//* compute number of lines *//*
   int ncol = nch - ncl + 1;	*//* compute number of columns *//*

   double **pArray  = (double **) malloc((size_t)( nrow * sizeof(double*)) );
   double  *pMatrix = (double *)  malloc((size_t)( nrow * ncol * sizeof( double )));

   if( pArray  == 0)  ERROR("Storage cannot be allocated");
   if( pMatrix == 0)  ERROR("Storage cannot be allocated");

   *//* first entry of the array points to the value corrected by the
      beginning of the column *//*
   pArray[0] = pMatrix - ncl;

   *//* compute the remaining array entries *//*
   for( i = 1; i < nrow; i++ )
   {
       pArray[i] = pArray[i-1] + ncol;
   }

   // return the value corrected by the beginning of a line
   return pArray - nrl;
}*/


/* deallocates the storage of a matrix  */
void free_matrix( double **m, int nrl, int nrh, int ncl, int nch )
{
   double **pArray  = m + nrl;
   double  *pMatrix = m[nrl]+ncl;

   delete [] pMatrix;
   delete [] pArray;
}

void init_matrix( double **m, int nrl, int nrh, int ncl, int nch, double a)
{
   int i,j;
   for( i = nrl; i <= nrh; i++)
       for( j = ncl; j <= nch; j++)
	   m[i][j] = a;
}

void set_paths(int scenarioSpec, std::string& SCENARIO_NAME, std::string& SCENARIO_DAT_FILE, std::string& SCENARIO_PGM_FILE) {
    switch (scenarioSpec)
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

    case 8:
        printf("catalyst_reactor \n");
        SCENARIO_NAME = "catalyst_reactor";
        SCENARIO_DAT_FILE = "../parameters/catalyst_reactor.dat";
        SCENARIO_PGM_FILE = "../geometry/catalyst_reactor.pgm";
        break;

    case 10:
        printf("catalyst_reactor2 \n");
        SCENARIO_NAME = "catalyst_reactor2";
        SCENARIO_DAT_FILE = "../parameters/catalyst_reactor2.dat";
        SCENARIO_PGM_FILE = "../geometry/catalyst_reactor2.pgm";
        break;

    default:
        printf("Couldn't select any secnario \n");
        exit(EXIT_FAILURE);

    }

}
/* allocates storage for a matrix */
int **imatrix( int nrl, int nrh, int ncl, int nch )
{
   int i;

   int nrow = nrh - nrl + 1;	/* compute number of rows */
   int ncol = nch - ncl + 1;	/* compute number of columns */

   int **pArray  = (int **) malloc((size_t)( nrow * sizeof( int* )) );
   int  *pMatrix = (int *)  malloc((size_t)( nrow * ncol * sizeof( int )));


   if( pArray  == 0)  ERROR("Storage cannot be allocated");
   if( pMatrix == 0)  ERROR("Storage cannot be allocated");

   /* first entry of the array points to the value corrected by the
      beginning of the column */
   pArray[0] = pMatrix - ncl;

   /* compute the remaining array entries */
   for( i = 1; i < nrow; i++ )
   {
       pArray[i] = pArray[i-1] + ncol;
   }

   /* return the value corrected by the beginning of a line */
   return pArray - nrl;
}

/* deallocates the storage of a matrix  */
void free_imatrix( int **m, int nrl, int nrh, int ncl, int nch )
{
    int **pArray  = m + nrl;
    int  *pMatrix = m[nrl]+ncl;

   
    delete [] pMatrix;
    delete [] pArray;
}

void init_imatrix( int **m, int nrl, int nrh, int ncl, int nch, int a)
{
   int i,j;
   for( i = nrl; i <= nrh; i++)
       for( j = ncl; j <= nch; j++)
	   m[i][j] = a;
}

bool assert_problem_solvability(int** PGM_cellt, Grid &grid) {

    const int i_length= grid.imaxb();
    const int j_length= grid.jmaxb();
    int PGM_cell[i_length][j_length];
    int num_non_Fluid_Cells;
    int num_Fluid_Cells;
    for (int i = 0; i < grid.imaxb(); i++)
        for (int j = 0; j < grid.jmaxb(); j++) {
            PGM_cell[i][j] = PGM_cellt[i][j];
            if (PGM_cell[i][j] > 4)PGM_cell[i][j] = 0;
        }
    for (int i = 0; i < grid.imaxb(); i++) {
        if (PGM_cell[i][0] == 4 || PGM_cell[i][grid.jmaxb() -1] == 4)
            return false;
    }
    for (int j = 0; j < grid.jmaxb(); j++) {
        if (PGM_cell[0][j] == 4 || PGM_cell[grid.imaxb() -1][j] == 4)
            return false;
    }

    for (int i = 1; i < grid.imaxb() - 1; i++) {
        for (int j = 1; j < grid.jmaxb() - 1; j++) {
            num_Fluid_Cells = (PGM_cell[i][j + 1] == 4) + (PGM_cell[i][j - 1] == 4) + (PGM_cell[i+1][j] == 4) + (PGM_cell[i-1][j] == 4);
            num_non_Fluid_Cells = (PGM_cell[i][j + 1] != 4) + (PGM_cell[i][j - 1] != 4) + (PGM_cell[i + 1][j] != 4) + (PGM_cell[i - 1][j] != 4);
            if (PGM_cell[i][j] == 0 && PGM_cell[i][j+1] > 1 && PGM_cell[i][j - 1] > 1) return false;
            if (PGM_cell[i][j] == 0 && PGM_cell[i-1][j] > 1 && PGM_cell[i+1][j] > 1) return false;
            if (PGM_cell[i][j] == 4 && num_non_Fluid_Cells == 4) return false;
        }
    }
    for (int i = 1; i < grid.imaxb()-1; i++) {
        if (PGM_cell[i][0] == 4)return false;
        else if (PGM_cell[i][0] != 0 and PGM_cell[i][1] == 0) return false;
        if (PGM_cell[i][grid.jmaxb() -1] == 4)return false;
        else if (PGM_cell[i][grid.jmaxb() - 1] != 0 and PGM_cell[i][grid.jmaxb() - 2] == 0) return false;
    }
    for (int j = 1; j < grid.jmaxb()-1; j++) {
        if (PGM_cell[0][j] == 4)return false;
        else if (PGM_cell[0][j] != 0 and PGM_cell[1][j] == 0) return false;
        if (PGM_cell[grid.imaxb() - 1][j] == 4)return false;
        else if (PGM_cell[grid.imaxb() - 1][j] != 0 and PGM_cell[grid.imaxb() - 2][j] == 0) return false;
    }
    return true;
}

int **read_pgm(const char *filename, int imax, int jmax) {
    FILE *input = NULL;
    char line[1024];
    int levels;
    int xsize, ysize;
    int **pic = NULL;

    if ((input = fopen(filename, "rb")) == 0) {
        char szBuff[80];
        sprintf(szBuff, "Can not read file %s !!!", filename);
        ERROR(szBuff);
    }

    /* read first line*/
    if(fgets(line, sizeof line, input) == NULL) {
        ERROR("cannot read PGM.");
    }
    if(strncmp("P2", line, 2) != 0) {
        ERROR("file is not a valid PGM file. If file is a valid PGM file, first two characters of the first line must read P2");
    }

    /* skip the comments */
    int is_comment_line = (*line == '#');
    do{
        if(is_comment_line != 0)
	    printf("Skipping comment line: %s", line);
        if(fgets(line, sizeof line, input) == NULL)
            ERROR("cannot read PGM.");
        is_comment_line = (*line == '#');
    }while (is_comment_line);

    /* read the width and height */
    sscanf(line, "%d %d\n", &xsize, &ysize);
    if (xsize == imax + 2 and ysize == jmax + 2)
        printf("Image size: %d x %d\n", xsize, ysize);
    else {
        std::cout << "dimensions mismatch between pgm and input file... exiting";
        exit(EXIT_FAILURE);
    }
    
    /* get number of greyscale levels */
    fscanf(input, "%d", &levels);
    
    /* allocate memory for image */
    pic = imatrix(0, xsize-1, 0, ysize-1);
    printf("Image initialised...\n");

    for (int j = ysize-1; j >= 0; --j) {
        for (int i = 0; i < xsize; ++i) {
            int value;
            fscanf(input, "%d", &value);

            if (value == EOF) {
                fclose(input);
                ERROR("read of geometry file failed!");
            }
            pic[i][j] = value;
        }
    }

    /* close file */
    fclose(input);

    return pic;
}

/* ----------------------------------------------------------------------- */
/*                      folder creation functions                          */
/* ----------------------------------------------------------------------- */

/* checks if a respective folder exists */
void check_dir_exists(std::string outputfolder) {
    // Check if the provided string contains a top level directory
    // i.e. "results/output" -> top level directory is results
    filesystem::path folder(outputfolder);

    if (filesystem::is_directory(folder)) {
        std::cout << "Folder: "<< folder <<  " already exists." << std::endl;
    }
    // if the directory is provided like "results", the top level directory doesn't need to be used
    else {
        std::cout << "Folder: "<< folder <<  " doesn't exists." << std::endl;
        if (filesystem::create_directory(folder))
            std::cout << "Folder: "<< folder <<  " created successfully." << std::endl;
    }
}

