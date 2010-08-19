#include <stdio.h>
#include "slope.c"

int main(int argc, char *argv[])
{
   double Q, b;
   double tmp;

   int len = 91;
   int i=0;
   int result=-1;
   int dif=0;
   double MAPE=0;
   double in_data[91][2]= {01.0, 696.000000,
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
24.0, 159.000000,
25.0, 167.000000,
26.0, 210.000000,
27.0, 154.000000,
28.0, 961.000000,
29.0, 909.000000,
30.0, 757.000000,
31.0, 837.000000,
32.0, 236.000000,
33.0, 325.000000,
34.0, 88.000000,
35.0, 924.000000,
36.0, 571.000000,
37.0, 744.000000,
38.0, 233.000000,
39.0, 530.000000,
40.0, 124.000000,
41.0, 70.000000,
42.0, 373.000000,
43.0, 355.000000,
44.0, 2031.000000,
45.0, 353.000000,
46.0, 644.000000,
47.0, 54.000000,
48.0, 88.000000,
49.0, 963.000000,
50.0, 962.000000,
51.0, 580.000000,
52.0, 21.000000,
53.0, 29.000000,
54.0, 9.000000,
55.0, 24.000000,
56.0, 871.000000,
57.0, 609.000000,
58.0, 1340.000000,
59.0, 2131.000000,
60.0, 1379.000000,
61.0, 226.000000,
62.0, 35.000000,
63.0, 925.000000,
64.0, 866.000000,
65.0, 820.000000,
66.0, 974.000000,
67.0, 618.000000,
68.0, 123.000000,
69.0, 56.000000,
70.0, 755.000000,
71.0, 730.000000,
72.0, 634.000000,
73.0, 696.000000,
74.0, 672.000000,
75.0, 145.000000,
76.0, 85.000000,
77.0, 1020.000000,
78.0, 906.000000,
79.0, 479.000000,
80.0, 1088.000000,
81.0, 992.000000,
82.0, 171.000000,
83.0, 124.000000,
84.0, 703.000000,
85.0, 801.000000,
86.0, 340.000000,
87.0, 40.000000,
88.0, 32.000000,
89.0, 43.000000,
90.0, 487.000000,
91.0, 262.000000};

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