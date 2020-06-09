#include "parallel.hpp"
#include <math.h>
#include <mpi.h>



void init_parallel(
    int iproc,
    int jproc,
    int imax,
    int jmax,
    int* myrank,
    int* il,
    int* ir,
    int* jb,
    int* jt,
    int* rank_l,
    int* rank_r,
    int* rank_b,
    int* rank_t,
    int* omg_i,
    int* omg_j,
    int num_proc) {


    // compute indices for subdomains
    float i_delta_f = static_cast<float>(imax + 2) / static_cast<float>(iproc);
    float j_delta_f = static_cast<float>(jmax + 2) / static_cast<float>(jproc);
    int i_delta = ceil(i_delta_f);
    int j_delta = ceil(j_delta_f);
    int i_delta_last_col = (imax + 2) - (iproc - 1) * i_delta;
    int j_delta_last_row = (jmax + 2) - (jproc - 1) * j_delta;

    int ir_arr[iproc], il_arr[iproc], jb_arr[jproc], jt_arr[jproc];

    /* omg_i = {1, 2,..., iproc}, omg_j = {1, 2,..., jproc} */
    int omg_i_arr[iproc], omg_j_arr[jproc];

    // store indices for columns
    for (int i = 0; i < iproc - 1; i++) {
        il_arr[i] = i * i_delta;
        ir_arr[i] = i * i_delta + i_delta - 1;

        omg_i_arr[i] = i + 1;
    }

    // store indices for last column
    il_arr[iproc - 1] = (iproc - 1) * i_delta;
    ir_arr[iproc - 1] = (iproc - 1) * i_delta + i_delta_last_col - 1;

    omg_i_arr[iproc - 1] = iproc;

    // store indices for rows
    for (int j = 0; j < jproc - 1; j++) {
        jb_arr[j] = j * j_delta;
        jt_arr[j] = j * j_delta + j_delta - 1;

        omg_j_arr[j] = j + 1;
    }

    //store indices for last row
    jb_arr[jproc - 1] = (jproc - 1) * j_delta;
    jt_arr[jproc - 1] = (jproc - 1) * j_delta + j_delta_last_row - 1;

    omg_j_arr[jproc - 1] = jproc;

    // send the indices from rank 0 (master process) to the assigned processes
    if (*myrank == 0) {
        for (int j = 0; j < jproc; j++) {
            for (int i = 0; i < iproc; i++) {
                int proc = (i + j * iproc); //% num_proc; //start sending to process 0
                //int tag = (i + j * iproc);

                *omg_i = omg_i_arr[i];
                *omg_j = omg_j_arr[j];

                *il = il_arr[i];
                *ir = ir_arr[i];
                *jb = jb_arr[j];
                *jt = jt_arr[j];


                // check weather neighbour process is outside of boundary or not and specify ranks of neighbour processes
                if (*il == 0) { *rank_l = MPI_PROC_NULL; }
                else { *rank_l = proc - 1; }
                if (*jb == 0) { *rank_b = MPI_PROC_NULL; }
                else { *rank_b = proc - iproc; }
                if (*ir == imax + 1) { *rank_r = MPI_PROC_NULL; }
                else { *rank_r = proc + 1; }
                if (*jt == jmax + 1) { *rank_t = MPI_PROC_NULL; }
                else { *rank_t = proc + iproc; }


                //check for safty
                if(proc < num_proc) {
                    MPI_Send(omg_i, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);
                    MPI_Send(omg_j, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);

                    MPI_Send(il, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);
                    MPI_Send(ir, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);
                    MPI_Send(jb, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);
                    MPI_Send(jt, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);

                    MPI_Send(rank_l, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);
                    MPI_Send(rank_r, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);
                    MPI_Send(rank_b, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);
                    MPI_Send(rank_t, 1, MPI_INT, proc, proc, MPI_COMM_WORLD);
                }
            }
        }
    }
    // receive indices from rank 0
    for (int j = 0; j < jproc; j++) {
        for (int i = 0; i < iproc; i++) {
            //int tag = (i + j * iproc);
            int proc = (i + j * iproc); //% num_proc;
            if (*myrank == proc) {
                MPI_Recv(omg_i, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(omg_j, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Recv(il, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(ir, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(jb, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(jt, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                MPI_Recv(rank_l, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(rank_r, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(rank_b, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Recv(rank_t, 1, MPI_INT, 0, proc, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            }
        }
    }
}




// --------- Communication of pressure P --------- //

void pressure_comm(
    matrix<double>& P,
    int il,
    int ir,
    int jb,
    int jt,
    int rank_l,
    int rank_r,
    int rank_b,
    int rank_t,
    double* bufSend,
    double* bufRecv,
    MPI_Status* status,
    int chunk)
{

    // Size of the chunk in y-direction (rows)
    // inner cells + 1 cell of BOTTOM ghost layer + 1 cell of TOP ghost layer in y-direction
    int chunk_size_Y = jt - jb + 3;


    // Size of the chunk in x-direction (columns)
    // inner cells + 1 cell of LEFT ghost layer + 1 cell of RIGHT ghost layer in x-direction
    int chunk_size_X = ir - il + 3;


    // ---------- Send to the LEFT - receive from the RIGHT ---------- //

    // Fill send buffer with pressure belonging to the chunk's internal cells of the LEFT wall, i.e. i=1,
    // which will become the ghost cells of the RIGHT wall of the receiving process.

    for (int j = 0; j < chunk_size_Y; j++)
        bufSend[j] = P[1][j];

    MPI_Send(bufSend, chunk_size_Y, MPI_DOUBLE, rank_l,
        CommunicationType::TO_LEFT, MPI_COMM_WORLD);


    // ---------- Send to the RIGHT - receive from the LEFT ---------- //

    // Fill send buffer with pressure belonging to the chunk's internal cells of the RIGHT wall,
    // i.e. i=chunk_size_X - 2,
    // which will become the ghost cells of the LEFT wall of the receiving process.

    for (int j = 0; j < chunk_size_Y; j++)
        bufSend[j] = P[chunk_size_X - 2][j];

    MPI_Send(bufSend, chunk_size_Y, MPI_DOUBLE, rank_r,
        CommunicationType::TO_RIGHT, MPI_COMM_WORLD);


    // ---------- Send to the BOTTOM - receive from the TOP ---------- //

    // Fill send buffer with pressure belonging to the chunk's internal cells of the BOTTOM wall, i.e. j=1,
    // which will become the ghost cells of the TOP wall of the receiving process.

    for (int i = 0; i < chunk_size_X; i++)
        bufSend[i] = P[i][1];

    MPI_Send(bufSend, chunk_size_X, MPI_DOUBLE, rank_b,
        CommunicationType::TO_BOTTOM, MPI_COMM_WORLD);


    // ---------- Send to the TOP - receive from the BOTTOM ---------- //

    // Fill send buffer with pressure belonging to the chunk's internal cells of the TOP wall, i.e. j=chunk_size_Y - 2,
    // which will become the ghost cells of the BOTTOM wall of the receiving process.

    for (int i = 0; i < chunk_size_X; i++)
        bufSend[i] = P[i][chunk_size_Y - 2];

    MPI_Send(bufSend, chunk_size_X, MPI_DOUBLE, rank_t,
        CommunicationType::TO_TOP, MPI_COMM_WORLD);


    // UPDATE the pressure values of the ghost cells in accordance with the received message:

    // RIGHT wall 
    if(rank_r != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y, MPI_DOUBLE, rank_r,
            CommunicationType::TO_LEFT, MPI_COMM_WORLD, status);

        for (int j = 0; j < chunk_size_Y; j++)
            P[chunk_size_X - 1][j] = bufRecv[j];
    }

    // LEFT wall
    if(rank_l != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y, MPI_DOUBLE, rank_l,
            CommunicationType::TO_RIGHT, MPI_COMM_WORLD, status);

        for (int j = 0; j < chunk_size_Y; j++)
            P[0][j] = bufRecv[j];
    }

    // TOP wall
    if(rank_t != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X, MPI_DOUBLE, rank_t,
            CommunicationType::TO_BOTTOM, MPI_COMM_WORLD, status);

        for (int i = 0; i < chunk_size_X; i++)
            P[i][chunk_size_Y - 1] = bufRecv[i];
    }

    // BOTTOM wall
    if(rank_b != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X, MPI_DOUBLE, rank_b,
            CommunicationType::TO_TOP, MPI_COMM_WORLD, status);

        for (int i = 0; i < chunk_size_X; i++)
            P[i][0] = bufRecv[i];
    }
}


// --------- Communication of U velocity (along x axis) --------- //

void u_velocity_comm(
    matrix<double> U,
    int il,
    int ir,
    int jb,
    int jt,
    int rank_l,
    int rank_r,
    int rank_b,
    int rank_t,
    double* bufSend,
    double* bufRecv,
    MPI_Status* status,
    int chunk)
{

    // Size of the chunk in y-direction (rows)
    // inner cells + 2 cells of BOTTOM ghost layer + 1 cell of TOP ghost layer in y-direction
    int chunk_size_Y = jt - jb + 3;


    // Size of the chunk in x-direction (columns)
    // inner cells + 2 cells of LEFT ghost layer + 1 cell of RIGHT ghost layer in x-direction
    int chunk_size_X = ir - il + 4;

    int left_ghost_layer_width = 2;
    int bottom_ghost_layer_width = 1;

    // ---------- Send to the LEFT ---------- //

    // Fill send buffer with velocities belonging to the chunk's internal cells of the LEFT wall, i.e. i=2,
    // which will become the ghost cells of the RIGHT wall of the receiving process.

    for (int j = 0; j < chunk_size_Y; j++)
        bufSend[j] = U[2][j];

    MPI_Send(bufSend, chunk_size_Y, MPI_DOUBLE, rank_l,
        CommunicationType::TO_LEFT, MPI_COMM_WORLD);


    // ---------- Send to the RIGHT ---------- //

    // Fill send buffer with velocities belonging to the chunk's internal cells of the RIGHT wall,
    // i.e. i=[chunk_size_X - 3, chunk_size_X - 2],
    // which will become the ghost cells of the LEFT wall of the receiving process.

    for (int i = 0; i < left_ghost_layer_width; i++)
        for (int j = 0; j < chunk_size_Y; j++)
            bufSend[i * chunk_size_Y + j] = U[i + chunk_size_X - 3][j];

    MPI_Send(bufSend, chunk_size_Y * left_ghost_layer_width, MPI_DOUBLE, rank_r,
        CommunicationType::TO_RIGHT, MPI_COMM_WORLD);


    // ---------- Send to the BOTTOM ---------- //

    // Fill send buffer with velocities belonging to the chunk's internal cells of the BOTTOM wall, i.e. j=1,
    // which will become the ghost cells of the TOP wall of the receiving process.

    for (int i = 0; i < chunk_size_X; i++)
        bufSend[i] = U[i][1];

    MPI_Send(bufSend, chunk_size_X, MPI_DOUBLE, rank_b,
        CommunicationType::TO_BOTTOM, MPI_COMM_WORLD);


    // ---------- Send to the TOP ---------- //

    // Fill send buffer with velocities belonging to the chunk's internal cells of the TOP wall, i.e. j=chunk_size_Y - 2,
    // which will become the ghost cells of the BOTTOM wall of the receiving process.

    for (int i = 0; i < chunk_size_X; i++)
        bufSend[i] = U[i][chunk_size_Y - 2];

    MPI_Send(bufSend, chunk_size_X, MPI_DOUBLE, rank_t,
        CommunicationType::TO_TOP, MPI_COMM_WORLD);


    // UPDATE the velocity values of the ghost cells in accordance with the received message:
    
    // ---------- Receive from the RIGHT ---------- //

    if (rank_r != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y, MPI_DOUBLE, rank_r,
            CommunicationType::TO_LEFT, MPI_COMM_WORLD, status);

        for (int j = 0; j < chunk_size_Y; j++)
            U[chunk_size_X - 1][j] = bufRecv[j];
    }

    // ---------- Receive from the LEFT - two (!) columns ---------- //

    if (rank_l != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y * left_ghost_layer_width, MPI_DOUBLE, rank_l,
            CommunicationType::TO_RIGHT, MPI_COMM_WORLD, status);

        for (int i = 0; i < left_ghost_layer_width; i++)
            for (int j = 0; j < chunk_size_Y; j++)
                U[i][j] = bufRecv[i * chunk_size_Y + j];
    }

    // ---------- Receive from the TOP ---------- //
    if (rank_t != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X, MPI_DOUBLE, rank_t,
            CommunicationType::TO_BOTTOM, MPI_COMM_WORLD, status);

        for (int i = 0; i < chunk_size_X; i++)
            U[i][chunk_size_Y - 1] = bufRecv[i];
    }

    // ---------- Receive from the BOTTOM ---------- //
    if (rank_b != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X, MPI_DOUBLE, rank_b,
            CommunicationType::TO_TOP, MPI_COMM_WORLD, status);

        for (int i = 0; i < chunk_size_X; i++)
            U[i][0] = bufRecv[i];
    }
}


// --------- Communication of V velocity (along y axis) --------- //

void v_velocity_comm(matrix<double> V,
    int il,
    int ir,
    int jb,
    int jt,
    int rank_l,
    int rank_r,
    int rank_b,
    int rank_t,
    double* bufSend,
    double* bufRecv,
    MPI_Status* status,
    int chunk)
{

    // Size of the chunk in y-direction (rows)
    // inner cells + 2 cells of BOTTOM ghost layer + 1 cell of TOP ghost layer in y-direction
    int chunk_size_Y = jt - jb + 4;


    // Size of the chunk in x-direction (columns)
    // inner cells + 2 cells of LEFT ghost layer + 1 cell of RIGHT ghost layer in x-direction
    int chunk_size_X = ir - il + 3;

    int left_ghost_layer_width = 1;
    int bottom_ghost_layer_width = 2;


    // ---------- Send to the LEFT ---------- //

    // Fill send buffer with velocities belonging to the chunk's internal cells of the LEFT wall, i.e. i=2,
    // which will become the ghost cells of the RIGHT wall of the receiving process.

    for (int j = 0; j < chunk_size_Y; j++)
        bufSend[j] = V[1][j];

    MPI_Send(bufSend, chunk_size_Y, MPI_DOUBLE, rank_l,
        CommunicationType::TO_LEFT, MPI_COMM_WORLD);


    // ---------- Send to the RIGHT ---------- //

    // Fill send buffer with velocities belonging to the chunk's internal cells of the RIGHT wall,
    // i.e. i=chunk_size_X - 2,
    // which will become the ghost cells of the LEFT wall of the receiving process.

    for (int j = 0; j < chunk_size_Y; j++)
        bufSend[j] = V[chunk_size_X - 2][j];

    MPI_Send(bufSend, chunk_size_Y, MPI_DOUBLE, rank_r,
        CommunicationType::TO_RIGHT, MPI_COMM_WORLD);


    // ---------- Send to the BOTTOM ---------- //

    // Fill send buffer with velocities belonging to the chunk's internal cells of the BOTTOM wall, i.e. j=2,
    // which will become the ghost cells of the TOP wall of the receiving process.

    for (int i = 0; i < chunk_size_X; i++)
        bufSend[i] = V[i][2];

    MPI_Send(bufSend, chunk_size_X, MPI_DOUBLE, rank_b,
        CommunicationType::TO_BOTTOM, MPI_COMM_WORLD);


    // ---------- Send to the TOP ---------- //

    // Fill send buffer with velocities belonging to the chunk's internal cells of the TOP wall,
    // i.e. j=[chunk_size_Y - 3, chunk_size_Y - 2]
    // which will become the ghost cells of the BOTTOM wall of the receiving process.

    for (int j = 0; j < bottom_ghost_layer_width; j++)
        for (int i = 0; i < chunk_size_X; i++)
            bufSend[j * chunk_size_X + i] = V[i][j + chunk_size_Y - 3];

    MPI_Send(bufSend, chunk_size_X * bottom_ghost_layer_width, MPI_DOUBLE, rank_t,
        CommunicationType::TO_TOP, MPI_COMM_WORLD);

    // UPDATE the velocity values of ghost cells in accordance with the received message:

    // ---------- Receive from the RIGHT ---------- //
    if (rank_r != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y, MPI_DOUBLE, rank_r,
            CommunicationType::TO_LEFT, MPI_COMM_WORLD, status);

        for (int j = 0; j < chunk_size_Y; j++)
            V[chunk_size_X - 1][j] = bufRecv[j];
    }

    // ---------- Receive from the LEFT ---------- //
    if (rank_l != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y, MPI_DOUBLE, rank_l,
            CommunicationType::TO_RIGHT, MPI_COMM_WORLD, status);

        for (int j = 0; j < chunk_size_Y; j++)
            V[0][j] = bufRecv[j];
    }

    // ---------- Receive from the TOP ---------- //
    if (rank_t != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X, MPI_DOUBLE, rank_t,
            CommunicationType::TO_BOTTOM, MPI_COMM_WORLD, status);

        for (int i = 0; i < chunk_size_X; i++)
            V[i][chunk_size_Y - 1] = bufRecv[i];
    }

    // ---------- Receive from the BOTTOM - two (!) rows ---------- //
    if (rank_b != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X * bottom_ghost_layer_width, MPI_DOUBLE, rank_b,
            CommunicationType::TO_TOP, MPI_COMM_WORLD, status);

        for (int j = 0; j < bottom_ghost_layer_width; j++)
            for (int i = 0; i < chunk_size_X; i++)
                V[i][j] = bufRecv[j * chunk_size_X + i];
    }
}


