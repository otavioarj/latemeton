int naive( double *in_data, int in_time, double *out_data)
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

