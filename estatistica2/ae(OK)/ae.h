
int _ae(double *in_data, int in_time, double *out_data, float alfa)
{
   double St_1=0; //Stores value of S(t-1)
   double St=0; //Stores value of S(t)
   
   int i=0;

   if(in_time==0) //Used on the first forecast
   {
      *out_data=in_data[0]; 
      return 0;
   }

   else if(in_time<0) //Erro: negative time
      return -1;

   for(i=0;i<=in_time;i++)
   {
      if(i==0)
         St_1=in_data[0];
      else
      {
         St = alfa*in_data[i-1] + (1.0-alfa)*St_1;
         St_1=St; 
      }
   }
   *out_data=St;
   return 0;
}

double ae(double * in_data,double *out_data, int len)
{
	 int i=0, dif=0;
   double MAPE=0;
   for(i=0;i<len;i++)
		{
      _ae(in_data, i,out_data[i], 0.9);
      dif=out_data[i]-in_data[i];
      if(dif<0)
        dif=-dif;
      MAPE=MAPE+(dif/in_data[i]);
    }
  return MAPE/len;
}
