## Parallel Toy example to test azure-batch



Start by updating the packages list:

    sudo apt update
    sudo apt upgrade


**Ubuntu version**

    lsb_release -a

    No LSB modules are available.
    Distributor ID: Ubuntu
    Description:    Ubuntu 16.04.7 LTS
    Release:        16.04
    Codename:       xenial

**C compiler**

Install the build-essential package by typing:

    sudo apt install build-essential

The command installs a bunch of new packages including gcc, g++ and make.

You may also want to install the manual pages about using GNU/Linux for development:

    sudo apt-get install manpages-dev

To validate that the GCC compiler is successfully installed, use the gcc --version command which prints the GCC version:

    gcc --version
    The default version of GCC available in the Ubuntu 16.04 repositories is 5.4.0:

    gcc (Ubuntu 5.4.0-6ubuntu1~16.04.12) 5.4.0 20160609
    Copyright (C) 2015 Free Software Foundation, Inc.
    This is free software; see the source for copying conditions.  There is NO
    warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


**MPI**

    sudo apt install openmpi-bin
    sudo apt install openmpi-common
	sudo apt install libopenmpi-dev

**Source code**

The source code generate a 3D matrix (dataset) to transform it by multiply each element by a constant value. There are two versions, the serial and the parallel one.


  * serial_ToyExample.cpp
  * parallel_ToyExample.cpp


Requires 4 arguments: 3D matrix size (nx ny nz) and  factor

* nx : size of matrix values in x
* ny : size of matrix values in y
* nz : size of matrix values in z
* factor : multiplication factor to affect all elements in the matrix.  


**Compilation**

Serial version:

    g++ serial_ToyExample.cpp -lstdc++  \
           -o serial_ToyExample.out 

Parallel version:

*chrono* is a library to track times and requires the use of c++ 11 standard (i.e. -std=c++11 in compilation options)


    mpic++ parallel_ToyExample.cpp -lstdc++ -std=c++11 \
           -o parallel_ToyExample.out 

**Execute**

Serial version:

    ./serial_ToyExample 4 2 3 2

Parallel version:
  
    mpirun -np 4 ./parallel_ToyExample.out 4 2 3 2



