#ifndef MATRIX_H
#define MATRIX_H

#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <time.h>

#define ROWS 40
#define COLS 40

// Function prototypes
void printMatrix(double complex mat[ROWS][COLS], int rows, int cols);
void transpose(double complex src[ROWS][COLS], double complex dest[COLS][ROWS], int rows, int cols);
void multiply(double complex mat1[][ROWS], double complex mat2[][COLS], double complex result[][COLS], int rows1, int cols1, int cols2);
void invert(double complex mat[COLS][COLS], double complex inv[COLS][COLS], int n);
void calculatePseudoInverse(double complex A[ROWS][COLS], double complex A_pseudo[COLS][ROWS]);
double getTime();

#endif
