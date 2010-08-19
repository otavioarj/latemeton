#include <stdio.h>
#include <stdlib.h>
#include <rrd.h>
//#include "rrd_create.c"
//#include "rrd_diff.c"
//#include "rrd_error.c"
//#include "rrd_fetch.c"
//#include "rrd_format.c"
//#include "rrd_last.c"
//#include "rrd_open.c"
//#include "rrd_update.c"
//#include "parsetime.c"
//#include "rrd_graph.c"

//---------------------------------------------
char * weekday[] = { "Sunday", "Monday",
                     "Tuesday", "Wednesday",
                     "Thursday", "Friday",
                      "Saturday"};


int main(int argc, char *argv[])
{

  char *rrd_argv[10];
  int rrd_argc;
  int rrd_ret;
  
//===================CREATE===================
  rrd_argc=9;
  rrd_argv[0]="create";
  rrd_argv[1]="otest.rrd";
  rrd_argv[2]="--start";
  rrd_argv[3]= "920804400";
  rrd_argv[4]="--step";
  rrd_argv[5]="300";
  rrd_argv[6]="DS:speed:GAUGE:600:U:U";
  rrd_argv[7]="RRA:AVERAGE:0.5:1:24";
  rrd_argv[8]="RRA:AVERAGE:0.5:7:10";
//  rrd_argv[9]=NULL;

  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_create(rrd_argc, rrd_argv);
  printf("rrd_create: \t[%d]\n",rrd_ret);
//===================UPDATE===================

  rrd_argc=5;
  rrd_argv[0]="update";
  rrd_argv[1]="otest.rrd";
  rrd_argv[2]="920804700:12345";
  rrd_argv[3]="920805000:12357";
  rrd_argv[4]="920805300:12363";
//  rrd_argv[5]=NULL;
  
  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_update(rrd_argc, rrd_argv);
  printf("rrd_update: \t[%d]\n",rrd_ret);
    
//===================UPDATE===================

 rrd_argc=8;
  rrd_argv[0]="update";
  rrd_argv[1]="otest.rrd";
  rrd_argv[2]="920805600:12363";
  rrd_argv[3]="920805900:12361";
  rrd_argv[4]="920806200:12373";
  //rrd_argv[5]="920806500:12393";
  //rrd_argv[6]="920808000:12399";
 // rrd_argv[7]="920808300:12493";

//  rrd_argv[5]=NULL;
  
  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=rrd_update(rrd_argc, rrd_argv);
  printf("rrd_update: \t[%d]\n",rrd_ret);

//===================LAST===================
  rrd_argc=2; //OBS: Erro (deveria dar certo com rrd_argc=1)
  rrd_argv[0]="last";
  rrd_argv[1]="otest.rrd";
//  rrd_argv[2]=NULL;

  optind = opterr = 0;
  rrd_clear_error();
  
  rrd_ret=(int)rrd_last(rrd_argc, rrd_argv);
  time_t rrd_ret_time = (time_t)rrd_ret;
  char* date = ctime(&rrd_ret_time);
  printf("rrd_last:\tseconds: %d  |  date: %s",rrd_ret,date);
  
//===================FETCH===================

  rrd_argc=7;
  
 time_t start,end;
 rrd_value_t *datatmp;
 unsigned long step, ds_cnt;
 char **ds_namv;
 
  rrd_argv[0]="fetch";
  rrd_argv[5]="otest.rrd";
  rrd_argv[6]="AVERAGE";
  rrd_argv[1]="--start";
  rrd_argv[2]="920804400";
  rrd_argv[3]="--end";
  rrd_argv[4]="920805300";
 // rrd_argv[7]=NULL;
  
  optind = opterr = 0;
  rrd_clear_error();

 if((rrd_ret=rrd_fetch(7, (char**)rrd_argv,&start,&end,&step,&ds_cnt,&ds_namv,&datatmp))!= -1)
 {
  printf("rrd_fetch: [%d]\n",rrd_ret);
  printf(">> num_ds: %d  name: %s (--step: %d | --start: %d | end: %d)\n\n",ds_cnt,ds_namv[0],(int)step,(int)start,(int)end);
  int i=0;
  for(i=0;i<4;i++)
  {
    printf("%d: %e\n",(int)start+(i*(int)step),datatmp[i]);
  }

}	
  return 0;
}
