
#include "../../include/Server/matrix.h"

#define ROWS 30
#define COLS 40

// Function prototypes
void printMatrix(double complex mat[ROWS][COLS], int rows, int cols);
void transpose(double complex src[ROWS][COLS], double complex dest[COLS][ROWS], int rows, int cols);
void multiply(double complex mat1[][COLS], double complex mat2[][ROWS], double complex result[][ROWS], int rows1, int cols1, int rows2, int cols2);
void invert(double complex mat[COLS][COLS], double complex inv[COLS][COLS], int n);
void calculatePseudoInverse(double complex A[ROWS][COLS], double complex A_pseudo[COLS][ROWS]);

double getTime() {
    // Initialize a 30x40 complex matrix
    double complex A[ROWS][COLS];
    srand(time(NULL));
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            A[i][j] = rand() % 10 + rand() % 10 * I;
        }
    }

    double complex A_pseudo[COLS][ROWS];
    clock_t start, end;
    double cpu_time_used;

    // Measure the time taken to calculate the pseudo-inverse
    start = clock();
    calculatePseudoInverse(A, A_pseudo);
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Time taken to compute the pseudo-inverse: %f seconds\n", cpu_time_used);

    return cpu_time_used;
}

// Function to print a complex matrix
void printMatrix(double complex mat[ROWS][COLS], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("(%f, %f) ", creal(mat[i][j]), cimag(mat[i][j]));
        }
        printf("\n");
    }
}

// Function to transpose a matrix
void transpose(double complex src[ROWS][COLS], double complex dest[COLS][ROWS], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            dest[j][i] = conj(src[i][j]);
        }
    }
}

// Function to multiply two matrices
void multiply(double complex mat1[][COLS], double complex mat2[][ROWS], double complex result[][ROWS], int rows1, int cols1, int rows2, int cols2) {
    // Ensure result is initialized to 0
    for (int i = 0; i < rows1; i++) {
        for (int j = 0; j < cols2; j++) {
            result[i][j] = 0.0 + 0.0 * I;
            for (int k = 0; k < cols1; k++) {
                result[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
}

// Function to invert a matrix using Gaussian elimination
void invert(double complex mat[COLS][COLS], double complex inv[COLS][COLS], int n) {
    // Create an augmented matrix [mat|I]
    double complex augmented[COLS][2*COLS];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            augmented[i][j] = mat[i][j];
            augmented[i][j+n] = (i == j) ? 1.0 + 0.0 * I : 0.0 + 0.0 * I;
        }
    }

    // Perform Gaussian elimination
    for (int i = 0; i < n; i++) {
        double complex pivot = augmented[i][i];
        for (int j = 0; j < 2*n; j++) {
            augmented[i][j] /= pivot;
        }
        for (int k = 0; k < n; k++) {
            if (k != i) {
                double complex factor = augmented[k][i];
                for (int j = 0; j < 2*n; j++) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }

    // Extract the inverse matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inv[i][j] = augmented[i][j+n];
        }
    }
}

// Function to calculate the pseudo-inverse of a matrix
void calculatePseudoInverse(double complex A[ROWS][COLS], double complex A_pseudo[COLS][ROWS]) {
    double complex A_H[COLS][ROWS];
    double complex A_H_A[COLS][COLS];
    double complex A_H_A_inv[COLS][COLS];

    // Compute A^H (Hermitian transpose of A)
    transpose(A, A_H, ROWS, COLS);

    // Compute A^H * A
    multiply(A_H, A, A_H_A, COLS, ROWS, ROWS, COLS);

    // Compute (A^H * A)^-1
    invert(A_H_A, A_H_A_inv, COLS);

    // Compute pseudo-inverse: A^+ = (A^H * A)^-1 * A^H
    multiply(A_H_A_inv, A_H, A_pseudo, COLS, COLS, COLS, ROWS);
}