#include <stdio.h>
#include "ar.h"
#include <math.h>

void imprime_matrix(MATRIX *matrix);

int main(int argc, char *argv[])
{
    int i, k, l, j;
    MATRIX *mat;
    double parametros[7];
    int len=24;
    double IC=2.0/sqrt(len);
    double estimativas[24];

   int dif=0;
   double MAPE=0;
   
    for(i=0;i<len;i++)
        estimativas[i]=0.0;
        
   double vet[24]= {3446.0, 3028.0, 1268.0, 4412.0, 3305.0,
                    3046.0, 2195.0, 2441.0, 3286.0, 7981.0,
		    5788.0, 7175.0, 3190.0, 5874.0, 6828.0,
		    9137.0, 6507.0, 6562.0, 5981.0, 5937.0,
	            5297.0, 4525.0, 5336.0, 4727.0};
    /*
    while (lag<=0)
    {
	printf("lag: (>=1) ");
        scanf("%d",&lag);
    }*/
    //faz AR(1) ao AR(8) pra pegar os parametros...
    for(j=1;j<8;j++)
    {
        mat=ar(vet, len, j);
        parametros[j-1]=(*mat).matrix[(*mat).m-1][0];
    }
    
    printf("PARAMETROS: \n");
    for(j=0;j<7;j++)
        printf("(%d): %lf\n",j,parametros[j]);
        
    for(i=0;i<7;i++)
        if(parametros[i]<IC)
        {
            printf("\nIC: %lf\nTruncada em: AR(%d)\n",IC,i);
            break;
        }
    
    mat=ar(vet, len, i);
    for(k=0;k<len;k++)
    {
        for(l=1;l<=i;l++)
        {
            estimativas[k]=(*mat).matrix[0][0];
            if((k-l)>=0)
                estimativas[k]=estimativas[k]+(*mat).matrix[l][0]*estimativas[k-l];
        }
    }
    
    for(i=0;i<len;i++)
    {
       dif=estimativas[i]-vet[i];
       if(dif<0)
          dif=-dif;

       MAPE=MAPE+(dif/vet[i]);
       printf("REAL: %lf    AR: %lf      ERRO: %lf\n",vet[i],estimativas[i],estimativas[i]-vet[i]);
    }
   MAPE=MAPE/len;
   printf("\nMAPE= %lf (per cent)     Acuracidade= %lf (per cent) \n",MAPE*100,(1-MAPE)*100);
    return 0;

} 
//==========================================================
//==========================================================

