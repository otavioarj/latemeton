#include <stdio.h>
#include "naive.c"

int main(int argc, char *argv[])
{
   double out_data;

   int len = 91;
   int i=0;
   int result=-1;
   int dif=0;
   double MAPE=0;

   double in_data[91]={696.0,
479.0,
464.0,
594.0,
287.0,
197.0,
906.0,
660.0,
632.0,
604.0,
612.0,
248.0,
255.0,
598.0,
698.0,
808.0,
641.0,
514.0,
238.0,
135.0,
821.0,
624.0,
217.0,
159.0,
167.0,
210.0,
154.0,
961.0,
909.0,
757.0,
837.0,
236.0,
325.0,
88.0,
924.0,
571.0,
744.0,
233.0,
530.0,
124.0,
70.0,
373.0,
355.0,
2031.0,
353.0,
644.0,
54.0,
88.0,
963.0,
962.0,
580.0,
21.0,
29.0,
9.0,
24.0,
871.0,
609.0,
1340.0,
2131.0,
1379.0,
226.0,
35.0,
925.0,
866.0,
820.0,
974.0,
618.0,
123.0,
56.0,
755.0,
730.0,
634.0,
696.0,
672.0,
145.0,
85.0,
1020.0,
906.0,
479.0,
1088.0,
992.0,
171.0,
124.0,
703.0,
801.0,
340.0,
40.0,
32.0,
43.0,
487.0,
262.0};


   for(i=0;i<len;i++)
   {
      result=naive(in_data,i,&out_data);
      dif=out_data-in_data[i];
      if(dif<0)
         dif=-dif;

      MAPE=MAPE+(dif/in_data[i]);
      printf("%lf\n",out_data);
   }
   MAPE=MAPE/len;
   printf("\nMAPE= %lf (per cent)     Acuracidade= %lf (per cent) \n",MAPE*100,(1-MAPE)*100);
}