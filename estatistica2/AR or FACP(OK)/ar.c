#include "matrix.h"

MATRIX *ar(double *vet, int len, int lag)
{
    int i, j;

    MATRIX *B;
    MATRIX *Y;
    MATRIX *X;
    MATRIX *X_t;
    MATRIX *RESULT1;
    MATRIX *RESULT2;

    if((Y=cria_matrix(len-lag,1))==NULL)
    {
        printf("Erro: NULL pointer...\n");
        exit(1);
    }
    if((X=cria_matrix(len-lag,(lag+1)))==NULL)
    {
        printf("Erro: NULL pointer...\n");
        exit(1);
    }

    for(i=0;i<len-lag;i++)
    {
        (*Y).matrix[i][0]=vet[i+lag];
    }

    for(i=0;i<len-lag;i++)
    {
        for(j=0;j<=lag;j++)
        {
	    if(j==0)
	        (*X).matrix[i][0]=1.0;
            else
                (*X).matrix[i][j]=vet[i+(lag-j)];
        }
    }

    X_t=transposta_matrix(X);

    RESULT1=multiplica_matrix(X_t,X);


    RESULT1=inversa_matrix(RESULT1);

    RESULT2=multiplica_matrix(X_t,Y);

    B=multiplica_matrix(RESULT1,RESULT2);
    imprime_matrix(B);
    printf("\n");

    return B;

} 
//==========================================================
//==========================================================
