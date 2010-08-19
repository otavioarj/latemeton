#include<stdlib.h>


int compare(const void * a, const void * b)
{
  return (int )a - (int)b;
}

int slope(double in_data[], double in_len, double *out_Q, double *out_b)
{
   int i=0;
   int j=0;
   int k=0; //Q index

   double sumXY=0;
   double sumX=0;
   double sumY=0;
   double sumX2=0; 
   double out_a; //For RLS

   int N=0;

   double *Q;


   if(in_len<=2) //Erro: in_len<=2
      return -2;

   N=in_len*((in_len-1)/2); //Calculate the value of N

   /* Allocate memory for double array Q */
   if( (Q = malloc(N * sizeof(double))) == NULL ) {
      printf("Out of memory\n");
      return -1;
   } 

   /* Calculate the value of Q[k] */
   for(i=0;i<in_len-1;i++)
   {
      for(j=i+1;j<in_len;j++)
         Q[k++]=(in_data[j]-(i+1))/(j-i);
   }

//   quicksort(Q, 0, N-1); //Sort array Q
   qsort(Q,N,sizeof(double),compare);

   
   if(N%2==0) //Checks if the number of elements in Q is even
      *out_Q = (Q[(N/2)-1]+Q[N/2])/2.0;
   else
      *out_Q = Q[N/2];

   /* Calculing RLS param b */
   for(i=0;i<in_len;i++)
   {
      sumX  = sumX  + i+1;
      sumY  = sumY  + in_data[i];
      sumXY = sumXY + (i+1)*(in_data[i]);
      sumX2 = sumX2 + ((i+1)*(i+1));
   }
   out_a = (sumXY-(sumX*sumY) / in_len) / (sumX2-(sumX*sumX) / in_len);
   *out_b = (sumY -out_a*sumX) / in_len;

   free(Q);

   return 0;
} 


