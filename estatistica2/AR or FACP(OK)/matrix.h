#include <stdio.h>
#include <stdlib.h>

#ifndef _MATRIX_H_

#define _MATRIX_H_

typedef struct{
    int m;
    int n;
    double **matrix;
    }MATRIX;

MATRIX *cria_matrix(int m, int n);
MATRIX *transposta_matrix(MATRIX *matrix);
MATRIX *inversa_matrix(MATRIX *matrix);
MATRIX *identidade_matrix(int t);
double expon(double number, double exponencial);
double determinante_matrix(MATRIX *mat);
MATRIX *soma_matrix(MATRIX *matrix1, MATRIX *matrix2);
MATRIX *multiplica_matrix(MATRIX *matrix1, MATRIX *matrix2);
MATRIX *cofator_matrix(MATRIX *matrix);
MATRIX *const_multiplica_matrix(double constante, MATRIX *matrix);
void imprime_matrix(MATRIX *matrix);

#endif
