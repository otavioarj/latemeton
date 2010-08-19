#include "matrix.h"

/*==============================================*/
MATRIX *cria_matrix(int m, int n)
{
    int i,j;
    MATRIX *new_matrix;
    double **matrix;

    if(m<1 || n<1)
        return NULL;

    if((new_matrix=(MATRIX *)malloc(sizeof(MATRIX)))==NULL)
        return NULL;

    (*new_matrix).m=m;
    (*new_matrix).n=n;

    if((matrix=(double**)malloc(m*sizeof(double *)))==NULL)
        return NULL;

    for(i=0;i<m;i++)
        if((matrix[i]=(double*)malloc(n*sizeof(double)))==NULL)
            return NULL;

    for(i=0;i<m;i++)
        for(j=0;j<n;j++)
            matrix[i][j]=0.0;

    (*new_matrix).matrix=matrix;
    return new_matrix;
} 
/*==============================================*/
MATRIX *transposta_matrix(MATRIX *matrix)
{
    int i,j;
    MATRIX *new_matrix;

    if((new_matrix=cria_matrix((int)(*matrix).n, (int)(*matrix).m))==NULL)
        return NULL;

    for(i=0;i<(*matrix).m;i++)
        for(j=0;j<(*matrix).n;j++)
            (*new_matrix).matrix[j][i]=(*matrix).matrix[i][j];

    return new_matrix;
}
/*==============================================*/
MATRIX *identidade_matrix(int t)
{
    int i,j;
    MATRIX *new_matrix;

    if((new_matrix=cria_matrix(t,t))==NULL)
        return NULL;

    for(i=0;i<t;i++)
        for(j=0;j<t;j++)
            if(i==j)
                (*new_matrix).matrix[i][j]=1.0;

    return new_matrix;
} 
/*==============================================*/
MATRIX *inversa_matrix(MATRIX *matrix)
{
    MATRIX *new_matrix;
    MATRIX *cof_matrix;
    MATRIX *cofator_transposto_matrix;

    double determinante=0.0;
    double inverse_determinante;

    if((*matrix).m!=(*matrix).n)
        return NULL; //When number of colums are diferent of number of lines


    if((determinante=determinante_matrix(matrix))==0.0)
        return NULL; //When det(matrix)==0 the inverse doesn`t exist.

    inverse_determinante=(1.0/determinante);
    //printf("Det: %f   1-div-det: %f\n",determinante,inverse_determinante);
    if((cof_matrix=cofator_matrix(matrix))==NULL)
        return NULL;

    if((cofator_transposto_matrix=transposta_matrix(cof_matrix))==NULL)
        return NULL;

    free(cof_matrix);

    if((new_matrix=const_multiplica_matrix(inverse_determinante, cofator_transposto_matrix))==NULL)
        return NULL;

    free(cofator_transposto_matrix);

    return new_matrix;
}
/*==============================================*/
double determinante_matrix(MATRIX *mat)
{
    int i, j, l;
    MATRIX **tmp;
    int delay;
    double *tmp_return;
    double ret=0.0;

    if((tmp_return=(double*)malloc((*mat).m*sizeof(double)))==NULL)
        exit(1); 

    if((*mat).m!=(*mat).n)
        exit(1); //When number of colums are diferent of number of lines

    if((*mat).m==1) // Case the matrix m==n==1
        return (*mat).matrix[0][0];
    else if((*mat).m==2) // Case the matrix m==n==2
    {
    	/*for(i=0;i<2;i++)
    	{
        	for(j=0;j<2;j++)
            		printf("  %f",(*mat).matrix[i][j]);
        	printf("\n"); 
   	}
        printf("\n");*/
        return (*mat).matrix[0][0]*(*mat).matrix[1][1]-(*mat).matrix[1][0]*(*mat).matrix[0][1];
    }
    else
    {
        tmp=(MATRIX**)malloc((*mat).m*sizeof(MATRIX*)); // Used to: Det A = a11 A11 + a21 A21 + a31 A31
        for(l=0;l<(*mat).m;l++)
        {
            delay=0;

            if((tmp[l]=cria_matrix(((*mat).m)-1,((*mat).n)-1))==NULL)
                exit(1); 

            for(i=0;i<(*mat).m;i++) //Starts with 0 and goes to m-1
            {
                for(j=1;j<(*mat).m;j++) //Starts with 1 and goes to m-1 (because the first col is lost)
                     if(l!=i)
                         (*(tmp[l])).matrix[delay][j-1]=(*mat).matrix[i][j];
                if(l!=i)
                    delay++;
            }
             tmp_return[l]=((*mat).matrix[l][0])*(expon(-1.0,(double)(l+2)))*(determinante_matrix(tmp[l]));
             free(tmp[l]);

        }
        for(l=0;l<(*mat).m;l++)
            ret=ret+tmp_return[l];
        return ret;
    }
}
/*==============================================*/
MATRIX *cofator_matrix(MATRIX *matrix)
{
    int i,j,k,l;
    int delay_i;
    int delay_j;
    MATRIX *matrix_of_determ;
    MATRIX *tmp;

    if((matrix_of_determ=cria_matrix((int)(*matrix).m,(int)(*matrix).n))==NULL)
        return NULL;

    if((tmp=cria_matrix((int)(((*matrix).m)-1),(int)(((*matrix).n)-1)))==NULL)
        return NULL;

    for(i=0;i<(*matrix).m;i++)
    {
        for(j=0;j<(*matrix).n;j++)
        {
             delay_i=0;
             delay_j=0;
             for(k=0;k<(*matrix).m;k++)
             {
                 if(i!=k)
                 {
                     for(l=0;l<(*matrix).n;l++)
                     {
                         if(j!=l)
                         {
                             (*tmp).matrix[delay_i][delay_j]=(*matrix).matrix[k][l];
                             delay_j++;
                         }
                     }
                     delay_j=0;
                     delay_i++;
                 }
             }
            (*matrix_of_determ).matrix[i][j]=determinante_matrix(tmp);
        }
    }

    free(tmp);
//----------------------------------------------------
    for(i=0;i<(*matrix).m;i++)
        for(j=0;j<(*matrix).n;j++)
            (*matrix_of_determ).matrix[i][j] = (expon(-1.0,(double)(i+j+2))) * ((*matrix_of_determ).matrix[i][j]);

    return matrix_of_determ;
}
/*==============================================*/
double expon(double number, double exponencial)
{
    int i;
    double tmp=number;
    for(i=exponencial-1;i>0;i--)
        tmp=tmp*number;
    return tmp;
}
/*==============================================*/
MATRIX *soma_matrix(MATRIX *matrix1, MATRIX *matrix2)
{

    int i,j;
    MATRIX *new_matrix;

    if((*matrix1).m!=(*matrix2).m || (*matrix1).n!=(*matrix2).n) //If m1!=m2 or n1!=n2
        return NULL;

    if((new_matrix=cria_matrix((int)(*matrix1).m, (int)(*matrix1).n))==NULL) //Try create the new_matrix
        return NULL;

    for(i=0;i<(*matrix1).m;i++)
        for(j=0;j<(*matrix1).n;j++)
            (*new_matrix).matrix[i][j]=(*matrix1).matrix[i][j]+(*matrix2).matrix[i][j];

    return new_matrix;
}
/*==============================================*/
MATRIX *multiplica_matrix(MATRIX *matrix1, MATRIX *matrix2)
{

    int i,j,k;
    double sum;
    MATRIX *new_matrix;

    if((*matrix1).n!=(*matrix2).m) //If n1!=m2
        return NULL;

    if((new_matrix=cria_matrix((int)(*matrix1).m, (int)(*matrix2).n))==NULL) //Try create the new_matrix(m1,n2)
        return NULL;

    for(i=0;i<(*matrix1).m;i++)
        for(k=0;k<(*matrix2).n;k++)
        {
            sum=0.0;
            for(j=0;j<(*matrix1).n;j++)
                sum = sum + ((*matrix1).matrix[i][j]) * ((*matrix2).matrix[j][k]);
            (*new_matrix).matrix[i][k]=sum;
        }

    return new_matrix;
}
/*==============================================*/
MATRIX *const_multiplica_matrix(double constante, MATRIX *matrix)
{
    int i,j;
    MATRIX *new_matrix;

    if((new_matrix=cria_matrix((int)(*matrix).m,(int)(*matrix).n))==NULL)
        return NULL;
    
    for(i=0;i<(*matrix).m;i++)
        for(j=0;j<(*matrix).n;j++)
            (*new_matrix).matrix[i][j] = constante * ((*matrix).matrix[i][j]);

    return new_matrix;
}
/*==============================================*/
void imprime_matrix(MATRIX *matrix)
{
    int i,j;
    for(i=0;i<(*matrix).m;i++)
    {
        for(j=0;j<(*matrix).n;j++)
            printf(" %f",(*matrix).matrix[i][j]);
        printf("\n");
    }
}