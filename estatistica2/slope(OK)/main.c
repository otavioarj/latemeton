#include <stdio.h>
#include "slope.c"

int main(int argc, char *argv[])
{
   double Q, b;
   double tmp;

   int len = 24;
   int i=0;
   int result=-1;
   int dif=0;
   double MAPE=0;
   double in_data[24][2]= {01.0, 696.000000,
02.0, 479.000000,
03.0, 464.000000,
04.0, 594.000000,
05.0, 287.000000,
06.0, 197.000000,
07.0, 906.000000,
08.0, 660.000000,
09.0, 632.000000,
10.0, 604.000000,
11.0, 612.000000,
12.0, 248.000000,
13.0, 255.000000,
14.0, 598.000000,
15.0, 698.000000,
16.0, 808.000000,
17.0, 641.000000,
18.0, 514.000000,
19.0, 238.000000,
20.0, 135.000000,
21.0, 821.000000,
22.0, 624.000000,
23.0, 217.000000,
24.0, 159.000000};


   result=slope(in_data, len, &Q, &b);

   printf("Valor Q: %lf  Valor b: %lf\n",Q,b);

   for(i=0;i<len;i++)
   {
      tmp = Q*(in_data[i][0]) + b;
      dif=tmp-in_data[i][1];
      if(dif<0)
         dif=-dif;

      MAPE=MAPE+(dif/in_data[i][1]);
      printf("%lf\n",tmp); 
   }
   MAPE=MAPE/len;
   printf("\nMAPE= %lf (per cent)     Acuracidade= %lf (per cent) \n",MAPE*100,(1-MAPE)*100);
}
