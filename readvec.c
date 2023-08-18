static char help[] = "Read a vector from a Matrix Market (v. 2.0) file\n\
and write it to a file in petsc binary format.  \n\
Usage:  ./ex72 -fin <infile> -fout <outfile> \n\
(See https://math.nist.gov/MatrixMarket/ for details.)\n\n";

/*
   NOTES:

   1) Matrix Market files are always 1-based, i.e. the index of the first
      element of a matrix is (1,1), not (0,0) as in C.  ADJUST THESE
      OFFSETS ACCORDINGLY offsets accordingly when reading and writing
      to files.

   2) ANSI C requires one to use the "l" format modifier when reading
      double precision floating point numbers in scanf() and
      its variants.  For example, use "%lf", "%lg", or "%le"
      when reading doubles, otherwise errors will occur.
*/
#include <petscmat.h>
#include "ex72mmio.h"

int main(int argc, char **argv)
{
  MM_typecode  matcode;
  FILE        *file;
  PetscInt     M, N, ninput;
  Vec          b;
  char         filein[PETSC_MAX_PATH_LEN], fileout[PETSC_MAX_PATH_LEN];
  PetscInt     i, j;
  PetscScalar *val;
  PetscViewer  view;
  PetscMPIInt  size;
  PetscBool    flag;

  PetscInitialize(&argc, &argv, (char *)0, help);
  PetscCallMPI(MPI_Comm_size(PETSC_COMM_WORLD, &size));
  PetscCheck(size == 1, PETSC_COMM_WORLD, PETSC_ERR_WRONG_MPI_SIZE, "This is a uniprocessor example only!");

  PetscOptionsBegin(PETSC_COMM_WORLD, NULL, "Matrix Market example options", "");
  {
    PetscCall(PetscOptionsString("-fin", "Input Matrix Market file", "", filein, filein, sizeof(filein), &flag));
    PetscCheck(flag, PETSC_COMM_SELF, PETSC_ERR_USER_INPUT, "Please use -fin <filename> to specify the input file name!");
    PetscCall(PetscOptionsString("-fout", "Output file in petsc sparse binary format", "", fileout, fileout, sizeof(fileout), &flag));
    PetscCheck(flag, PETSC_COMM_SELF, PETSC_ERR_USER_INPUT, "Please use -fout <filename> to specify the output file name!");
  }
  PetscOptionsEnd();

  /* Read in vector */
  PetscCall(PetscFOpen(PETSC_COMM_SELF, filein, "r", &file));

  PetscCheck(mm_read_banner(file, &matcode) == 0, PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Could not process Matrix Market banner.");

  /* Find out size of vector .... */
  PetscCheck(mm_read_mtx_array_size(file, &M, &N) == 0, PETSC_COMM_SELF, PETSC_ERR_ARG_WRONG, "Size of sparse matrix is wrong.");

  PetscCall(mm_write_banner(stdout, matcode));
  PetscCall(PetscPrintf(PETSC_COMM_SELF, "M: %d, N: %d\n", M, N));

  /* Reseve memory for vector */
  PetscCall(PetscMalloc1(M, &val));

  /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
  /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
  /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */
  for (i = 0; i < M; i++) {
    ninput = fscanf(file, "%lg\n", &val[i]);
    PetscCheck(ninput >= 1, PETSC_COMM_SELF, PETSC_ERR_FILE_UNEXPECTED, "Badly formatted input file and end line is %d",i);
  }
  PetscCall(PetscFClose(PETSC_COMM_SELF, file));
  PetscCall(PetscPrintf(PETSC_COMM_SELF, "Reading vector completes.\n"));

  /* Create */
  PetscCall(VecCreate(PETSC_COMM_SELF, &b));
  PetscCall(VecSetSizes(b, PETSC_DECIDE, M));
  PetscCall(VecSetType(b, VECSEQ));
  PetscCall(VecSetFromOptions(b));
  PetscCall(VecSetUp(b));

  /* Add values to the vector */
  for (j = 0; j < M; j++) PetscCall(VecSetValues(b, 1, &j, &val[j], INSERT_VALUES));
  PetscCall(VecAssemblyBegin(b));
  PetscCall(VecAssemblyEnd(b));
  

  /* Write out vector */
  PetscCall(PetscViewerBinaryOpen(PETSC_COMM_SELF, fileout, FILE_MODE_WRITE, &view));
  PetscCall(VecView(b, view));
  PetscCall(PetscViewerDestroy(&view));
  PetscCall(PetscPrintf(PETSC_COMM_SELF, "Writing matrix completes.\n"));

  PetscCall(PetscFree(val));
  PetscCall(VecDestroy(&b));
  PetscCall(PetscFinalize());
  return 0;
}

/*TEST

   build:
      requires:  !complex double !defined(PETSC_USE_64BIT_INDICES)
      depends: ex72mmio.c

   test:
      suffix: 1
      args: -fin ${wPETSC_DIR}/share/petsc/datafiles/matrices/amesos2_test_mat0.mtx -fout petscmat.aij
      output_file: output/ex72_1.out

   test:
      suffix: 2
      args: -fin ${wPETSC_DIR}/share/petsc/datafiles/matrices/LFAT5.mtx -fout petscmat.sbaij
      output_file: output/ex72_2.out

   test:
      suffix: 3
      args: -fin ${wPETSC_DIR}/share/petsc/datafiles/matrices/m_05_05_crk.mtx -fout petscmat2.aij
      output_file: output/ex72_3.out

   test:
      suffix: 4
      args: -fin ${wPETSC_DIR}/share/petsc/datafiles/matrices/amesos2_test_mat0.mtx -fout petscmat.aij -permute rcm
      output_file: output/ex72_4.out
TEST*/
