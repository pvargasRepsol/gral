//  ---------------------------------------------------------------------------
//  (c)   Copyright  REPSOL S.A.  All Right reserved
// 
//  parallel_ToyExample.cpp
// 
//  Created on:   Feb 2021
//      Author:   Pablo E Vargas - pabloenrique.vargas@repsol.com
// 
//  Generate a 3D integer array and operate it using
//  factor multiplication.
// 
//  Domain decomposition is based on nx size of 3D array, therefore
//  num processors and nx size must fit.
//  ---------------------------------------------------------------------------

# include <stdio.h>
# include <cstdlib>
# include <mpi.h>
# include <chrono>

# define MASTER 0
# define Verbose 1
# define Verbose_matrix 0

// ----------------------------------------------------------------------------
// Function headers
// ----------------------------------------------------------------------------

int main ( int argc, char *argv[] );
int ***int_vector_3D(int nx, int ny, int nz);
int ***change_vector_3D(int *** inVector, int nx, int ny, int nz, int factor);
int ***alloc_3d_int(int nx, int ny, int nz);
int *vector3D_2_1D(int *** in_vec, int nx, int ny, int nz);
int ***vector1D_2_3D(int *in_vec, int size1D, int nx, int ny, int nz);
void dealloc_3d_int(int ***A, int nx, int ny, int nz);
void print3DVector(int *** vec, int nx, int ny, int nz);
void print1DVector(int * vec, int n);

// ----------------------------------------------------------------------------
// Function definition
// ----------------------------------------------------------------------------

int main (int argc, char *argv[])
{

    // *************************************
    //   MPI Init
    // *************************************

    auto start_time = std::chrono::high_resolution_clock::now();
    
    int ierr = MPI_Init ( &argc, &argv );

    if ( ierr != 0 ) {
        printf ("\nFatal error!\n MPI_Init returned ierr = %d \n",ierr);
        exit (1);
    }

    int world_size, myRank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank ( MPI_COMM_WORLD, &myRank );

    int numProcs = world_size;
    
    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, myRank, world_size);

    // *************************************
    //   Check input arguments
    // *************************************

    if (argc != 5) {            
        if (myRank == MASTER) {
            fprintf (stderr,"\n\n nargs: %d \n----> Error! Requires at lest 4 int args: nx ny nz factor\n\n",argc);
        }
        MPI_Finalize();
        exit(1);
    }

    int nx = atoi(argv[1]);
    int ny = atoi(argv[2]);
    int nz = atoi(argv[3]);
    int factor = atoi(argv[4]);
    int matrix_size = nx*ny*nz;
    
    // Check size problem
    int remainder = nx % numProcs;

    if (remainder != 0) {
        if (myRank == MASTER) {
            printf("num procs (%d) must fit nx size (%d). Remainder = %d",
                numProcs,nx,remainder);
        }
        MPI_Finalize();
        exit(1);
    }

    // *************************************
    //   Generate base dataset (3D array)
    // *************************************

    int *** testVector;
    int *newGlobalvector;
    int *Global_modifvector;
    int *** newModif;
    
    if (myRank == MASTER) {
        // Assign values
        testVector  = int_vector_3D(nx, ny, nz);
        printf ("Check data within 3D vector nx:%d, ny:%d, nz:%d...\n",nx,ny,nz);
        print3DVector(testVector,nx, ny, nz); 

        // Resize to 1D vector
        newGlobalvector = vector3D_2_1D(testVector,nx, ny, nz);
        if (Verbose) { printf ("Check mappint to 1D vector. Size:%d...\n",matrix_size); }
        if (Verbose_matrix)  { print1DVector(newGlobalvector,matrix_size);  }

        Global_modifvector = new int[matrix_size];
    }
    
    // *************************************
    //   Prepare 3d array domain decomposition
    // *************************************

    int sub_nx = nx / numProcs;                  // Domain subdivision
    int sub_size = matrix_size / numProcs;       // Domain subdivision


    // *************************************
    //   Scatter dataset to each processor
    // *************************************

    if (Verbose) printf("Before scatter-rank:%d, size subvector:%d\n",myRank,sub_size);

    int *newlocalvector = new int[sub_size];

    //scattering array a from MASTER node out to the other nodes
    ierr = MPI_Scatter(newGlobalvector, sub_size, MPI_INT, newlocalvector, sub_size, 
            MPI_INT, MASTER, MPI_COMM_WORLD); 
        
    if (Verbose) {
        printf ("Scatter portion of vector in rank:%d  size:%d\n",myRank,sub_size);
        print1DVector(newlocalvector,sub_size);
    }

    if (Verbose) 
        printf("After  scatter-rank:%d, size subvector:%d, size 3D:%d,%d,%d ierr=%d\n",myRank,
            sub_size,nx,ny,nz,ierr);

    int *** subVector = vector1D_2_3D(newlocalvector,sub_size,sub_nx,ny,nz);

    if (Verbose) { printf ("Scatter subvector in 3D. rank:%d\n",myRank); }
    if (Verbose_matrix) { print3DVector(subVector,sub_nx, ny, nz); }
    
    // *************************************
    //   Apply transformation to 3D subVector
    // *************************************

    int *** modif_subVector = change_vector_3D(subVector,sub_nx,ny,nz,factor);
    if (Verbose) {
        printf ("Rank:%d  Check result of apply a factor (%d) within 3D subvector...\n",myRank,factor);        
    }

    if (Verbose_matrix) { print3DVector(modif_subVector,sub_nx, ny, nz);}

    int *new_modif_subVector = vector3D_2_1D(modif_subVector,sub_nx, ny, nz);
    
    // *************************************
    //   Gather dataset from new_modif_subVector to Global_modifvector
    // *************************************

    if (Verbose) printf("Before gather-rank:%d, size subvector:%d\n",myRank,sub_size);

    //gathering array a from slave nodes
    ierr = MPI_Gather(new_modif_subVector, sub_size, MPI_INT, Global_modifvector, sub_size, 
            MPI_INT, MASTER, MPI_COMM_WORLD); 

    if (Verbose) printf("After  gather-rank:%d ierr=%d\n",myRank,ierr);

    MPI_Barrier(MPI_COMM_WORLD);

    // *************************************
    //   Show results
    // *************************************

    if (myRank == MASTER) {
        print1DVector(Global_modifvector,matrix_size);
        newModif = vector1D_2_3D(Global_modifvector,matrix_size,nx,ny,nz);
        printf ("Result after modification within 3D vector nx:%d, ny:%d, nz:%d...\n",nx,ny,nz);        
        print3DVector(newModif,nx, ny, nz); 
    }    

    MPI_Barrier(MPI_COMM_WORLD);

    // *************************************
    //   Clean-up
    // *************************************

    if (myRank == MASTER) {
        dealloc_3d_int(testVector, nx, ny, nz);
        dealloc_3d_int(newModif, nx, ny, nz);
        delete [] newGlobalvector;
        delete [] Global_modifvector;
    }
    dealloc_3d_int(subVector, sub_nx, ny, nz);
    dealloc_3d_int(modif_subVector, sub_nx, ny, nz);
    delete [] newlocalvector;
    delete [] new_modif_subVector;

    MPI_Finalize();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;

    if (myRank == MASTER)
        printf ("Done: took %ld ms to run.\n",time/std::chrono::milliseconds(1));

}

