#include<stdlib.h>

#define less(A,B) (A<B)
#define exch(A,B) {double t=A; A=B; B=t;}

void quicksort(double *Q, int l, int r);
int partition(double *Q, int l, int r);

int slope(double in_data[][2], double in_len, double *out_Q, double *out_b)
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
      {
         Q[k]=(in_data[j][1]-in_data[i][1])/(j-i);
         k++;
      }
   }

   quicksort(Q, 0, N-1); //Sort array Q

   
   if(N%2==0) //Checks if the number of elements in Q is even
      *out_Q = (Q[(N/2)-1]+Q[N/2])/2.0;
   else
      *out_Q = Q[N/2];

   /* Calculing RLS param b */
   for(i=0;i<in_len;i++)
   {
      sumX  = sumX  + in_data[i][0];
      sumY  = sumY  + in_data[i][1];
      sumXY = sumXY + (in_data[i][0])*(in_data[i][1]);
      sumX2 = sumX2 + (in_data[i][0])*(in_data[i][0]);
   }
   out_a = (sumXY-(sumX*sumY) / in_len) / (sumX2-(sumX*sumX) / in_len);
   *out_b = (sumY -out_a*sumX) / in_len;

   free(Q);

   return 0;
} 


void quicksort(double *Q, int l, int r)
{
   int i;
   
   if(r<=l)
      return;

   i=partition(Q,l,r);
   quicksort(Q,l,i-1);
   quicksort(Q,i+1,r);
}

int partition(double *Q, int l, int r)
{
   int i;
   int j;
   double v;

   i=l-1;
   j=r;
   v=Q[r];

   for(;;)
   {
      while(less(Q[++i],v));

      while(less(v,Q[--j]))
         if(j==l)
            break;

      if(i>=j)
         break;
      exch(Q[i],Q[j]);
   }
   exch(Q[i],Q[r]);
   return i;
}