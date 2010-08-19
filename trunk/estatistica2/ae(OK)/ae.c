int ae(double *in_data, int in_time, double *out_data, float alfa)
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