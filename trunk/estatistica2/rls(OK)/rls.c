int rls(double in_data[], double in_len, double *out_data)
{
   double sumXY=0;
   double sumX=0;
   double sumY=0;
   double sumX2=0; 

   int i=0;

   if(in_len<=1) //Erro: in_len<=1
      return -1;
   
   for(i=0;i<in_len;i++)
   {
      sumX  = sumX  + i+1;
      sumY  = sumY  + in_data[i];
      sumXY = sumXY + (i+1)*(in_data[i]);
      sumX2 = sumX2 + ((i+1)*(i+1));
   }
   out_data =((sumXY-(sumX*sumY) / in_len) / (sumX2-(sumX*sumX) / in_len))+((sumY -(*out_b)*sumX) / in_len);
   return 0;
} 
