#include <stdio.h>
#include "ae.c"

int main(int argc, char *argv[])
{
   double out_data;

   int len = 59;
   int i=0;
   float k=0.0;
   int result=-1;
   int dif=0;
   double MAPE=0;

   //double in_data[20]={4695, 2899, 4008, 6301, 1617, 2023, 2494, 5096, 3784, 5394, 5154, 5967, 4603, 4468, 4361, 2900, 4505, 4084, 3325, 5311};

   double in_data[59]= {1577.0,
762.0,
940.0,
1250.0,
1041.0,
1346.0,
1241.0,
2270.0,
2703.0,
2868.0,
1288.0,
1386.0,
1207.0,
1263.0,
1156.0,
2128.0,
3783.0,
2427.0,
1638.0,
1360.0,
834.0,
797.0,
354.0,
1605.0,
949.0,
877.0,
1285.0,
1691.0,
2288.0,
4624.0,
3638.0,
3825.0,
1841.0,
2885.0,
3630.0,
3846.0,
2549.0,
2161.0,
1797.0,
1423.0,
1653.0,
1506.0,
1103.0,
1219.0,
1259.0,
1063.0,
1641.0,
1087.0,
909.0,
1113.0,
1254.0,
1638.0,
2727.0,
2871.0,
2706.0,
2912.0,
2471.0,
2984.0,
1998.0};
   
   //for(k=1;k<10;k=k+0.5)
   //{
        //printf("\nValor de alfa: %f\n",k/10.0);
        for(i=0;i<len;i++)
        {
            //result = ae(in_data, i, &out_data, k/10.0);
            result = ae(in_data, i, &out_data, 0.9);
            dif=out_data-in_data[i];
            if(dif<0)
                dif=-dif;
        
            MAPE=MAPE+(dif/in_data[i]);
            printf("%lf\n",out_data);
        }
        MAPE=MAPE/len;
        printf("\nMAPE= %lf (per cent)     Acuracidade= %lf (per cent) \n",MAPE*100,(1-MAPE)*100);
   //}
}