// --------- Communication of F-forces (along x axis) --------- //

void f_force_comm(
    double** F,
    int il,
    int ir,
    int jb,
    int jt,
    int rank_l,
    int rank_r,
    int rank_b,
    int rank_t,
    double* bufSend,
    double* bufRecv,
    MPI_Status* status,
    int chunk)
{

    // Size of the chunk in y-direction (rows)
    // inner cells + 2 cells of BOTTOM ghost layer + 1 cell of TOP ghost layer in y-direction
    int chunk_size_Y = jt - jb + 3;


    // Size of the chunk in x-direction (columns)
    // inner cells + 2 cells of LEFT ghost layer + 1 cell of RIGHT ghost layer in x-direction
    int chunk_size_X = ir - il + 4;

    int left_ghost_layer_width = 2;
    int bottom_ghost_layer_width = 1;

    // ---------- Send to the LEFT ---------- //

    // Fill send buffer with F-forces belonging to the chunk's internal cells of the LEFT wall, i.e. i=2,
    // which will become the ghost cells of the RIGHT wall of the receiving process.

    for (int j = 0; j < chunk_size_Y; j++)
        bufSend[j] = F[2][j];

    MPI_Send(bufSend, chunk_size_Y, MPI_DOUBLE, rank_l,
        CommunicationType::TO_LEFT, MPI_COMM_WORLD);


    // ---------- Send to the RIGHT ---------- //

    // Fill send buffer with F-forces belonging to the chunk's internal cells of the RIGHT wall,
    // i.e. i=[chunk_size_X - 3, chunk_size_X - 2],
    // which will become the ghost cells of the LEFT wall of the receiving process.

    for (int i = 0; i < left_ghost_layer_width; i++)
        for (int j = 0; j < chunk_size_Y; j++)
            bufSend[i * chunk_size_Y + j] = F[i + chunk_size_X - 3][j];

    MPI_Send(bufSend, chunk_size_Y * left_ghost_layer_width, MPI_DOUBLE, rank_r,
        CommunicationType::TO_RIGHT, MPI_COMM_WORLD);


    // ---------- Send to the BOTTOM ---------- //

    // Fill send buffer with F-forces belonging to the chunk's internal cells of the BOTTOM wall, i.e. j=1,
    // which will become the ghost cells of the TOP wall of the receiving process.

    for (int i = 0; i < chunk_size_X; i++)
        bufSend[i] = F[i][1];

    MPI_Send(bufSend, chunk_size_X, MPI_DOUBLE, rank_b,
        CommunicationType::TO_BOTTOM, MPI_COMM_WORLD);


    // ---------- Send to the TOP ---------- //

    // Fill send buffer with F-forces belonging to the chunk's internal cells of the TOP wall, i.e. j=chunk_size_Y - 2,
    // which will become the ghost cells of the BOTTOM wall of the receiving process.

    for (int i = 0; i < chunk_size_X; i++)
        bufSend[i] = F[i][chunk_size_Y - 2];

    MPI_Send(bufSend, chunk_size_X, MPI_DOUBLE, rank_t,
        CommunicationType::TO_TOP, MPI_COMM_WORLD);


    // UPDATE the F-forces values of the ghost cells in accordance with the received message:

    // ---------- Receive from the RIGHT ---------- //

    if (rank_r != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y, MPI_DOUBLE, rank_r,
            CommunicationType::TO_LEFT, MPI_COMM_WORLD, status);

        for (int j = 0; j < chunk_size_Y; j++)
            F[chunk_size_X - 1][j] = bufRecv[j];
    }

    // ---------- Receive from the LEFT - two (!) columns ---------- //

    if (rank_l != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y * left_ghost_layer_width, MPI_DOUBLE, rank_l,
            CommunicationType::TO_RIGHT, MPI_COMM_WORLD, status);

        for (int i = 0; i < left_ghost_layer_width; i++)
            for (int j = 0; j < chunk_size_Y; j++)
                F[i][j] = bufRecv[i * chunk_size_Y + j];
    }

    // ---------- Receive from the TOP ---------- //
    if (rank_t != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X, MPI_DOUBLE, rank_t,
            CommunicationType::TO_BOTTOM, MPI_COMM_WORLD, status);

        for (int i = 0; i < chunk_size_X; i++)
            F[i][chunk_size_Y - 1] = bufRecv[i];
    }

    // ---------- Receive from the BOTTOM ---------- //
    if (rank_b != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X, MPI_DOUBLE, rank_b,
            CommunicationType::TO_TOP, MPI_COMM_WORLD, status);

        for (int i = 0; i < chunk_size_X; i++)
            F[i][0] = bufRecv[i];
    }
}


