#include <math.h>
#include "matrix.h"

MATRIX * _ar(double *vet, int len, int lag)
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


double ar(double * in_data,double * out_data,int len)
{
    int i, k, l, j;
    MATRIX *mat;
    double parametros[7];
    double IC=2.0/sqrt(len);
    int dif=0;
    double MAPE=0;   
    out_data=calloc(sizeof(double)*len);        
    double in_data[59]= 
    //faz AR(1) ao AR(8) pra pegar os parametros...
    for(j=1;j<8;j++)
     {
       mat=_ar(in_data, len, j);
       parametros[j-1]=(*mat).matrix[(*mat).m-1][0];
     }   
           
    for(i=0;i<7;i++)
        if(parametros[i]<IC)
            break;    
    mat=_ar(in_data, len, i);
    
    for(k=0;k<len;k++)
    {
        out_data[k]=(*mat).matrix[0][0];
        for(l=1;l<=i;l++)
        {
            if((k-l)>=0)
                out_data[k]=out_data[k]+(*mat).matrix[l][0]*in_data[k-l];
        }

    }

    for(i=0;i<len;i++)
    {
       dif=out_data[i]-in_data[i];
       if(dif<0)
          dif=-dif;
       MAPE=MAPE+(dif/in_data[i]);       
    }
   reutrn MAPE/len;   

} 

