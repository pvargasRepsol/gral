# ---------------------------------------------------------------------------
#  (c)   Copyright  REPSOL S.A.  All Right reserved
# 
#  parallel_ToyExample.cpp
# 
#  Created on:   Feb 2021
#      Author:   Pablo E Vargas - pabloenrique.vargas@repsol.com
# 
#    Makefile to compile using MPICH
# ---------------------------------------------------------------------------

CC=mpicxx
override CFLAGS +=-O3 -std=c++11
LIBS=-lmpi -lm -lstdc++

all: parallel_ToyExample.cpp
	$(CC) $(CFLAGS) $< -o parallel_ToyExample $(LIBS)

clean: 
	rm -f parallel_ToyExample
