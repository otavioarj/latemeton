#include <stdio.h>
#include "naive_set.c"

int main(int argc, char *argv[])
{
   double out_data;
   int i=0;
   int result=-1;
   int dif=0;
   double MAPE=0;
   for(i=0;i<len;i++)
   {
      result=naive_set(in_data,i,&out_data);
      dif=out_data-in_data[i];
      if(dif<0)
         dif=-dif;

      MAPE=MAPE+(dif/in_data[i]);
      printf("%lf\n",out_data);
   }
   MAPE=MAPE/len;
   printf("\nMAPE= %lf (per cent)     Acuracidade= %lf (per cent) \n",MAPE*100,(1-MAPE)*100);
}