// Create 1D vector from 3D vector and map internal values
// ----------------------------------------------------------------------------
int *vector3D_2_1D(int *** in_vec, int nx, int ny, int nz) {
    
    int matrix_size = nx*ny*nz;
    int *out_vec = new int[matrix_size];
    
    //map elements
    for(int x = 0; x < nx; x++) {
        for(int y = 0; y < ny; y++)
            for(int z = 0; z < nz; z++) {
                int idx = z+y*nz+x*ny*nz;
                out_vec[idx] = in_vec[x][y][z];
            }
    }
    
    return out_vec;
}

// Create 3D vector from 1D vector and map internal values
// ----------------------------------------------------------------------------
int ***vector1D_2_3D(int *in_vec, int size1D, int nx, int ny, int nz) {
    
    int matrix_size = nx*ny*nz;
    if (size1D != matrix_size) {
        printf ("Check conversion 1D to 3D sizes. 1D size=%d   3D size=%d\n",size1D,matrix_size);
        exit(1);
    }
    
    int ***out_vec = alloc_3d_int(nx,ny,nz);

    //map elements
    
    for(int x = 0; x < nx; x++) {
        for(int y = 0; y < ny; y++)
            for(int z = 0; z < nz; z++) {
                int idx = z+y*nz+x*ny*nz;
                out_vec[x][y][z] = in_vec[idx];
            }
    }
    
    return out_vec;
}

// Create 3D vector of size nx,ny,nz, and assign values regarding x,y location
// ----------------------------------------------------------------------------
int ***int_vector_3D(int nx, int ny, int nz) {

    int *** vec = alloc_3d_int(nx,ny,nz);

    // Assign values. 
    //  1) Constant values in z
    //  2) Value corresponds to y+x*ny
    
    for(int x = 0; x < nx; x++) {
        for(int y = 0; y < ny; y++)
            for(int z = 0; z < nz; z++) 
                vec[x][y][z] = y+x*ny;
    }
          
    return vec;    
}

// Operate over 3D vector of size nx,ny,nz, using a factor
// ----------------------------------------------------------------------------
int ***change_vector_3D(int *** inVector, int nx, int ny, int nz, int factor) {

    // Define 3D int multi-array 
    int *** outVec = alloc_3d_int(nx,ny,nz);

    // Assign values    
    for(int x = 0; x < nx; x++)   {
        for(int y = 0; y < ny; y++)
            for(int z = 0; z < nz; z++)
                outVec[x][y][z] = inVector[x][y][z] * factor;
    }
          
    return outVec;    
}

// allocate 3D array
// ----------------------------------------------------------------------------
int ***alloc_3d_int(int nx, int ny, int nz) {

    int*** new_array = new int**[nx];
 
    for (int i = 0; i < nx; i++) {
        new_array[i] = new int*[ny];
        for (int j = 0; j < ny; j++)
            new_array[i][j] = new int[nz];
    }

    return new_array;
}

// deallocate 3D array
// ----------------------------------------------------------------------------
void dealloc_3d_int(int ***A, int nx, int ny, int nz) {

    for (int i = 0; i < nx; i++)     {
        for (int j = 0; j < ny; j++)
            delete[] A[i][j];
        delete[] A[i];
    }
 
    delete[] A;
}

// Print 3D vector values
// ----------------------------------------------------------------------------
void print3DVector(int *** vec, int nx, int ny, int nz) {
    
    for(int x = 0; x < nx; x++)
    {
        for(int y = 0; y < ny; y++)
            for(int z = 0; z < nz; z++)
                printf("x:%d, y:%d, z:%d, Value:%d \n",x,y,z,vec[x][y][z]);
        printf("\n");
    }
}

// Print 3D vector values
// ----------------------------------------------------------------------------
void print1DVector(int * vec, int n) {
    
    for(int i = 0; i < n; i++)
        printf("%d  ",vec[i]);
    printf("\n");
}