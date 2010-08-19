#include <stdio.h>
#include <stdlib.h>
#include "rrd.h"
#include "rrd_create.c"
#include "rrd_diff.c"
#include "rrd_error.c"
#include "rrd_fetch.c"
#include "rrd_format.c"
#include "rrd_last.c"
#include "rrd_open.c"
#include "rrd_update.c"
#include "parsetime.c"


int create_and_copy()
{
  char *rrd_argv[10];
  int rrd_argc;
  int rrd_ret;
  
//===================CREATE=================== First RRD, that will be copied to a second RRD
  rrd_argc=10;
  rrd_argv[0]="create";
  rrd_argv[1]="db1.rrd";
  rrd_argv[2]="--start";
  rrd_argv[3]= "920804400";
  rrd_argv[4]="--step";
  rrd_argv[5]="300";
  rrd_argv[6]="DS:prob1:GAUGE:600:U:U"; // I'm using just 3 DST but the code is generical, so this works for any numbers of DST
	rrd_argv[7]="DS:prob2:GAUGE:600:U:U";
	rrd_argv[8]="DS:prob3:GAUGE:600:U:U";
	rrd_argv[9]="RRA:LAST:0.5:1:10";
  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_create(rrd_argc, rrd_argv);
  //printf("rrd_create: \t[%d]\n",rrd_ret);
//===================UPDATE=================== Updating the first RRD

  rrd_argc=6;
  rrd_argv[0]="update";
  rrd_argv[1]="db1.rrd";
  rrd_argv[2]="920804700:12345:54321:12354";
  rrd_argv[3]="920805000:12357:75321:12375";
  rrd_argv[4]="920805300:12363:36321:12336";
	rrd_argv[5]="920805600:12363:36321:12336";
  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_update(rrd_argc, rrd_argv);
 // printf("rrd_update: \t[%d]\n",rrd_ret);
  
//===================CREATE 2=================== Creating the second RRD, note that the start time of this RRD is complety different from the first RRD
  rrd_argc=10;
  rrd_argv[0]="create";
  rrd_argv[1]="db2.rrd";
  rrd_argv[2]="--start";
  rrd_argv[3]= "1275161788";
  rrd_argv[4]="--step";
  rrd_argv[5]="300";
  rrd_argv[6]="DS:prob1:GAUGE:600:U:U";
	rrd_argv[7]="DS:prob2:GAUGE:600:U:U";
	rrd_argv[8]="DS:prob3:GAUGE:600:U:U";
	rrd_argv[9]="RRA:LAST:0.5:1:10";
  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_create(rrd_argc, rrd_argv);
 // printf("rrd_create 2: \t[%d]\n",rrd_ret);

//===================LAST=================== Getting the last data inserted in the first RRD
  rrd_argc=2;
	rrd_argv[0]="last";
  rrd_argv[1]="db1.rrd";

  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_last(rrd_argc, rrd_argv);  
	char last[10];
	sprintf(last,"%d",rrd_ret);


//===================FETCH=================== Fethcing an interval and storing this interval into the second RRD

  rrd_argc=7;  
  time_t start,end;
  rrd_value_t *datavalue;
  unsigned long step, ds_cnt;
  char **ds_namv,*rrd_argv_2[10];
  int rrd_ret_2,rrd_argc_2, start_2=1275161788; 
  int i=1,cnt=0,temp=0;
  char *data_f, data_temp[12];
  rrd_argv[0]="fetch";
  rrd_argv[5]="db1.rrd";
  rrd_argv[6]="LAST";
  rrd_argv[1]="--start";
  rrd_argv[2]="920804700";
  rrd_argv[3]="--end";
  rrd_argv[4]=last; // last data inserted.
 
 	//====Update parameters to second RRD 
  rrd_argc_2=3;
  rrd_argv_2[0]="update";
  rrd_argv_2[1]="db2.rrd";
  optind = opterr = 0;
  rrd_clear_error();
  
  if((rrd_ret=rrd_fetch(7,rrd_argv,&start,&end,&step,&ds_cnt,&ds_namv,&datavalue))!= -1)
   {
   // printf("rrd_fetch: [%d]\n",rrd_ret);
   // printf(">> num_ds: %d  name: %s (--step: %d | --start: %d | end: %d)\n\n",ds_cnt,ds_namv[0],(int)step,(int)start,(int)end);
   data_f=malloc(11+(18*ds_cnt));
   for(;i<=(((int)end -(int)start)/step)+1;i++) // each iteration is for one step
    {
     sprintf(data_f,"%d",start_2+(i*step)); // get the new time for the second RRD 
		 for(;cnt<ds_cnt+temp;cnt++)	 // each iteration if for one DST
		  {	
        sprintf(data_temp,":%.6f", (double)datavalue[cnt]);
			  //printf("%s\n",data_temp);
			  data_f=strcat(data_f,data_temp);
		  }
		 rrd_argv_2[2]=data_f; // the time:data:data:... to be inserted
     optind = opterr = 0;
     rrd_clear_error();
     printf("%s return:%d\n",data_f,rrd_ret_2=rrd_update(rrd_argc_2, rrd_argv_2)); // return of the update function, < 0 ins't a good value :(
		 temp=cnt;   	  
	 }
  }
return 0;
}

int main(int argc, char *argv[])
{
	int (*func)();        // A generic pointer, this can be passed as argument in the generic daemon code, in daemon.c 
	func=&create_and_copy;
	(*func)();
}

