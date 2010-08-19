#include <stdio.h>
#include <stdlib.h>
#include <rrd.h>
#define MAX 24

int create(char * name, char* start,short int qnt_ds, char* step )
{
	int rrd_argc=7+qnt_ds,cnt=1;
	char **rrd_argv,pam[23];
  rrd_argv=malloc(sizeof(6+cnt));
	rrd_argv[0]=malloc(7);		
	rrd_argv[0]="create";	
	rrd_argv[1]=name;
	rrd_argv[2]=malloc(7);
	rrd_argv[2]="--start";
	printf("%s\n",start);
	rrd_argv[3]=start;
	rrd_argv[4]=malloc(6);
	rrd_argv[4]="--step";
	rrd_argv[5]=step;
	for(;cnt<=qnt_ds;cnt++)
		{
			sprintf(pam,"DS:prob%c:GAUGE:600:U:U",cnt+48);
      rrd_argv[cnt+5]=malloc(23);			
			strncpy(rrd_argv[cnt+5],pam,23);
		}
	rrd_argv[5+qnt_ds+1]="RRA:LAST:0.5:1:24"; // need to use strcap to integrate max of RRA with macro MAX!
//	optind = opterr = 0;
//	rrd_clear_error();  
	rrd_create(rrd_argc, rrd_argv);
	return 1;
}

int  first(char *name)
{
	char *rrd_argv[2];    
	int arg=2;
	rrd_argv[0]="first";
	rrd_argv[1]=name;
  return rrd_first(arg,rrd_argv);   
}

void strip_data(char * data,int num,double * serie)
{
	int cnt,pos=11,diff=11;
	//int * temp_serie;
	char aux[12];
	printf("%s %d\n",data,num);
	//temp_serie=(int *)malloc(sizeof(int));	
	for(cnt=0;cnt<num;cnt++)
		{
			for(;data[pos]!=':' && pos<(10+13*num);pos++)
					aux[pos-diff]=data[pos];
			diff=++pos;
			printf("%s %d %d\n",aux,cnt, pos);
			serie[cnt]=atof((const char *)aux);
		}
	//printf("%f  %f \n",serie[0],serie[1]);	
}


