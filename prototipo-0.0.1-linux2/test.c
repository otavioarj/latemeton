#include "create.c"

int create_and_copy()
{
  char **rrd_argv;
  int rrd_argc;
  int rrd_ret;
	//printf("create %d\n",create("db1.rrd", "920804400", 3, "300"));

//===================UPDATE=================== Updating the first RRD

// rrd_argc=12;
//  rrd_argv[0]="update";
//  rrd_argv[1]="db1.rrd";
 // rrd_argv[2]="920804700:12345:54321:12354";
//  rrd_argv[3]="920805000:12357:75321:12375";
//  rrd_argv[4]="920805300:12363:36321:12336";
//	rrd_argv[5]="920805600:12363:36321:12336";
//	rrd_argv[6]="920805900:12345:54321:12354";
//  rrd_argv[7]="920806200:12357:75321:12375";
//  rrd_argv[8]="920806500:12363:36321:12336";
//	rrd_argv[9]="920806800:12363:36321:12336";
 // rrd_argv[10]="920807100:12357:75321:12375";
//	rrd_argv[11]="920807400:12363:36321:12336";
 // optind = opterr = 0;
//  rrd_clear_error();
  
//  rrd_ret=rrd_update(rrd_argc, rrd_argv);
//  printf("rrd_update: \t[%d]\n",rrd_ret);

//printf("create2 %d\n",create("db2.rrd", "1275161788", 3, "300"));
/*
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
	rrd_argv[9]="RRA:LAST:0.5:1:24";
  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_create(rrd_argc, rrd_argv);
  printf("rrd_create 2: \t[%d]\n",rrd_ret);*/

//===================LAST=================== Getting the last data inserted in the first RRD

  rrd_argc=2;
	rrd_argv=malloc( rrd_argc*sizeof(char *));
	rrd_argv[0]="last";
  rrd_argv[1]="db1.rrd";

//  optind = opterr = 0;
//  rrd_clear_error();
  
  rrd_ret=rrd_last(rrd_argc, rrd_argv);  
	char last[10];
	sprintf(last,"%d",rrd_ret);
	free(rrd_argv);
//==================INIT======================

printf("%s %d\n",last,rrd_ret);
//===================FETCH=================== Fethcing an interval and storing this interval into the second RRD

  rrd_argc=7;  
	rrd_argv=malloc( rrd_argc*sizeof(char *));
  time_t start,end;
  rrd_value_t *datavalue;
  unsigned long step, ds_cnt;
  char **ds_namv,*rrd_argv_2[3];
  int rrd_argc_2, start_2=1275161788; 
  int i=1,cnt=0,temp=0;
  char *data_f, data_temp[12],*temps;
  double **data_series;
  rrd_argv[0]="fetch";
  rrd_argv[5]="db1.rrd";
  rrd_argv[6]="LAST";
  rrd_argv[1]="--start";
  temps=malloc(10);  
  sprintf(temps,"%d",300*24*first("db1.rrd"));
  printf("%s\n",temps);
  rrd_argv[2]=(char *)temps;
 // rrd_argv[2]="920804400";
  free(temps);
  rrd_argv[3]="--end";
  rrd_argv[4]=last; // last data inserted.
 
 	//====Update parameters to second RRD 
  rrd_argc_2=3;
	rrd_argv_2[0]="update";
  rrd_argv_2[1]="db2.rrd";
	 
	if((rrd_ret=rrd_fetch(7,rrd_argv,&start,&end,&step,&ds_cnt,&ds_namv,&datavalue))!= -1)
   {
     printf("rrd_fetch: [%d]\n",rrd_ret);
		 data_series=malloc(24*sizeof(double *));
     data_f=malloc(10+(13*ds_cnt)); // era 11+18* 
    for(;i<=(((int)end -(int)start-step)/step);i++) // each iteration is for one step
     {
      sprintf(data_f,"%lu",start_2+(i*step)); // get the new time for the second RRD 
		  for(;cnt<ds_cnt+temp;cnt++)	 // each iteration if for one DST
		  {	
      			  sprintf(data_temp,":%.6f", (double)datavalue[cnt]);
			  data_f=strcat(data_f,data_temp);
		  }
			data_series[i]=malloc(6*sizeof(double));
			strip_data(data_f,ds_cnt,data_series[i]);
		 //rrd_argv_2[2]=data_f; // the time:data:data:... to be inserted
		 temp=cnt;   	  
	  }
   }
	free(rrd_argv);
	// aqui vai pras funcoes =]
	return 0;
}

int main(int argc, char *argv[])
{
	int (*func)();        // A generic pointer, this can be passed as argument in the generic daemon code, in daemon.c 
	func=&create_and_copy;
	(*func)();
}

