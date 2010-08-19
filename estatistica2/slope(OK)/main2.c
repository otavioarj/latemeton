#include <stdio.h>
#include "slope.h"

int main(int argc, char *argv[])
{
   double Q, b;
   double tmp;

   int len = 24;
   int i=0;
   int result=-1;
   int dif=0;
   double MAPE=0;
   double in_data[24]= { 696.000000, 479.000000, 464.000000, 594.000000, 287.000000, 197.000000, 906.000000, 660.000000, 632.000000, 604.000000, 612.000000, 248.000000, 255.000000, 598.000000, 698.000000, 808.000000, 641.000000,514.000000,238.000000,135.000000,821.000000,624.000000,217.000000,159.000000};

   result=slope(in_data, len, &Q, &b);

   printf("Valor Q: %lf  Valor b: %lf\n",Q,b);
  // Q=-5.410714; 
   for(i=0;i<len;i++)
   {
      tmp = Q*(i+1) + b;
      dif=tmp-in_data[i];
      if(dif<0)
         dif=-dif;

      MAPE=MAPE+(dif/in_data[i]);
      printf("%lf\n",tmp); 
   }
   MAPE=MAPE/len;
   printf("\nMAPE= %lf (per cent)     Acuracidade= %lf (per cent) \n",MAPE*100,(1-MAPE)*100);
}
