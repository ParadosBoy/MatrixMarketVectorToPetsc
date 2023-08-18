# MatrixMarketVectorToPetsc
This project aims to provide a tool for converting Matrix Market vector files into PETSc readable files. Petsc is a powerful numerical library designed for solving large-scale scientific and engineering problems.
PETSc provides the tool transformation matrix but not the tool transformation vector.
## Requirements
### Installing Petsc
You can check https://petsc.org/release/install/ for more details
### Copy and modify
First, copy the project files to ${PETSC_DIR}/src/mat/test. Then modify the makefile in the current directory: add readvec: readvec.o ex72mmio.o

## How to use
Same as ex72, make file and then ./readvec -fin <infile> -fout <outfile>

# Attention
The input file must be from a Matrix Market and must be a vector or array(typically ends with _b.mtx) and the output file is only used in petsc

