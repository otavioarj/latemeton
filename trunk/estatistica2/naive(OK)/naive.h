#include <stdio.h>

int _naive( double *in_data, int in_time, double *out_data)
{
   if(in_time==0) //Used on the first forecast
   {
      *out_data=in_data[0];
      return 0;
   }
   else if(in_time<0) //Erro: negative time
      return -1;

   *out_data=in_data[in_time-1];
   return 0;
}


double naive(double *in_data,int len)
{
   double *out_data;
   int i=0,result,dif=0,MAPE=0;
   for(i=0;i<len;i++)
		{
      if((result=_naive(in_data,i,out_data[i]))!=0)
				return (double)resu;
      dif=out_data[i]-in_data[i];
      if(dif<0)
         dif=-dif;
      MAPE=MAPE+(dif/in_data[i]);
 		}
   return MAPE/len;
}
