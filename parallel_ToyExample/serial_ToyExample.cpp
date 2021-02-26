//  ---------------------------------------------------------------------------
//  (c)   Copyright  REPSOL S.A.  All Right reserved
// 
//  serial_Example_array.cpp
// 
//  Created on:   Feb 2021
//      Author:   Pablo E Vargas - pabloenrique.vargas@repsol.com
// 
//  Generate a 3D integer array and operate it using
//  factor multiplication.
// 
//  ---------------------------------------------------------------------------

# include <stdio.h>
# include <cstdlib>

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

    if (argc != 5) {
        fprintf (stderr,"\n\n ----> Error! Requires at lest 4 int args: nx ny nz factor\n\n");
        exit(1);
    }
        
    int nx = atoi(argv[1]);
    int ny = atoi(argv[2]);
    int nz = atoi(argv[3]);
    int factor = atoi(argv[4]);
    int matrix_size = nx*ny*nz;
    
    int *** testVector  = int_vector_3D(nx, ny, nz);

    printf ("Check data within 3D vector nx:%d, ny:%d, nz:%d...\n",nx,ny,nz);
    print3DVector(testVector,nx, ny, nz); 

    int *newGlobalvector = vector3D_2_1D(testVector,nx, ny, nz);
    printf ("Check data within 1D vector size:%d...\n",matrix_size);
    print1DVector(newGlobalvector,matrix_size); 
    
    int *** modifVector = change_vector_3D(testVector,nx,ny,nz,factor);
    
    printf ("Check result of apply a factor (%d) within 3D vector...\n",factor);
    print3DVector(modifVector,nx, ny, nz); 

    int *newGlobalmodifvector = vector3D_2_1D(modifVector,nx, ny, nz);
    printf ("Check data within 1D vector size:%d...\n",matrix_size);
    print1DVector(newGlobalmodifvector,matrix_size); 
    
    int *** newModif = vector1D_2_3D(newGlobalmodifvector,matrix_size,nx,ny,nz);
    printf ("Check data within 3D vector nx:%d, ny:%d, nz:%d...\n",nx,ny,nz);
    print3DVector(newModif,nx, ny, nz); 
    

    // Clean-up
    dealloc_3d_int(testVector, nx, ny, nz);
    dealloc_3d_int(modifVector, nx, ny, nz);
    dealloc_3d_int(newModif, nx, ny, nz);
    delete [] newGlobalvector;
    delete [] newGlobalmodifvector;

    printf ("Done\n");
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