// --------- Communication of G-forces (along y axis) --------- //

void g_force_comm(
    double** G,
    int il,
    int ir,
    int jb,
    int jt,
    int rank_l,
    int rank_r,
    int rank_b,
    int rank_t,
    double* bufSend,
    double* bufRecv,
    MPI_Status* status,
    int chunk)
{

    // Size of the chunk in y-direction (rows)
    // inner cells + 2 cells of BOTTOM ghost layer + 1 cell of TOP ghost layer in y-direction
    int chunk_size_Y = jt - jb + 4;


    // Size of the chunk in x-direction (columns)
    // inner cells + 2 cells of LEFT ghost layer + 1 cell of RIGHT ghost layer in x-direction
    int chunk_size_X = ir - il + 3;

    int left_ghost_layer_width = 1;
    int bottom_ghost_layer_width = 2;


    // ---------- Send to the LEFT ---------- //

    // Fill send buffer with G-forces belonging to the chunk's internal cells of the LEFT wall, i.e. i=2,
    // which will become the ghost cells of the RIGHT wall of the receiving process.

    for (int j = 0; j < chunk_size_Y; j++)
        bufSend[j] = G[1][j];

    MPI_Send(bufSend, chunk_size_Y, MPI_DOUBLE, rank_l,
        CommunicationType::TO_LEFT, MPI_COMM_WORLD);


    // ---------- Send to the RIGHT ---------- //

    // Fill send buffer with G-forces belonging to the chunk's internal cells of the RIGHT wall,
    // i.e. i=chunk_size_X - 2,
    // which will become the ghost cells of the LEFT wall of the receiving process.

    for (int j = 0; j < chunk_size_Y; j++)
        bufSend[j] = G[chunk_size_X - 2][j];

    MPI_Send(bufSend, chunk_size_Y, MPI_DOUBLE, rank_r,
        CommunicationType::TO_RIGHT, MPI_COMM_WORLD);


    // ---------- Send to the BOTTOM ---------- //

    // Fill send buffer with G-forces belonging to the chunk's internal cells of the BOTTOM wall, i.e. j=2,
    // which will become the ghost cells of the TOP wall of the receiving process.

    for (int i = 0; i < chunk_size_X; i++)
        bufSend[i] = G[i][2];

    MPI_Send(bufSend, chunk_size_X, MPI_DOUBLE, rank_b,
        CommunicationType::TO_BOTTOM, MPI_COMM_WORLD);


    // ---------- Send to the TOP ---------- //

    // Fill send buffer with G-forces belonging to the chunk's internal cells of the TOP wall,
    // i.e. j=[chunk_size_Y - 3, chunk_size_Y - 2]
    // which will become the ghost cells of the BOTTOM wall of the receiving process.

    for (int j = 0; j < bottom_ghost_layer_width; j++)
        for (int i = 0; i < chunk_size_X; i++)
            bufSend[j * chunk_size_X + i] = G[i][j + chunk_size_Y - 3];

    MPI_Send(bufSend, chunk_size_X * bottom_ghost_layer_width, MPI_DOUBLE, rank_t,
        CommunicationType::TO_TOP, MPI_COMM_WORLD);

    // UPDATE the velocity values of ghost cells in accordance with the received message:

    // ---------- Receive from the RIGHT ---------- //
    if (rank_r != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y, MPI_DOUBLE, rank_r,
            CommunicationType::TO_LEFT, MPI_COMM_WORLD, status);

        for (int j = 0; j < chunk_size_Y; j++)
            G[chunk_size_X - 1][j] = bufRecv[j];
    }

    // ---------- Receive from the LEFT ---------- //
    if (rank_l != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_Y, MPI_DOUBLE, rank_l,
            CommunicationType::TO_RIGHT, MPI_COMM_WORLD, status);

        for (int j = 0; j < chunk_size_Y; j++)
            G[0][j] = bufRecv[j];
    }

    // ---------- Receive from the TOP ---------- //
    if (rank_t != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X, MPI_DOUBLE, rank_t,
            CommunicationType::TO_BOTTOM, MPI_COMM_WORLD, status);

        for (int i = 0; i < chunk_size_X; i++)
            G[i][chunk_size_Y - 1] = bufRecv[i];
    }

    // ---------- Receive from the BOTTOM - two (!) rows ---------- //
    if (rank_b != MPI_PROC_NULL)
    {
        MPI_Recv(bufRecv, chunk_size_X * bottom_ghost_layer_width, MPI_DOUBLE, rank_b,
            CommunicationType::TO_TOP, MPI_COMM_WORLD, status);

        for (int j = 0; j < bottom_ghost_layer_width; j++)
            for (int i = 0; i < chunk_size_X; i++)
                G[i][j] = bufRecv[j * chunk_size_X + i];
    }
}