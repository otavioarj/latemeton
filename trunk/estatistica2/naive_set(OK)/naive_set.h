#include <stdio.h>

int _naive_set( double *in_data, int in_time, double *out_data)
{
   if(in_time==0) //Used on the first forecast
   {
      *out_data = in_data[0];
      return 0;
   }
   else if(in_time==1) //Used on the second forecast
   {
      *out_data = in_data[1];
      return 0;
   }
   else if(in_time<0) //Erro: negative time
      return -1;

   *out_data = in_data[in_time-1] * ( 1 + ( (in_data[in_time-1] - in_data[in_time-2] )/in_data[in_time-2] ) );
   return 0;
}



double naive_set( double *in_data, int in_time, double *out_data)
		{
   double out_data;
   int i=0,result,dif=0;
	 double MAPE=0;
   for(i=0;i<len;i++)
   {
      result=_naive_set(in_data,i,&out_data);
      dif=out_data-in_data[i];
      if(dif<0)
         dif=-dif;
      MAPE=MAPE+(dif/in_data[i]);
   }
   return MAPE/len;
}
