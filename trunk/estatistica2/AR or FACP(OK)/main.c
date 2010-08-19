#include <stdio.h>
#include "ar.h"
#include <math.h>

void imprime_matrix(MATRIX *matrix);

int main(int argc, char *argv[])
{
    int i, k, l, j;
    MATRIX *mat;
    double parametros[7];
    int len=59;
    double IC=2.0/sqrt(len);
    double *estimativas;

   int dif=0;
   double MAPE=0;   
   estimativas=calloc(sizeof(double)*len);        
   double vet[59]= 
    //faz AR(1) ao AR(8) pra pegar os parametros...
    for(j=1;j<8;j++)
     {
       mat=ar(vet, len, j);
       parametros[j-1]=(*mat).matrix[(*mat).m-1][0];
     }   
           
    for(i=0;i<7;i++)
        if(parametros[i]<IC)
            break;    
    mat=ar(vet, len, i);
    
    for(k=0;k<len;k++)
    {
        estimativas[k]=(*mat).matrix[0][0];
        for(l=1;l<=i;l++)
        {
            if((k-l)>=0)
                estimativas[k]=estimativas[k]+(*mat).matrix[l][0]*vet[k-l];
        }

    }

    for(i=0;i<len;i++)
    {
       dif=estimativas[i]-vet[i];
       if(dif<0)
          dif=-dif;
       printf("%lf\n",estimativas[i]);
       MAPE=MAPE+(dif/vet[i]);
       
    }
   MAPE=MAPE/len;
    return 0;

} 

