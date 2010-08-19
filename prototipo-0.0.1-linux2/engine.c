#include "create.c"

int daemon_main() 
 {
	char *rrd_argv[10];
  int rrd_argc;
  int rrd_ret;
	printf("create %d\n",create("db1.rrd", "920804400", 3, "300"));
	// gera os probs

  rrd_argc=2;
	rrd_argv[0]="last";
  rrd_argv[1]="db1.rrd";

  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_last(rrd_argc, rrd_argv);  
	char last[10];
	sprintf(last,"%d",rrd_ret);
//==================INIT======================

  printf("%s %d\n",last,rrd_ret);
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
	sprintf(rrd_argv[2],"%d",first("db1.rrd"));
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
   printf("rrd_fetch: [%d]\n",rrd_ret);
   printf(">> num_ds: %d  name: %s (--step: %d | --start: %d | end: %d)\n\n",ds_cnt,ds_namv[0],(int)step,(int)start,(int)end);
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
}
