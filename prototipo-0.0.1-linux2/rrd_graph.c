
/****************************************************************************
 * RRDtool 1.0.42  Copyright Tobias Oetiker, 1997 - 2000
 ****************************************************************************
 * rrd__graph.c  make creates ne rrds
 ****************************************************************************/

#include "rrd_tool.h"
#include "rrd_graph.h"
#include <gd.h>

#ifdef LATIN2_ENABLE
#include <gdlucidan10l2.h>
#include <gdlucidab12l2.h>
#else
#include <gdlucidan10.h>
#include <gdlucidab12.h>
#endif

#include <sys/stat.h>
#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef HAVE_TIME_H
#include <time.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef LATIN2_ENABLE
#define SmallFont gdLucidaNormal10l2
#define LargeFont gdLucidaBold12l2
#else    
#define SmallFont gdLucidaNormal10
#define LargeFont gdLucidaBold12
#endif

/* #define DEBUG */

#ifdef DEBUG
# define DPRINT(x)    (void)(printf x, printf("\n"))
#else
# define DPRINT(x)
#endif


xlab_t xlab[] = {
    {0,        TMT_SECOND,30, TMT_MINUTE,5,  TMT_MINUTE,5,         0,"%H:%M"},
    {2,        TMT_MINUTE,1,  TMT_MINUTE,5,  TMT_MINUTE,5,         0,"%H:%M"},
    {5,        TMT_MINUTE,2,  TMT_MINUTE,10, TMT_MINUTE,10,        0,"%H:%M"},
    {10,       TMT_MINUTE,5,  TMT_MINUTE,20, TMT_MINUTE,20,        0,"%H:%M"},
    {30,       TMT_MINUTE,10, TMT_HOUR,1,    TMT_HOUR,1,           0,"%H:%M"},
    {60,       TMT_MINUTE,30, TMT_HOUR,2,    TMT_HOUR,2,           0,"%H:%M"},
    {180,      TMT_HOUR,1,    TMT_HOUR,6,    TMT_HOUR,6,           0,"%H:%M"},
    /*{300,      TMT_HOUR,3,    TMT_HOUR,12,   TMT_HOUR,12,    12*3600,"%a %p"},  this looks silly*/
    {600,      TMT_HOUR,6,    TMT_DAY,1,     TMT_DAY,1,      24*3600,"%a"},
    {1800,     TMT_HOUR,12,   TMT_DAY,1,     TMT_DAY,2,      24*3600,"%a"},
    {3600,     TMT_DAY,1,     TMT_WEEK,1,     TMT_WEEK,1,    7*24*3600,"Week %V"},
    {3*3600,   TMT_WEEK,1,      TMT_MONTH,1,     TMT_WEEK,2,    7*24*3600,"Week %V"},
    {6*3600,   TMT_MONTH,1,   TMT_MONTH,1,   TMT_MONTH,1, 30*24*3600,"%b"},
    {48*3600,  TMT_MONTH,1,   TMT_MONTH,3,   TMT_MONTH,3, 30*24*3600,"%b"},
    {10*24*3600, TMT_YEAR,1,  TMT_YEAR,1,    TMT_YEAR,1, 365*24*3600,"%y"},
    {-1,TMT_MONTH,0,TMT_MONTH,0,TMT_MONTH,0,0,""}
};

/* sensible logarithmic y label intervals ...
   the first element of each row defines the possible starting points on the
   y axis ... the other specify the */

double yloglab[][12]= {{ 1e9, 1,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0 },
		       {  1e3, 1,  0,  0,  0,   0,  0,  0,  0,  0,  0,  0 },
		       {  1e1, 1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
		       /* {  1e1, 1,  5,  0,  0,  0,  0,  0,  0,  0,  0,  0 }, */
		       {  1e1, 1,  2.5,  5,  7.5,  0,  0,  0,  0,  0,  0,  0 },
		       {  1e1, 1,  2,  4,  6,  8,  0,  0,  0,  0,  0,  0 },
		       {  1e1, 1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0 },
		       {  0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 }};

/* sensible y label intervals ...*/

ylab_t ylab[]= {
    {0.1, {1,2, 5,10}},
    {0.2, {1,5,10,20}},
    {0.5, {1,2, 4,10}},
    {1.0,   {1,2, 5,10}},
    {2.0,   {1,5,10,20}},
    {5.0,   {1,2, 4,10}},
    {10.0,  {1,2, 5,10}},
    {20.0,  {1,5,10,20}},
    {50.0,  {1,2, 4,10}},
    {100.0, {1,2, 5,10}},
    {200.0, {1,5,10,20}},
    {500.0, {1,2, 4,10}},
    {0.0,   {0,0,0,0}}};



col_trip_t graph_col[] = { /* default colors */
    {255,255,255,-1},   /* canvas */
    {245,245,245,-1},   /* background */
    {200,200,200,-1},   /* shade A    */
    {150,150,150,-1},   /* shade B    */
    {140,140,140,-1},      /* grid */
    {130,30,30,-1},      /* major grid */
    {0,0,0,-1},         /* font */ 	
    {0,0,0,-1},         /* frame */
    {255,0,0,-1}	/*arrow*/
};

/* this structure describes the elements which can make up a graph.
   because they are quite diverse, not all elements will use all the
   possible parts of the structure. */
#ifdef HAVE_SNPRINTF
#define FMT_LEG_LEN 200
#else
#define FMT_LEG_LEN 2000
#endif


/* translate time values into x coordinates */   
/*#define xtr(x) (int)((double)im->xorigin \
		+ ((double) im->xsize / (double)(im->end - im->start) ) \
		* ((double)(x) - im->start)+0.5) */
/* initialize with xtr(im,0); */
int
xtr(image_desc_t *im,time_t mytime){
    static double pixie;
    if (mytime==0){
	pixie = (double) im->xsize / (double)(im->end - im->start);
	return im->xorigin;
    }
    return (int)((double)im->xorigin 
		 + pixie * ( mytime - im->start ) );
}

/* translate data values into y coordinates */

/* #define ytr(x) (int)((double)im->yorigin \
		- ((double) im->ysize / (im->maxval - im->minval) ) \
		* ((double)(x) - im->minval)+0.5) */
int
ytr(image_desc_t *im, double value){
    static double pixie;
    double yval;
    if (isnan(value)){
      if(!im->logarithmic)
	pixie = (double) im->ysize / (im->maxval - im->minval);
      else 
	pixie = (double) im->ysize / (log10(im->maxval) - log10(im->minval));
      yval = im->yorigin;
    } else if(!im->logarithmic) {
      yval = im->yorigin - pixie * (value - im->minval) + 0.5;
    } else {
      if (value < im->minval) {
	yval = im->yorigin;
      } else {
	yval = im->yorigin - pixie * (log10(value) - log10(im->minval)) + 0.5;
      }
    }
    /* make sure we don't return anything too unreasonable. GD lib can
       get terribly slow when drawing lines outside its scope. This is 
       especially problematic in connection with the rigid option */
    if (! im->rigid) {
      return (int)yval;
    } else if ((int)yval > im->yorigin) {
      return im->yorigin+2;
    } else if ((int) yval < im->yorigin - im->ysize){
      return im->yorigin - im->ysize - 2;
    } else {
      return (int)yval;
    } 
}

   
    


/* conversion function for symbolic entry names */


#define conv_if(VV,VVV) \
   if (strcmp(#VV, string) == 0) return VVV ;

enum gf_en gf_conv(char *string){
    
    conv_if(PRINT,GF_PRINT)
    conv_if(GPRINT,GF_GPRINT)
    conv_if(COMMENT,GF_COMMENT)
    conv_if(HRULE,GF_HRULE)
    conv_if(VRULE,GF_VRULE)
    conv_if(LINE1,GF_LINE1)
    conv_if(LINE2,GF_LINE2)
    conv_if(LINE3,GF_LINE3)
    conv_if(AREA,GF_AREA)
    conv_if(STACK,GF_STACK)
    conv_if(DEF,GF_DEF)
    conv_if(CDEF,GF_CDEF)
    conv_if(XPORT,GF_XPORT)
    
    return (-1);
}

enum if_en if_conv(char *string){
    
    conv_if(GIF,IF_GIF)
    conv_if(PNG,IF_PNG)
    conv_if(GD,IF_GD)

    return (-1);
}

enum tmt_en tmt_conv(char *string){

    conv_if(SECOND,TMT_SECOND)
    conv_if(MINUTE,TMT_MINUTE)
    conv_if(HOUR,TMT_HOUR)
    conv_if(DAY,TMT_DAY)
    conv_if(WEEK,TMT_WEEK)
    conv_if(MONTH,TMT_MONTH)
    conv_if(YEAR,TMT_YEAR)
    return (-1);
}

enum grc_en grc_conv(char *string){

    conv_if(BACK,GRC_BACK)
    conv_if(CANVAS,GRC_CANVAS)
    conv_if(SHADEA,GRC_SHADEA)
    conv_if(SHADEB,GRC_SHADEB)
    conv_if(GRID,GRC_GRID)
    conv_if(MGRID,GRC_MGRID)
    conv_if(FONT,GRC_FONT)
    conv_if(FRAME,GRC_FRAME)
    conv_if(ARROW,GRC_ARROW)

    return -1;	
}

#undef conv_if



int
im_free(image_desc_t *im)
{
    long i,ii;
    if (im == NULL) return 0;
    for(i=0;i<im->gdes_c;i++){
      if (im->gdes[i].data_first){
	/* careful here, because a single pointer can occur several times */
	  free (im->gdes[i].data);
	  if (im->gdes[i].ds_namv){
	      for (ii=0;ii<im->gdes[i].ds_cnt;ii++)
		  free(im->gdes[i].ds_namv[ii]);
	      free(im->gdes[i].ds_namv);
	  }
      }
      free (im->gdes[i].p_data);
      free (im->gdes[i].rpnp);
    }
    free(im->gdes);
    return 0;
}

/* find SI magnitude symbol for the given number*/
void
auto_scale(
	   image_desc_t *im,   /* image description */
	   double *value,
	   char **symb_ptr,
	   double *magfact
	   )
{
	
    char *symbol[] = {"a", /* 10e-18 Ato */
		      "f", /* 10e-15 Femto */
		      "p", /* 10e-12 Pico */
		      "n", /* 10e-9  Nano */
		      "u", /* 10e-6  Micro */
		      "m", /* 10e-3  Milli */
		      " ", /* Base */
		      "k", /* 10e3   Kilo */
		      "M", /* 10e6   Mega */
		      "G", /* 10e9   Giga */
		      "T", /* 10e12  Terra */
		      "P", /* 10e15  Peta */
		      "E"};/* 10e18  Exa */

    int symbcenter = 6;
    int sindex;  

    if (*value == 0.0 || isnan(*value) ) {
	sindex = 0;
	*magfact = 1.0;
    } else {
	sindex = floor(log(fabs(*value))/log((double)im->base)); 
	*magfact = pow((double)im->base, (double)sindex);
	(*value) /= (*magfact);
    }
    if ( sindex <= symbcenter && sindex >= -symbcenter) {
	(*symb_ptr) = symbol[sindex+symbcenter];
    }
    else {
	(*symb_ptr) = "?";
    }
}


/* find SI magnitude symbol for the numbers on the y-axis*/
void 
si_unit(
    image_desc_t *im   /* image description */
)
{

    char symbol[] = {'a', /* 10e-18 Ato */ 
		     'f', /* 10e-15 Femto */
		     'p', /* 10e-12 Pico */
		     'n', /* 10e-9  Nano */
		     'u', /* 10e-6  Micro */
		     'm', /* 10e-3  Milli */
		     ' ', /* Base */
		     'k', /* 10e3   Kilo */
		     'M', /* 10e6   Mega */
		     'G', /* 10e9   Giga */
		     'T', /* 10e12  Terra */
		     'P', /* 10e15  Peta */
		     'E'};/* 10e18  Exa */

    int   symbcenter = 6;
    double digits;  
    
    if (im->unitsexponent != 9999) {
	/* unitsexponent = 9, 6, 3, 0, -3, -6, -9, etc */
        digits = floor(im->unitsexponent / 3);
    } else {
        digits = floor( log( max( fabs(im->minval),fabs(im->maxval)))/log((double)im->base)); 
    }
    im->magfact = pow((double)im->base , digits);

#ifdef DEBUG
    printf("digits %6.3f  im->magfact %6.3f\n",digits,im->magfact);
#endif

    if ( ((digits+symbcenter) < sizeof(symbol)) &&
		    ((digits+symbcenter) >= 0) )
        im->symbol = symbol[(int)digits+symbcenter];
    else
	im->symbol = ' ';
 }

/*  move min and max values around to become sensible */

void 
expand_range(image_desc_t *im)
{
    double sensiblevalues[] ={1000.0,900.0,800.0,750.0,700.0,
			      600.0,500.0,400.0,300.0,250.0,
			      200.0,125.0,100.0,90.0,80.0,
			      75.0,70.0,60.0,50.0,40.0,30.0,
			      25.0,20.0,10.0,9.0,8.0,
			      7.0,6.0,5.0,4.0,3.5,3.0,
			      2.5,2.0,1.8,1.5,1.2,1.0,
			      0.8,0.7,0.6,0.5,0.4,0.3,0.2,0.1,0.0,-1};
    
    double scaled_min,scaled_max;  
    double adj;
    int i;
    

    
#ifdef DEBUG
    printf("Min: %6.2f Max: %6.2f MagFactor: %6.2f\n",
	   im->minval,im->maxval,im->magfact);
#endif

    if (isnan(im->ygridstep)){
	if(im->extra_flags & ALTYMRTG) { /* mrtg */
 int  decimals = ceil(log10(max(fabs(im->maxval),fabs(im->minval))));
 double scaledstep = ceil(25 * pow((double)10,-decimals) * im->maxval) * pow((double)10,decimals - 2);
 im->minval = 0;
 im->maxval = 4 * scaledstep;
	} else
	if(im->extra_flags & ALTAUTOSCALE) {
	    /* measure the amplitude of the function. Make sure that
	       graph boundaries are slightly higher then max/min vals
	       so we can see amplitude on the graph */
	      double delt, fact;

	      delt = im->maxval - im->minval;
	      adj = delt * 0.1;
	      fact = 2.0 * pow(10.0,
		    floor(log10(max(fabs(im->minval), fabs(im->maxval)))) - 2);
	      if (delt < fact) {
		adj = (fact - delt) * 0.55;
#ifdef DEBUG
	      printf("Min: %6.2f Max: %6.2f delt: %6.2f fact: %6.2f adj: %6.2f\n", im->minval, im->maxval, delt, fact, adj);
#endif
	      }
	      im->minval -= adj;
	      im->maxval += adj;
	}
	else if(im->extra_flags & ALTAUTOSCALE_MAX) {
	    /* measure the amplitude of the function. Make sure that
	       graph boundaries are slightly higher than max vals
	       so we can see amplitude on the graph */
	      adj = (im->maxval - im->minval) * 0.1;
	      im->maxval += adj;
	}
	else {
	    scaled_min = im->minval / im->magfact;
	    scaled_max = im->maxval / im->magfact;
	    
	    for (i=1; sensiblevalues[i] > 0; i++){
		if (sensiblevalues[i-1]>=scaled_min &&
		    sensiblevalues[i]<=scaled_min)	
		    im->minval = sensiblevalues[i]*(im->magfact);
		
		if (-sensiblevalues[i-1]<=scaled_min &&
		-sensiblevalues[i]>=scaled_min)
		    im->minval = -sensiblevalues[i-1]*(im->magfact);
		
		if (sensiblevalues[i-1] >= scaled_max &&
		    sensiblevalues[i] <= scaled_max)
		    im->maxval = sensiblevalues[i-1]*(im->magfact);
		
		if (-sensiblevalues[i-1]<=scaled_max &&
		    -sensiblevalues[i] >=scaled_max)
		    im->maxval = -sensiblevalues[i]*(im->magfact);
	    }
	}
    } else {
	/* adjust min and max to the grid definition if there is one */
	im->minval = (double)im->ylabfact * im->ygridstep * 
	    floor(im->minval / ((double)im->ylabfact * im->ygridstep));
	im->maxval = (double)im->ylabfact * im->ygridstep * 
	    ceil(im->maxval /( (double)im->ylabfact * im->ygridstep));
    }
    
#ifdef DEBUG
    fprintf(stderr,"SCALED Min: %6.2f Max: %6.2f Factor: %6.2f\n",
	   im->minval,im->maxval,im->magfact);
#endif
}

    
/* reduce data reimplementation by Alex */

void
reduce_data(
    enum cf_en     cf,         /* which consolidation function ?*/
    unsigned long  cur_step,   /* step the data currently is in */
    time_t         *start,     /* start, end and step as requested ... */
    time_t         *end,       /* ... by the application will be   ... */
    unsigned long  *step,      /* ... adjusted to represent reality    */
    unsigned long  *ds_cnt,    /* number of data sources in file */
    rrd_value_t    **data)     /* two dimensional array containing the data */
{
    int i,reduce_factor = ceil((double)(*step) / (double)cur_step);
    unsigned long col,dst_row,row_cnt,start_offset,end_offset,skiprows=0;
    rrd_value_t    *srcptr,*dstptr;

    (*step) = cur_step*reduce_factor; /* set new step size for reduced data */
    dstptr = *data;
    srcptr = *data;

    /* We were given one extra row at the beginning of the interval.
    ** We also need to return one extra row.  The extra interval is
    ** the one defined by the start time in both cases.  It is not
    ** used when graphing but maybe we can use it while reducing the
    ** data.
    */
    row_cnt = ((*end)-(*start))/cur_step +1;

    /* alter start and end so that they are multiples of the new steptime.
    ** End will be shifted towards the future and start will be shifted
    ** towards the past in order to include the requested interval
    */ 
    end_offset = (*end) % (*step);
    if (end_offset) end_offset = (*step)-end_offset;
    start_offset = (*start) % (*step);
    (*end) = (*end)+end_offset;
    (*start) = (*start)-start_offset;

    /* The first destination row is unknown yet it still needs
    ** to be present in the returned data.  Skip it.
    ** Don't make it NaN or we might overwrite the source.
    */
    dstptr += (*ds_cnt);

    /* Depending on the amount of extra data needed at the
    ** start of the destination, three things can happen:
    ** -1- start_offset == 0:  skip the extra source row
    ** -2- start_offset == cur_step: do nothing
    ** -3- start_offset > cur_step: skip some source rows and 
    **                      fill one destination row with NaN
    */
    if (start_offset==0) {
	srcptr+=(*ds_cnt);
	row_cnt--;
    } else if (start_offset!=cur_step) {
	skiprows=((*step)-start_offset)/cur_step+1;
	srcptr += ((*ds_cnt)*skiprows);
	row_cnt-=skiprows;
	for (col=0;col<(*ds_cnt);col++) *dstptr++=DNAN;
    }

    /* If we had to alter the endtime, there won't be
    ** enough data to fill the last row.  This means
    ** we have to skip some rows at the end
    */
    if (end_offset) {
	skiprows = ((*step)-end_offset)/cur_step;
	row_cnt-=skiprows;
    }


/* Sanity check: row_cnt should be multiple of reduce_factor */
/* if this gets triggered, something is REALY WRONG ... we die immediately */

    if (row_cnt%reduce_factor) {
	printf("SANITY CHECK: %lu rows cannot be reduced by %i \n",
				row_cnt,reduce_factor);
	printf("BUG in reduce_data()\n");
	exit(1);
    }

    /* Now combine reduce_factor intervals at a time
    ** into one interval for the destination.
    */

    for (dst_row=0;row_cnt>=reduce_factor;dst_row++) {
	for (col=0;col<(*ds_cnt);col++) {
	    rrd_value_t newval=DNAN;
	    unsigned long validval=0;

	    for (i=0;i<reduce_factor;i++) {
		if (isnan(srcptr[i*(*ds_cnt)+col])) {
		    continue;
		}
		validval++;
		if (isnan(newval)) newval = srcptr[i*(*ds_cnt)+col];
		else {
		    switch (cf) {
			case CF_AVERAGE:
			    newval += srcptr[i*(*ds_cnt)+col];
			    break;
			case CF_MINIMUM:
			    newval = min (newval,srcptr[i*(*ds_cnt)+col]);
			    break;
			case CF_MAXIMUM:
			    newval = max (newval,srcptr[i*(*ds_cnt)+col]);
			    break;
			case CF_LAST:
			    newval = srcptr[i*(*ds_cnt)+col];
			    break;
		    }
		}
	    }
	    if (validval == 0){newval = DNAN;} else{
		switch (cf) {
		    case CF_AVERAGE:		
			newval /= validval;
			break;
		    case CF_MINIMUM:
		    case CF_MAXIMUM:
		    case CF_LAST:
			break;
		}
	    }
	    *dstptr++=newval;
	}
	srcptr+=(*ds_cnt)*reduce_factor;
	row_cnt-=reduce_factor;
    }

    /* If we had to alter the endtime, we didn't have enough
    ** source rows to fill the last row. Fill it with NaN.
    */
    if (end_offset!=0) for (col=0;col<(*ds_cnt);col++) *dstptr++ = DNAN;
}


/* get the data required for the graphs from the 
   relevant rrds ... */

int
data_fetch( image_desc_t *im )
{
    int       i,ii;
    int skip;
    /* pull the data from the log files ... */
    for (i=0;i<im->gdes_c;i++){
	/* only GF_DEF elements fetch data */
	if (im->gdes[i].gf != GF_DEF) 
	    continue;

	skip=0;
	/* do we have it already ?*/
	for (ii=0;ii<i;ii++){
	    if (im->gdes[ii].gf != GF_DEF) 
		continue;
	    if((strcmp(im->gdes[i].rrd,im->gdes[ii].rrd) == 0)
		&& (im->gdes[i].cf == im->gdes[ii].cf)){
		/* OK the data it is here already ... 
		 * we just copy the header portion */
		im->gdes[i].start = im->gdes[ii].start;
		im->gdes[i].end = im->gdes[ii].end;
		im->gdes[i].step = im->gdes[ii].step;
		im->gdes[i].ds_cnt = im->gdes[ii].ds_cnt;
		im->gdes[i].ds_namv = im->gdes[ii].ds_namv;		
		im->gdes[i].data = im->gdes[ii].data;
		im->gdes[i].data_first = 0;
		skip=1;
	    }
	    if (skip) 
		break;
	}
	if (! skip) {
	    unsigned long  ft_step = im->gdes[i].step ;
	    
	    if((rrd_fetch_fn(im->gdes[i].rrd,
			     im->gdes[i].cf,
			     &im->gdes[i].start,
			     &im->gdes[i].end,
			     &ft_step,
			     &im->gdes[i].ds_cnt,
			     &im->gdes[i].ds_namv,
			     &im->gdes[i].data)) == -1){		
		return -1;
	    }
	    im->gdes[i].data_first = 1;	    
	
	    if (ft_step < im->gdes[i].step) {
		reduce_data(im->gdes[i].cf,
			    ft_step,
			    &im->gdes[i].start,
			    &im->gdes[i].end,
			    &im->gdes[i].step,
			    &im->gdes[i].ds_cnt,
			    &im->gdes[i].data);
	    } else {
		im->gdes[i].step = ft_step;
	    }
	}
	
        /* lets see if the required data source is realy there */
	for(ii=0;ii<im->gdes[i].ds_cnt;ii++){
	    if(strcmp(im->gdes[i].ds_namv[ii],im->gdes[i].ds_nam) == 0){
		im->gdes[i].ds=ii; }
	}
	if (im->gdes[i].ds== -1){
	    rrd_set_error("No DS called '%s' in '%s'",
			  im->gdes[i].ds_nam,im->gdes[i].rrd);
	    return -1; 
	}
	
    }
    return 0;
}

/* evaluate the expressions in the CDEF functions */

/*************************************************************
 * CDEF stuff 
 *************************************************************/

/* find gdes containing var*/
long
find_var(image_desc_t *im, char *key){
    long ii;
    for(ii=0;ii<im->gdes_c-1;ii++){
	if((im->gdes[ii].gf == GF_DEF 
	    || im->gdes[ii].gf == GF_CDEF) 
	   && (strcmp(im->gdes[ii].vname,key) == 0)){
	    return ii; 
	}	   
    }	    	    
    return -1;
}

/* find the largest common denominator for all the numbers
   in the 0 terminated num array */
long
lcd(long *num){
    long rest;
    int i;
    for (i=0;num[i+1]!=0;i++){
	do { 
	    rest=num[i] % num[i+1];
	    num[i]=num[i+1]; num[i+1]=rest;
	} while (rest!=0);
	num[i+1] = num[i];
    }
/*    return i==0?num[i]:num[i-1]; */
      return num[i];
}


/* convert string to rpnp */
rpnp_t * 
str2rpn(image_desc_t *im,char *expr){
    int pos=0;
    long steps=-1;    
    rpnp_t  *rpnp;
    char vname[30];

    rpnp=NULL;

    while(*expr){
	if ((rpnp = (rpnp_t *) rrd_realloc(rpnp, (++steps + 2)* 
				       sizeof(rpnp_t)))==NULL){
	    return NULL;
	}

	else if((sscanf(expr,"%lf%n",&rpnp[steps].val,&pos) == 1) 
	        && (expr[pos] == ',')){
 	    rpnp[steps].op = OP_NUMBER;
	    expr+=pos;
	} 
	
#define match_op(VV,VVV) \
        else if (strncmp(expr, #VVV, strlen(#VVV))==0 && \
                (expr[strlen(#VVV)] == ',' || expr[strlen(#VVV)] == '\0') ){ \
	    rpnp[steps].op = VV; \
	    expr+=strlen(#VVV); \
	}

#define match_op_param(VV,VVV) \
        else if (sscanf(expr, #VVV "(" DEF_NAM_FMT ")",vname) == 1) { \
			int length = 0; \
			if ((length = strlen(#VVV)+strlen(vname)+2, \
				 expr[length] == ',' || expr[length] == '\0') ) { \
				rpnp[steps].op = VV; \
				if ((rpnp[steps].ptr = find_var(im,vname)) == -1) { \
					im_free(im); \
					rrd_set_error("unknown variable '%s'",vname); \
					return NULL; \
				} else { expr+=length; } \
			} \
	}

	match_op(OP_ADD,+)
	match_op(OP_SUB,-)
	match_op(OP_MUL,*)
	match_op(OP_DIV,/)
	match_op(OP_MOD,%)
	match_op(OP_SIN,SIN)
	match_op(OP_COS,COS)
	match_op(OP_LOG,LOG)
	match_op(OP_FLOOR,FLOOR)
	match_op(OP_CEIL,CEIL)
	match_op(OP_EXP,EXP)
	match_op(OP_DUP,DUP)
	match_op(OP_EXC,EXC)
	match_op(OP_POP,POP)
	match_op(OP_LT,LT)
	match_op(OP_LE,LE)
	match_op(OP_GT,GT)
	match_op(OP_GE,GE)
	match_op(OP_EQ,EQ)
	match_op(OP_IF,IF)
	match_op(OP_MIN,MIN)
	match_op(OP_MAX,MAX)
	match_op(OP_LIMIT,LIMIT)
	  /* order is important here ! .. match longest first */
	match_op(OP_UNKN,UNKN)
	match_op(OP_UN,UN)
	match_op(OP_NEGINF,NEGINF)
	match_op(OP_PREV,PREV)
	match_op_param(OP_PREV_OTHER,PREV)
	match_op(OP_INF,INF)
	match_op(OP_NOW,NOW)
	match_op(OP_LTIME,LTIME)
	match_op(OP_TIME,TIME)


#undef match_op


	else if ((sscanf(expr,DEF_NAM_FMT "%n",
			 vname,&pos) == 1) 
		 && ((rpnp[steps].ptr = find_var(im,vname)) != -1)){
	    rpnp[steps].op = OP_VARIABLE;
	    expr+=pos;
	}	   

	else {
	    free(rpnp);
	    return NULL;
	}
	if (*expr == 0)
	  break;
	if (*expr == ',')
	    expr++;
	else {
	    free(rpnp);
	    return NULL;
	}  
    }
    rpnp[steps+1].op = OP_END;
    return rpnp;
}

/* figure out what the local timezone offset for any point in
   time was. Return it in seconds */

int
tzoffset( time_t now ){
  int gm_sec, gm_min, gm_hour, gm_yday, gm_year,
    l_sec, l_min, l_hour, l_yday, l_year;
  struct tm *t;
  int off;
  t = gmtime(&now);
  gm_sec = t->tm_sec;
  gm_min = t->tm_min;
  gm_hour = t->tm_hour;
  gm_yday = t->tm_yday;
  gm_year = t->tm_year;
  t = localtime(&now);
  l_sec = t->tm_sec;
  l_min = t->tm_min;
  l_hour = t->tm_hour;
  l_yday = t->tm_yday;
  l_year = t->tm_year;
  off = (l_sec-gm_sec)+(l_min-gm_min)*60+(l_hour-gm_hour)*3600; 
  if ( l_yday > gm_yday || l_year > gm_year){
        off += 24*3600;
  } else if ( l_yday < gm_yday || l_year < gm_year){
        off -= 24*3600;
  }

  return off;
}

    

#define dc_stackblock 100

/* run the rpn calculator on all the CDEF arguments */

int
data_calc( image_desc_t *im){

    int       gdi,rpi;
    int       dataidx;
    long      *steparray;
    int       stepcnt;
    time_t    now;
    double    *stack = NULL;
    long      dc_stacksize = 0;

    for (gdi=0;gdi<im->gdes_c;gdi++){
	/* only GF_CDEF elements are of interest */
	if (im->gdes[gdi].gf != GF_CDEF) 
	    continue;
	im->gdes[gdi].ds_cnt = 1;
	im->gdes[gdi].ds = 0;
	im->gdes[gdi].data_first = 1;
	im->gdes[gdi].start = 0;
	im->gdes[gdi].end = 0;
	steparray=NULL;
	stepcnt = 0;
	dataidx=-1;

	/* find the variables in the expression. And calc the lowest
           common denominator of all step sizes of the data sources involved.
	   this will be the step size for the cdef created data source*/

	for(rpi=0;im->gdes[gdi].rpnp[rpi].op != OP_END;rpi++){
	    if(im->gdes[gdi].rpnp[rpi].op == OP_VARIABLE ||
		   im->gdes[gdi].rpnp[rpi].op == OP_PREV_OTHER){
		long ptr = im->gdes[gdi].rpnp[rpi].ptr;
		if ((steparray = rrd_realloc(steparray, (++stepcnt+1)*sizeof(*steparray)))==NULL){
		  rrd_set_error("realloc steparray");
		  free(stack);
		  return -1;
		};
	

		steparray[stepcnt-1] = im->gdes[ptr].step;

		/* adjust start and end of cdef (gdi) so that it runs from
		   the latest start point to the earliest endpoint of any of the
		   rras involved (ptr) */

		if(im->gdes[gdi].start < im->gdes[ptr].start)
		    im->gdes[gdi].start = im->gdes[ptr].start;

		if(im->gdes[gdi].end == 0 
		   || im->gdes[gdi].end > im->gdes[ptr].end)
		    im->gdes[gdi].end = im->gdes[ptr].end;
		
		/* store pointer to the first element of the rra providing
		   data for variable, further save step size and data source count
		   of this rra*/ 
		im->gdes[gdi].rpnp[rpi].data = 
		    im->gdes[ptr].data + im->gdes[ptr].ds;
		im->gdes[gdi].rpnp[rpi].step = im->gdes[ptr].step;
		im->gdes[gdi].rpnp[rpi].ds_cnt = im->gdes[ptr].ds_cnt;
	    }

	}
	if(steparray == NULL){
	    rrd_set_error("rpn expressions without variables are not supported");
	    free(stack);
	    return -1;    
	}
	steparray[stepcnt]=0;
	/* now find the step for the result of the cdef. so that we land on
	   each step in all of the variables rras */

	im->gdes[gdi].step = lcd(steparray);
	
	
	free(steparray);

	if((im->gdes[gdi].data = malloc(((im->gdes[gdi].end
				     -im->gdes[gdi].start) 
				    / im->gdes[gdi].step +1)
				    * sizeof(double)))==NULL){
	    rrd_set_error("malloc im->gdes[gdi].data");
	    free(stack);
	    return -1;
	}
	
        /* move the data pointers to the correct period */
        for(rpi=0;im->gdes[gdi].rpnp[rpi].op != OP_END;rpi++){
            if(im->gdes[gdi].rpnp[rpi].op == OP_VARIABLE ||
			   im->gdes[gdi].rpnp[rpi].op == OP_PREV_OTHER){
              long ptr = im->gdes[gdi].rpnp[rpi].ptr;
              if(im->gdes[gdi].start > im->gdes[ptr].start) {
                im->gdes[gdi].rpnp[rpi].data += im->gdes[gdi].rpnp[rpi].ds_cnt;
              }
            }
        }
        
	/* step through the new cdef results array and calculate the values */
	for (now = im->gdes[gdi].start;
	     now<=im->gdes[gdi].end;
	     now += im->gdes[gdi].step){
	    long       stptr=-1;
	    /* process each op from the rpn in turn */
	    for (rpi=0;im->gdes[gdi].rpnp[rpi].op != OP_END;rpi++){
		if (stptr +5 > dc_stacksize){
		    dc_stacksize += dc_stackblock;		
		    stack = rrd_realloc(stack,dc_stacksize*sizeof(*stack));
		    if (stack==NULL){
			rrd_set_error("RPN stack overflow");
			return -1;
		    }
		}
		switch (im->gdes[gdi].rpnp[rpi].op){
		case OP_NUMBER:
		    stack[++stptr] = im->gdes[gdi].rpnp[rpi].val;
		    break;
		case OP_VARIABLE:
		    /* go to the next value of the variable if necessary */
		    stack[++stptr] =  *im->gdes[gdi].rpnp[rpi].data;
		    if ( /* now >  im->gdes[gdi].start &&  */
			now % im->gdes[gdi].rpnp[rpi].step == 0  ){
		       im->gdes[gdi].rpnp[rpi].data += im->gdes[gdi].rpnp[rpi].ds_cnt;
		    }
		    break;
		case OP_PREV:
			if (dataidx <= 0) {
				stack[++stptr] = DNAN;
			} else {
				stack[++stptr] = im->gdes[gdi].data[dataidx];
			}
			break;
		case OP_PREV_OTHER:
			if (dataidx <= 0) {
				stack[++stptr] = DNAN;
			} else {
                            int rpi_ptr = im->gdes[gdi].rpnp[rpi].ptr;
                            int ds_cnt = im->gdes[gdi].rpnp[rpi].ds_cnt;
                            stack[++stptr] = im->gdes[rpi_ptr].data[dataidx*ds_cnt+im->gdes[rpi_ptr].ds];
			}
		    break;
		case OP_UNKN:
		    stack[++stptr] = DNAN; 
		    break;
		case OP_INF:
		    stack[++stptr] = DINF; 
		    break;
		case OP_NEGINF:
		    stack[++stptr] = -DINF; 
		    break;
		case OP_NOW:
		    stack[++stptr] = (double)time(NULL);
		    break;
		case OP_TIME:
		    stack[++stptr] = (double)now;
		    break;
		case OP_LTIME:
		    stack[++stptr] = (double)tzoffset(now)+(double)now;
		    break;
		case OP_ADD:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr-1] = stack[stptr-1] + stack[stptr];
		    stptr--;
		    break;
		case OP_SUB:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr-1] = stack[stptr-1] - stack[stptr];
		    stptr--;
		    break;
		case OP_MUL:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr-1] = stack[stptr-1] * stack[stptr];
		    stptr--;
		    break;
		case OP_DIV:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr-1] = stack[stptr-1] / stack[stptr];
		    stptr--;
		    break;
		case OP_MOD:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr-1] = fmod(stack[stptr-1],stack[stptr]);
		    stptr--;
		    break;
		case OP_SIN:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr] = sin(stack[stptr]);
		    break;
		case OP_COS:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr] = cos(stack[stptr]);
		    break;
		case OP_CEIL:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr] = ceil(stack[stptr]);
		    break;
		case OP_FLOOR:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr] = floor(stack[stptr]);
		    break;
		case OP_LOG:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr] = log(stack[stptr]);
		    break;
		case OP_DUP:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr+1] = stack[stptr];
		    stptr++;
		    break;
		case OP_POP:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stptr--;
		    break;
		case OP_EXC:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    } else {
		      double dummy;
		      dummy = stack[stptr] ;
		      stack[stptr] = stack[stptr-1];
		      stack[stptr-1] = dummy;
		    }
		    break;
		case OP_EXP:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr] = exp(stack[stptr]);
		    break;
		case OP_LT:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    if (isnan(stack[stptr-1]) || isnan(stack[stptr]))
		        stack[stptr-1] = 0.0;
		    else
			stack[stptr-1] = stack[stptr-1] < stack[stptr] ? 1.0 : 0.0;
		    stptr--;
		    break;
		case OP_LE:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    if (isnan(stack[stptr-1]) || isnan(stack[stptr]))
		        stack[stptr-1] = 0.0;
		    else
		        stack[stptr-1] = stack[stptr-1] <= stack[stptr] ? 1.0 : 0.0;
		    stptr--;
		    break;
		case OP_GT:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    if (isnan(stack[stptr-1]) || isnan(stack[stptr]))
		        stack[stptr-1] = 0.0;
		    else
		        stack[stptr-1] = stack[stptr-1] > stack[stptr] ? 1.0 : 0.0;
		    stptr--;
		    break;
		case OP_GE:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    if (isnan(stack[stptr-1]) || isnan(stack[stptr]))
		        stack[stptr-1] = 0.0;
		    else
		        stack[stptr-1] = stack[stptr-1] >= stack[stptr] ? 1.0 : 0.0;
		    stptr--;
		    break;
		case OP_EQ:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    if (isnan(stack[stptr-1]) || isnan(stack[stptr]))
		        stack[stptr-1] = 0.0;
		    else
		        stack[stptr-1] = stack[stptr-1] == stack[stptr] ? 1.0 : 0.0;
		    stptr--;
		    break;
		case OP_IF:
		    if(stptr<2){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr-2] = stack[stptr-2] != 0.0 ? stack[stptr-1] : stack[stptr];
		    stptr--;
		    stptr--;
		    break;
		case OP_MIN:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    if (isnan(stack[stptr-1])) 
			;
		    else if (isnan(stack[stptr]))
		        stack[stptr-1] = stack[stptr];
		    else if (stack[stptr-1] > stack[stptr])
		        stack[stptr-1] = stack[stptr];
		    stptr--;
		    break;
		case OP_MAX:
		    if(stptr<1){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    if (isnan(stack[stptr-1])) 
			;
		    else if (isnan(stack[stptr]))
		        stack[stptr-1] = stack[stptr];
		    else if (stack[stptr-1] < stack[stptr])
		        stack[stptr-1] = stack[stptr];
		    stptr--;
		    break;
		case OP_LIMIT:
		    if(stptr<2){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    if (isnan(stack[stptr-2])) 
			;
		    else if (isnan(stack[stptr-1]))
		        stack[stptr-2] = stack[stptr-1];
		    else if (isnan(stack[stptr]))
		        stack[stptr-2] = stack[stptr];
		    else if (stack[stptr-2] < stack[stptr-1])
		        stack[stptr-2] = DNAN;
		    else if (stack[stptr-2] > stack[stptr])
		        stack[stptr-2] = DNAN;
		    stptr-=2;
		    break;
		case OP_UN:
		    if(stptr<0){
			rrd_set_error("RPN stack underflow");
			free(stack);
			return -1;
		    }
		    stack[stptr] = isnan(stack[stptr]) ? 1.0 : 0.0;
		    break;
		case OP_END:
		    break;
		}
	    }
	    if(stptr!=0){
		rrd_set_error("RPN final stack size != 1");
		free(stack);
		return -1;
	    }
	    im->gdes[gdi].data[++dataidx] = stack[0];
	}
    }
    free(stack);
    return 0;
}

#undef dc_stacksize

/* massage data so, that we get one value for each x coordinate in the graph */
int
data_proc( image_desc_t *im ){
    long i,ii;
    double pixstep = (double)(im->end-im->start)
	/(double)im->xsize; /* how much time 
			       passes in one pixel */
    double paintval;
    double minval=DNAN,maxval=DNAN;
    
    unsigned long gr_time;    

    /* memory for the processed data */
    for(i=0;i<im->gdes_c;i++){
      if((im->gdes[i].gf==GF_LINE1) ||
	 (im->gdes[i].gf==GF_LINE2) ||
	 (im->gdes[i].gf==GF_LINE3) ||
	 (im->gdes[i].gf==GF_AREA) ||
	 (im->gdes[i].gf==GF_STACK)){
	if((im->gdes[i].p_data = malloc((im->xsize +1)
					* sizeof(rrd_value_t)))==NULL){
	  rrd_set_error("malloc data_proc");
	  return -1;
	}
      }
    }
    
    for(i=0;i<im->xsize;i++){
	long vidx;
	gr_time = im->start+pixstep*i; /* time of the 
					  current step */
	paintval=0.0;
	
	for(ii=0;ii<im->gdes_c;ii++){
	  double value;
	    switch(im->gdes[ii].gf){
	    case GF_LINE1:
	    case GF_LINE2:
	    case GF_LINE3:
	    case GF_AREA:
		paintval = 0.0;
	    case GF_STACK:
		vidx = im->gdes[ii].vidx;

		value =
		    im->gdes[vidx].data[
					((unsigned long)floor((double)
							     (gr_time - im->gdes[vidx].start ) 
							     / im->gdes[vidx].step)+1)			

					/* added one because data was not being aligned properly
					   this fixes it. We may also be having a problem in fetch ... */

					*im->gdes[vidx].ds_cnt
					+im->gdes[vidx].ds];

		if (! isnan(value)) {
		  paintval += value;
		  im->gdes[ii].p_data[i] = paintval;
		  if (finite(paintval)){
  		   if (isnan(minval) || paintval <  minval)
		     minval = paintval;
		   if (isnan(maxval) || paintval >  maxval)
		     maxval = paintval;
		  }
		} else {
		  im->gdes[ii].p_data[i] = DNAN;
		}
		break;
	    case GF_PRINT:
	    case GF_GPRINT:
	    case GF_COMMENT:
	    case GF_HRULE:
	    case GF_VRULE:
	    case GF_DEF:	       
	    case GF_CDEF:
	    case GF_XPORT:
		break;
	    }
	}
    }

    /* if min or max have not been asigned a value this is because
       there was no data in the graph ... this is not good ...
       lets set these to dummy values then ... */

    if (isnan(minval)) minval = 0.0;
    if (isnan(maxval)) maxval = 1.0;
    
    /* adjust min and max values */
    if (isnan(im->minval) 
	|| ((!im->logarithmic && !im->rigid) /* don't adjust low-end with log scale */
	    && im->minval > minval))
	im->minval = minval;
    if (isnan(im->maxval) 
	|| (!im->rigid 
	    && im->maxval < maxval)){
	if (im->logarithmic)
	    im->maxval = maxval * 1.1;
	else
	    im->maxval = maxval;
    }
    /* make sure min and max are not equal */
    if (im->minval == im->maxval) {
      im->maxval *= 1.01; 
      if (! im->logarithmic) {
	im->minval *= 0.99;
      }
      
      /* make sure min and max are not both zero */
      if (im->maxval == 0.0) {
	    im->maxval = 1.0;
      }
        
    }
    return 0;
}



/* identify the point where the first gridline, label ... gets placed */

time_t
find_first_time(
    time_t   start, /* what is the initial time */
    enum tmt_en baseint,  /* what is the basic interval */
    long     basestep /* how many if these do we jump a time */
    )
{
    struct tm tm;
    tm = *localtime(&start);
    switch(baseint){
    case TMT_SECOND:
	tm.tm_sec -= tm.tm_sec % basestep; break;
    case TMT_MINUTE: 
	tm.tm_sec=0;
	tm.tm_min -= tm.tm_min % basestep; 
	break;
    case TMT_HOUR:
	tm.tm_sec=0;
	tm.tm_min = 0;
	tm.tm_hour -= tm.tm_hour % basestep; break;
    case TMT_DAY:
	/* we do NOT look at the basestep for this ... */
	tm.tm_sec=0;
	tm.tm_min = 0;
	tm.tm_hour = 0; break;
    case TMT_WEEK:
	/* we do NOT look at the basestep for this ... */
	tm.tm_sec=0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	tm.tm_mday -= tm.tm_wday -1;	/* -1 because we want the monday */
	if (tm.tm_wday==0) tm.tm_mday -= 7; /* we want the *previous* monday */
	break;
    case TMT_MONTH:
	tm.tm_sec=0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	tm.tm_mday = 1;
	tm.tm_mon -= tm.tm_mon % basestep; break;

    case TMT_YEAR:
	tm.tm_sec=0;
	tm.tm_min = 0;
	tm.tm_hour = 0;
	tm.tm_mday = 1;
	tm.tm_mon = 0;
	tm.tm_year -= (tm.tm_year+1900) % basestep;
	
    }
    return mktime(&tm);
}
/* identify the point where the next gridline, label ... gets placed */
time_t 
find_next_time(
    time_t   current, /* what is the initial time */
    enum tmt_en baseint,  /* what is the basic interval */
    long     basestep /* how many if these do we jump a time */
    )
{
    struct tm tm;
    time_t madetime;
    tm = *localtime(&current);
    do {
	switch(baseint){
	case TMT_SECOND:
	    tm.tm_sec += basestep; break;
	case TMT_MINUTE: 
	    tm.tm_min += basestep; break;
	case TMT_HOUR:
	    tm.tm_hour += basestep; break;
	case TMT_DAY:
	    tm.tm_mday += basestep; break;
	case TMT_WEEK:
	    tm.tm_mday += 7*basestep; break;
	case TMT_MONTH:
	    tm.tm_mon += basestep; break;
	case TMT_YEAR:
	    tm.tm_year += basestep;	
	}
	madetime = mktime(&tm);
    } while (madetime == -1); /* this is necessary to skip impssible times
				 like the daylight saving time skips */
    return madetime;
	  
}

void gator( gdImagePtr gif, int x, int y){ 

/* this function puts the name of the author and the tool into the
   graph. Remove if you must, but please note, that it is here,
   because I would like people who look at rrdtool generated graphs to
   see what was used to do it. No obviously you can also add a credit
   line to your webpage or printed document, this is fine with me. But
   as I have no control over this, I added the little tag in here. 
*/

/* the fact that the text of what gets put into the graph is not
   visible in the function, has lead some to think this is for
   obfuscation reasons. While this is a nice side effect (I addmit),
   it is not the prime reason. The prime reason is, that the font
   used, is so small, that I had to hand edit the characters to ensure
   readability. I could thus not use the normal gd functions to write,
   but had to embed a slightly compressed bitmap version into the code. 
*/
    int li[]={0,0,1, 0,4,5, 0,8,9, 0,12,14, 0,17,17, 0,21,21, 
	      0,24,24, 0,34,34, 0,40,42, 0,45,45, 0,48,49, 0,52,54, 
	      0,61,61, 0,64,66, 0,68,70, 0,72,74, 0,76,76, 0,78,78, 
	      0,80,82, 0,84,85, 
	      1,0,0, 1,2,2, 1,4,4, 1,6,6, 1,8,8, 1,10,10, 
	      1,13,13, 1,16,16, 1,18,18, 1,20,20, 1,22,22, 1,24,24, 
	      1,34,34, 1,41,41, 1,44,44, 1,46,46, 1,48,48, 1,50,50, 
	      1,53,53, 1,60,60, 1,62,62, 1,64,64, 1,69,69, 1,73,73, 
	      1,76,76, 1,78,78, 1,80,80, 1,84,84, 1,86,86, 
	      2,0,1, 2,4,5, 2,8,8, 2,10,10, 2,13,13, 2,16,16, 
	      2,18,18, 2,20,20, 2,22,22, 2,24,24, 2,33,33, 2,41,41, 
	      2,44,44, 2,46,46, 2,48,49, 2,53,53, 2,60,60, 2,62,62, 
	      2,64,65, 2,69,69, 2,73,73, 2,76,77, 2,80,81, 2,84,85, 	      
	      3,0,0, 3,2,2, 3,4,4, 3,6,6, 3,8,8, 3,10,10, 
	      3,13,13, 3,16,16, 3,18,18, 3,20,20, 3,22,22, 3,24,24, 
	      3,32,32, 3,41,41, 3,44,44, 3,46,46, 3,48,48, 3,50,50, 
	      3,53,53, 3,60,60, 3,62,62, 3,64,64, 3,69,69, 3,73,73, 
	      3,76,76, 3,78,78, 3,80,80, 3,84,84, 3,86,86, 
	      4,0,0, 4,2,2, 4,4,4, 4,6,6, 4,8,9, 4,13,13, 
	      4,17,17, 4,21,21, 4,24,26, 4,32,32, 4,41,41, 4,45,45, 
	      4,48,49, 4,52,54, 4,61,61, 4,64,66, 4,69,69, 4,72,74, 
	      4,76,76, 4,78,78, 4,80,82, 4,84,84}; 
    int i,ii; 
    for(i=0; i<DIM(li); i=i+3)
	for(ii=y+li[i+1]; ii<=y+li[i+2];ii++)
	  gdImageSetPixel(gif,x-li[i],ii,graph_col[GRC_GRID].i); 
}


/* calculate values required for PRINT and GPRINT functions */

int
print_calc(image_desc_t *im, char ***prdata) 
{
    long i,ii,validsteps;
    double printval;
    int graphelement = 0;
    long vidx;
    int max_ii;	
    double magfact = -1;
    char *si_symb = "";
    char *percent_s;
    int prlines = 1;
    if (im->imginfo) prlines++;
    for(i=0;i<im->gdes_c;i++){
	switch(im->gdes[i].gf){
	case GF_PRINT:
	    prlines++;
	    if(((*prdata) = rrd_realloc((*prdata),prlines*sizeof(char *)))==NULL){
		rrd_set_error("realloc prdata");
		return 0;
	    }
	case GF_GPRINT:
	    vidx = im->gdes[i].vidx;
	    max_ii =((im->gdes[vidx].end 
		      - im->gdes[vidx].start)
		     /im->gdes[vidx].step
		     *im->gdes[vidx].ds_cnt);
	    printval = DNAN;
	    validsteps = 0;
	    for(ii=im->gdes[vidx].ds+im->gdes[vidx].ds_cnt;
		ii < max_ii+im->gdes[vidx].ds_cnt;
		ii+=im->gdes[vidx].ds_cnt){
		 if (! finite(im->gdes[vidx].data[ii]))
		     continue;
		 if (isnan(printval)){
		     printval = im->gdes[vidx].data[ii];
		     validsteps++;
		    continue;
		}

		switch (im->gdes[i].cf){
		case CF_AVERAGE:
		    validsteps++;
		    printval += im->gdes[vidx].data[ii];
		    break;
		case CF_MINIMUM:
		    printval = min( printval, im->gdes[vidx].data[ii]);
		    break;
		case CF_MAXIMUM:
		    printval = max( printval, im->gdes[vidx].data[ii]);
		    break;
		case CF_LAST:
		    printval = im->gdes[vidx].data[ii];
		}
	    }
	    if (im->gdes[i].cf ==  CF_AVERAGE) {
		if (validsteps > 1) {
		    printval = (printval / validsteps);
		}
	    }
	    if ((percent_s = strstr(im->gdes[i].format,"%S")) != NULL) {
		/* Magfact is set to -1 upon entry to print_calc.  If it
		 * is still less than 0, then we need to run auto_scale.
		 * Otherwise, put the value into the correct units.  If
		 * the value is 0, then do not set the symbol or magnification
		 * so next the calculation will be performed again. */
		if (magfact < 0.0) {
		    auto_scale(im,&printval,&si_symb,&magfact);
		    if (printval == 0.0)
			magfact = -1.0;
		} else {
		    printval /= magfact;
		}
		*(++percent_s) = 's';
	    }
	    else if (strstr(im->gdes[i].format,"%s") != NULL) {
		auto_scale(im,&printval,&si_symb,&magfact);
	    }
	    if (im->gdes[i].gf == GF_PRINT){
		(*prdata)[prlines-2] = malloc((FMT_LEG_LEN+2)*sizeof(char));
		(*prdata)[prlines-1] = NULL;
		if (bad_format(im->gdes[i].format)) {
			rrd_set_error("bad format for [G]PRINT in '%s'", im->gdes[i].format);
			return -1;
		}
#ifdef HAVE_SNPRINTF
		snprintf((*prdata)[prlines-2],FMT_LEG_LEN,im->gdes[i].format,printval,si_symb);
#else
		sprintf((*prdata)[prlines-2],im->gdes[i].format,printval,si_symb);
#endif
	    } else {
		/* GF_GPRINT */

		if (bad_format(im->gdes[i].format)) {
			rrd_set_error("bad format for [G]PRINT in '%s'", im->gdes[i].format);
			return -1;
		}
#ifdef HAVE_SNPRINTF
		snprintf(im->gdes[i].legend,FMT_LEG_LEN-2,im->gdes[i].format,printval,si_symb);
#else
		sprintf(im->gdes[i].legend,im->gdes[i].format,printval,si_symb);
#endif
		graphelement = 1;
	    }
	    break;
        case GF_COMMENT:
	case GF_LINE1:
	case GF_LINE2:
	case GF_LINE3:
	case GF_AREA:
	case GF_STACK:
	case GF_HRULE:
	case GF_VRULE:
	    graphelement = 1;
	    break;
	case GF_DEF:
	case GF_CDEF:	    
	case GF_XPORT:	    
	    break;
	}
    }
    return graphelement;
}


/* place legends with color spots */
int
leg_place(image_desc_t *im)
{
    /* graph labels */
    int   interleg = SmallFont->w*2;
    int   box = SmallFont->h*1.2;
    int   border = SmallFont->w*2;
    int   fill=0, fill_last;
    int   leg_c = 0;
    int   leg_x = border, leg_y = im->ygif;
    int   leg_cc;
    int   glue = 0, useglue = 1;
    int   i,ii, mark = 0;
    char  prt_fctn; /*special printfunctions */
    int  *legspace;

    if( !(im->extra_flags & NOLEGEND) ) {
      if ((legspace = malloc(im->gdes_c*sizeof(int)))==NULL){
         rrd_set_error("malloc for legspace");
         return -1;
      }

    for(i=0;i<im->gdes_c;i++){
	fill_last = fill;

	if (im->gdes[i].gf == GF_HRULE &&
	    (im->gdes[i].yrule < im->minval || im->gdes[i].yrule > im->maxval))
	    im->gdes[i].legend[0] = '\0';

	if (im->gdes[i].gf == GF_VRULE &&
	    (im->gdes[i].xrule < im->start || im->gdes[i].xrule > im->end))
	    im->gdes[i].legend[0] = '\0';

	leg_cc = strlen(im->gdes[i].legend);
	
	/* is there a controle code ant the end of the legend string ? */ 
	if (leg_cc >= 2 && im->gdes[i].legend[leg_cc-2] == '\\') {
	    prt_fctn = im->gdes[i].legend[leg_cc-1];
	    leg_cc -= 2;
	    im->gdes[i].legend[leg_cc] = '\0';
	} else {
	    prt_fctn = '\0';
	}
        /* remove exess space */
        while (prt_fctn=='g' && 
	       leg_cc > 0 && 
	       im->gdes[i].legend[leg_cc-1]==' '){
	   leg_cc--;
	   im->gdes[i].legend[leg_cc]='\0';
	}
	if (leg_cc != 0 ){	    
	   legspace[i]=(prt_fctn=='g' ? 0 : interleg);
	   
	   if (fill > 0){ 
 	       /* no interleg space if string ends in \g */
	       fill += legspace[i];
	    }
	    if (im->gdes[i].gf != GF_GPRINT && 
		im->gdes[i].gf != GF_COMMENT) { 
		fill += box; 	   
	    }
	    fill += leg_cc * SmallFont->w;
	    leg_c++;
	} else {
	   legspace[i]=0;
	}
        /* who said there was a special tag ... ?*/
        if (prt_fctn=='J') {    
           useglue = 0;
           prt_fctn = '\0';
        } else if (prt_fctn=='g') {    
	   prt_fctn = '\0';
	} 

	if (prt_fctn == '\0') {
	    if (i == im->gdes_c -1 ) prt_fctn ='l';
	    
	    /* is it time to place the legends ? */
	    if (fill > im->xgif - 2*border){
		if (leg_c > 1) {
		    /* go back one */
		    i--; 
		    fill = fill_last;
		    leg_c--;
		    prt_fctn = 'j';
		} else {
		    prt_fctn = 'l';
		}
		
	    }
	}


	if (prt_fctn != '\0'){
	    leg_x = border;
	    if (leg_c >= 2 && prt_fctn == 'j' && useglue) {
                glue = (im->xgif - fill - 2* border) / (leg_c-1);
	        /* if (glue > 2 * SmallFont->w) glue = 0; */
	    } else {
		glue = 0;
	    }
	    if (prt_fctn =='c') leg_x =  (im->xgif - fill) / 2.0;
	    if (prt_fctn =='r') leg_x =  im->xgif - fill - border;

	    for(ii=mark;ii<=i;ii++){
		if(im->gdes[ii].legend[0]=='\0')
		    continue;
		im->gdes[ii].legloc.x = leg_x;
		im->gdes[ii].legloc.y = leg_y;
		leg_x =  leg_x 
		    + strlen(im->gdes[ii].legend)*SmallFont->w 
		    + legspace[ii]
		    + glue;
		if (im->gdes[ii].gf != GF_GPRINT && 
		    im->gdes[ii].gf != GF_COMMENT) 
		    leg_x += box; 	   
	    }	    
	    leg_y = leg_y + SmallFont->h*1.2;
	    if (prt_fctn == 's') leg_y -= SmallFont->h *0.5;
	    fill = 0;
	    leg_c = 0;
	    mark = ii;
	}	   
    }
    im->ygif = leg_y+6;
    free(legspace);
  }
  return 0;
}

/* create a grid on the graph. it determines what to do
   from the values of xsize, start and end */

/* the xaxis labels are determined from the number of seconds per pixel
   in the requested graph */



int
horizontal_grid(gdImagePtr gif, image_desc_t   *im)
{
    double   range;
    double   scaledrange;
    int      pixel,i;
    int      sgrid,egrid;
    double   gridstep;
    double   scaledstep;
    char     graph_label[100];
    gdPoint  polyPoints[4];
    int      labfact,gridind;
    int      styleMinor[2],styleMajor[2];
    int      decimals, fractionals;
    char     labfmt[64];

    labfact=2;
    gridind=-1;
    range =  im->maxval - im->minval;
    scaledrange = range / im->magfact;

	/* does the scale of this graph make it impossible to put lines
	   on it? If so, give up. */
	if (isnan(scaledrange)) {
		return 0;
	}

    styleMinor[0] = graph_col[GRC_GRID].i;
    styleMinor[1] = gdTransparent;

    styleMajor[0] = graph_col[GRC_MGRID].i;
    styleMajor[1] = gdTransparent;

    /* find grid spaceing */
    pixel=1;
    if(isnan(im->ygridstep)){
	if(im->extra_flags & ALTYGRID) {
	    /* find the value with max number of digits. Get number of digits */
	    decimals = ceil(log10(max(fabs(im->maxval), fabs(im->minval))));
	    if(decimals <= 0) /* everything is small. make place for zero */
		decimals = 1;
	    
	    fractionals = floor(log10(range));
	    if(fractionals < 0) /* small amplitude. */
		sprintf(labfmt, "%%%d.%df", decimals - fractionals + 1, -fractionals + 1);
	    else
		sprintf(labfmt, "%%%d.1f", decimals + 1);
	    gridstep = pow((double)10, (double)fractionals);
	    if(gridstep == 0) /* range is one -> 0.1 is reasonable scale */
		gridstep = 0.1;
	    /* should have at least 5 lines but no more then 15 */
	    if(range/gridstep < 5)
                gridstep /= 10;
	    if(range/gridstep > 15)
                gridstep *= 10;
	    if(range/gridstep > 5) {
		labfact = 1;
		if(range/gridstep > 8)
		    labfact = 2;
	    }
	    else {
		gridstep /= 5;
		labfact = 5;
	    }
	}
	else {
	    for(i=0;ylab[i].grid > 0;i++){
		pixel = im->ysize / (scaledrange / ylab[i].grid);
		if (gridind == -1 && pixel > 5) {
		    gridind = i;
		    break;
		}
	    }
	    
	    for(i=0; i<4;i++) {
		if (pixel * ylab[gridind].lfac[i] >=  2 * SmallFont->h) {
		    labfact =  ylab[gridind].lfac[i];
		    break;
		}		          
	    } 
	    
	    gridstep = ylab[gridind].grid * im->magfact;
	}
    } else {
	gridstep = im->ygridstep;
	labfact = im->ylabfact;
    }
    polyPoints[0].x=im->xorigin;
    polyPoints[1].x=im->xorigin+im->xsize;

    sgrid = (int)( im->minval / gridstep - 1);
    egrid = (int)( im->maxval / gridstep + 1);
    scaledstep = gridstep/im->magfact;
    for (i = sgrid; i <= egrid; i++){
	polyPoints[0].y=ytr(im,gridstep*i);
	if ( polyPoints[0].y >= im->yorigin-im->ysize
	     && polyPoints[0].y <= im->yorigin) {
	    if(i % labfact == 0){
		if (i==0 || im->symbol == ' ') {
		    if(scaledstep < 1){
			if(im->extra_flags & ALTYGRID) {
			    sprintf(graph_label,labfmt,scaledstep*i);
			}
			else {
			    sprintf(graph_label,"%4.1f",scaledstep*i);
			}
		    } else {
			sprintf(graph_label,"%4.0f",scaledstep*i);
		    }
		}else {
		    if(scaledstep < 1){
			sprintf(graph_label,"%4.1f %c",scaledstep*i, im->symbol);
		    } else {
			sprintf(graph_label,"%4.0f %c",scaledstep*i, im->symbol);
		    }
		}

		gdImageString(gif, SmallFont,
                              (polyPoints[0].x - (strlen(graph_label) * 
                                                  SmallFont->w)-7), 
                              polyPoints[0].y - SmallFont->h/2+1,
                              (unsigned char *)graph_label, graph_col[GRC_FONT].i);
		
		gdImageSetStyle(gif, styleMajor, 2);

		gdImageLine(gif, polyPoints[0].x-2,polyPoints[0].y,
			    polyPoints[0].x+2,polyPoints[0].y,graph_col[GRC_MGRID].i);
		gdImageLine(gif, polyPoints[1].x-2,polyPoints[0].y,
			    polyPoints[1].x+2,polyPoints[0].y,graph_col[GRC_MGRID].i);		    

                gdImageLine(gif, polyPoints[0].x,polyPoints[0].y,
                            polyPoints[1].x,polyPoints[0].y,gdStyled);
	    } else if ( !(im->extra_flags & NOMINOR) ) {		
		gdImageSetStyle(gif, styleMinor, 2);
		gdImageLine(gif, polyPoints[0].x-1,polyPoints[0].y,
			    polyPoints[0].x+1,polyPoints[0].y,graph_col[GRC_GRID].i);
		gdImageLine(gif, polyPoints[1].x-1,polyPoints[0].y,
			    polyPoints[1].x+1,polyPoints[0].y,graph_col[GRC_GRID].i);		    
                gdImageLine(gif, polyPoints[0].x,polyPoints[0].y,
                            polyPoints[1].x,polyPoints[0].y,gdStyled);
	    }	    
	}	
    } 
/*    if(im->minval * im->maxval < 0){
      polyPoints[0].y=ytr(0);
      gdImageLine(gif, polyPoints[0].x,polyPoints[0].y,
      polyPoints[1].x,polyPoints[0].y,graph_col[GRC_MGRID].i);
      } */

    return 1;
}

int
horizontal_mrtg_grid(gdImagePtr gif, image_desc_t *im)
{
 int      i, xLeft, xRight, y;
 double   scaledstep;
 char     graph_label[100];
 int      styleMajor[2];
 int      decimals;
 char     labfmt[64];

 /* does the scale of this graph make it impossible to put lines
    on it? If so, give up. */
 if (isnan((im->maxval - im->minval) / im->magfact))
  return 0;

 styleMajor[0] = graph_col[GRC_MGRID].i;
 styleMajor[1] = gdTransparent;

 xLeft = im->xorigin;
 xRight = im->xorigin + im->xsize;

 decimals = ceil(log10(max(fabs(im->maxval),fabs(im->minval))));
 scaledstep = ceil(25 * pow((double)10,-decimals) * im->maxval) * pow((double)10,decimals - 2) / im->magfact;

 sprintf(labfmt,"%%4.%df",1 - (scaledstep > (double)10 || ceil(scaledstep) == scaledstep));
 if (im->symbol != ' ' || im->unit)
  strcat(labfmt," ");
 if (im->symbol != ' ')
  sprintf(labfmt + strlen(labfmt),"%c",im->symbol);
 if (im->unit)
  strcat(labfmt,im->unit);

 for (i = 0; i <= 4; i++)
  {y = im->yorigin - im->ysize * i / 4;
   if (y >= im->yorigin-im->ysize && y <= im->yorigin)
    {sprintf(graph_label,labfmt,scaledstep * i);
     gdImageString(gif,SmallFont,(xLeft - (strlen(graph_label) * SmallFont->w) - 7),
                                 y - SmallFont->h / 2 + 1,
                                 (unsigned char *)graph_label,graph_col[GRC_FONT].i);
     gdImageSetStyle(gif,styleMajor,2);
     gdImageLine(gif,xLeft  - 2,y,xLeft  + 2,y,graph_col[GRC_MGRID].i);
     gdImageLine(gif,xRight - 2,y,xRight + 2,y,graph_col[GRC_MGRID].i);		    
     gdImageLine(gif,xLeft,     y,xRight,    y,gdStyled);
    }	
  } 
 return 1;
}

/* logaritmic horizontal grid */
int
horizontal_log_grid(gdImagePtr gif, image_desc_t   *im)
{
    double   pixpex;
    int      ii,i;
    int      minoridx=0, majoridx=0;
    char     graph_label[100];
    gdPoint  polyPoints[4];
    int      styleMinor[2],styleMajor[2];
    double   value, pixperstep, minstep;

    /* find grid spaceing */
    pixpex= (double)im->ysize / (log10(im->maxval) - log10(im->minval));

	if (isnan(pixpex)) {
		return 0;
	}

    for(i=0;yloglab[i][0] > 0;i++){
	minstep = log10(yloglab[i][0]);
	for(ii=1;yloglab[i][ii+1] > 0;ii++){
	    if(yloglab[i][ii+2]==0){
		minstep = log10(yloglab[i][ii+1])-log10(yloglab[i][ii]);
		break;
	    }
	}
	pixperstep = pixpex * minstep;
	if(pixperstep > 5){minoridx = i;}
	if(pixperstep > 2 * SmallFont->h){majoridx = i;}
    }
   
    styleMinor[0] = graph_col[GRC_GRID].i;
    styleMinor[1] = gdTransparent;

    styleMajor[0] = graph_col[GRC_MGRID].i;
    styleMajor[1] = gdTransparent;

    polyPoints[0].x=im->xorigin;
    polyPoints[1].x=im->xorigin+im->xsize;
    /* paint minor grid */
    for (value = pow((double)10, log10(im->minval) 
			  - fmod(log10(im->minval),log10(yloglab[minoridx][0])));
	 value  <= im->maxval;
	 value *= yloglab[minoridx][0]){
	if (value < im->minval) continue;
	i=0;	
	while(yloglab[minoridx][++i] > 0){	    
	    polyPoints[0].y = ytr(im,value * yloglab[minoridx][i]);
	    if (polyPoints[0].y <= im->yorigin - im->ysize) break;
	    gdImageSetStyle(gif, styleMinor, 2);
	    gdImageLine(gif, polyPoints[0].x-1,polyPoints[0].y,
			polyPoints[0].x+1,polyPoints[0].y,graph_col[GRC_GRID].i);
	    gdImageLine(gif, polyPoints[1].x-1,polyPoints[0].y,
			polyPoints[1].x+1,polyPoints[0].y,graph_col[GRC_GRID].i);	    

	    gdImageLine(gif, polyPoints[0].x,polyPoints[0].y,
			polyPoints[1].x,polyPoints[0].y,gdStyled);
	}
    }

    /* paint major grid and labels*/
    for (value = pow((double)10, log10(im->minval) 
			  - fmod(log10(im->minval),log10(yloglab[majoridx][0])));
	 value <= im->maxval;
	 value *= yloglab[majoridx][0]){
	if (value < im->minval) continue;
	i=0;	
	while(yloglab[majoridx][++i] > 0){	    
	    polyPoints[0].y = ytr(im,value * yloglab[majoridx][i]);	    
	    if (polyPoints[0].y <= im->yorigin - im->ysize) break;
	    gdImageSetStyle(gif, styleMajor, 2);
	    gdImageLine(gif, polyPoints[0].x-2,polyPoints[0].y,
			polyPoints[0].x+2,polyPoints[0].y,graph_col[GRC_MGRID].i);
	    gdImageLine(gif, polyPoints[1].x-2,polyPoints[0].y,
			polyPoints[1].x+2,polyPoints[0].y,graph_col[GRC_MGRID].i);		    
	    
	    gdImageLine(gif, polyPoints[0].x,polyPoints[0].y,
			polyPoints[1].x,polyPoints[0].y,gdStyled);
	    sprintf(graph_label,"%3.0e",value * yloglab[majoridx][i]);
	    gdImageString(gif, SmallFont,
			  (polyPoints[0].x - (strlen(graph_label) * 
					      SmallFont->w)-7), 
			  polyPoints[0].y - SmallFont->h/2+1,
			  (unsigned char *)graph_label, graph_col[GRC_FONT].i);	
	} 
    }
	return 1;
}


void
vertical_grid(
    gdImagePtr     gif,
    image_desc_t   *im )
{   
    int xlab_sel;		/* which sort of label and grid ? */
    time_t ti, tilab;
    long factor;
    char graph_label[100];
    gdPoint polyPoints[4];	 /* points for filled graph and more*/

    /* style for grid lines */
    int     styleDotted[4];

    
    /* the type of time grid is determined by finding
       the number of seconds per pixel in the graph */
    
    
    if(im->xlab_user.minsec == -1){
	factor=(im->end - im->start)/im->xsize;
	xlab_sel=0;
	while ( xlab[xlab_sel+1].minsec != -1 
		&& xlab[xlab_sel+1].minsec <= factor){ xlab_sel++; }
	im->xlab_user.gridtm = xlab[xlab_sel].gridtm;
	im->xlab_user.gridst = xlab[xlab_sel].gridst;
	im->xlab_user.mgridtm = xlab[xlab_sel].mgridtm;
	im->xlab_user.mgridst = xlab[xlab_sel].mgridst;
	im->xlab_user.labtm = xlab[xlab_sel].labtm;
	im->xlab_user.labst = xlab[xlab_sel].labst;
	im->xlab_user.precis = xlab[xlab_sel].precis;
	im->xlab_user.stst = xlab[xlab_sel].stst;
    }
    
    /* y coords are the same for every line ... */
    polyPoints[0].y = im->yorigin;
    polyPoints[1].y = im->yorigin-im->ysize;

    /* paint the minor grid */
    if ( !(im->extra_flags & NOMINOR ))
    {
    for(ti = find_first_time(im->start,
			    im->xlab_user.gridtm,
			    im->xlab_user.gridst);
	ti < im->end; 
	ti = find_next_time(ti,im->xlab_user.gridtm,im->xlab_user.gridst)
	){
	/* are we inside the graph ? */
	if (ti < im->start || ti > im->end) continue;
	polyPoints[0].x = xtr(im,ti);
	styleDotted[0] = graph_col[GRC_GRID].i;
	styleDotted[1] = gdTransparent;

	gdImageSetStyle(gif, styleDotted, 2);

	gdImageLine(gif, polyPoints[0].x,polyPoints[0].y,
		    polyPoints[0].x,polyPoints[1].y,gdStyled);
	gdImageLine(gif, polyPoints[0].x,polyPoints[0].y-1,
		    polyPoints[0].x,polyPoints[0].y+1,graph_col[GRC_GRID].i);
	gdImageLine(gif, polyPoints[0].x,polyPoints[1].y-1,
		    polyPoints[0].x,polyPoints[1].y+1,graph_col[GRC_GRID].i);
    }
    }

    /* paint the major grid */
    for(ti = find_first_time(im->start,
			    im->xlab_user.mgridtm,
			    im->xlab_user.mgridst);
	ti < im->end; 
	ti = find_next_time(ti,im->xlab_user.mgridtm,im->xlab_user.mgridst)
	){
	/* are we inside the graph ? */
	if (ti < im->start || ti > im->end) continue;
	polyPoints[0].x = xtr(im,ti);
	styleDotted[0] = graph_col[GRC_MGRID].i;
	styleDotted[1] = gdTransparent;
	gdImageSetStyle(gif, styleDotted, 2);

	gdImageLine(gif, polyPoints[0].x,polyPoints[0].y,
		    polyPoints[0].x,polyPoints[1].y,gdStyled);
	gdImageLine(gif, polyPoints[0].x,polyPoints[0].y-2,
		    polyPoints[0].x,polyPoints[0].y+2,graph_col[GRC_MGRID].i);
	gdImageLine(gif, polyPoints[0].x,polyPoints[1].y-2,
		    polyPoints[0].x,polyPoints[1].y+2,graph_col[GRC_MGRID].i);
    }
    /* paint the labels below the graph */
    for(ti = find_first_time(im->start,
			    im->xlab_user.labtm,
			    im->xlab_user.labst);
	ti <= im->end; 
	ti = find_next_time(ti,im->xlab_user.labtm,im->xlab_user.labst)
	){
	int gr_pos,width;
        tilab= ti + im->xlab_user.precis/2; /* correct time for the label */

#if HAVE_STRFTIME
	strftime(graph_label,99,im->xlab_user.stst,localtime(&tilab));
#else
# error "your libc has no strftime I guess we'll abort the exercise here."
#endif
	width=strlen(graph_label) *  SmallFont->w;
	gr_pos=xtr(im,tilab) - width/2;
	if (gr_pos  >= im->xorigin 
	    && gr_pos + width <= im->xorigin+im->xsize) 
	    gdImageString(gif, SmallFont,
			  gr_pos,  polyPoints[0].y+4,
			  (unsigned char *)graph_label, graph_col[GRC_FONT].i);
    }

}


void 
axis_paint(
    image_desc_t   *im,
    gdImagePtr     gif
    )
{   
    /* draw x and y axis */
    gdImageLine(gif, im->xorigin+im->xsize,im->yorigin,
		im->xorigin+im->xsize,im->yorigin-im->ysize,
		graph_col[GRC_GRID].i);
    
    gdImageLine(gif, im->xorigin,im->yorigin-im->ysize,
		im->xorigin+im->xsize,im->yorigin-im->ysize,
		graph_col[GRC_GRID].i);

    gdImageLine(gif, im->xorigin-4,im->yorigin,
		im->xorigin+im->xsize+4,im->yorigin,
		graph_col[GRC_FONT].i);

    gdImageLine(gif, im->xorigin,im->yorigin,
		im->xorigin,im->yorigin-im->ysize,
		graph_col[GRC_GRID].i);
    
    /* arrow for X axis direction */
    gdImageLine(gif, im->xorigin+im->xsize+4, im->yorigin-3, im->xorigin+im->xsize+4, im->yorigin+3,graph_col[GRC_ARROW].i);
    gdImageLine(gif, im->xorigin+im->xsize+4, im->yorigin-3, im->xorigin+im->xsize+9, im->yorigin,graph_col[GRC_ARROW].i);
    gdImageLine(gif, im->xorigin+im->xsize+4, im->yorigin+3, im->xorigin+im->xsize+9, im->yorigin,graph_col[GRC_ARROW].i);

    /*    gdImageLine(gif, im->xorigin+im->xsize-1, im->yorigin-3, im->xorigin+im->xsize-1, im->yorigin+3,graph_col[GRC_MGRID].i);
    gdImageLine(gif, im->xorigin+im->xsize, im->yorigin-2, im->xorigin+im->xsize, im->yorigin+2,graph_col[GRC_MGRID].i);
    gdImageLine(gif, im->xorigin+im->xsize+1, im->yorigin-2, im->xorigin+im->xsize+1, im->yorigin+2,graph_col[GRC_MGRID].i);
    gdImageLine(gif, im->xorigin+im->xsize+2, im->yorigin-2, im->xorigin+im->xsize+2, im->yorigin+2,graph_col[GRC_MGRID].i);
    gdImageLine(gif, im->xorigin+im->xsize+3, im->yorigin-1, im->xorigin+im->xsize+3, im->yorigin+1,graph_col[GRC_MGRID].i);
    gdImageLine(gif, im->xorigin+im->xsize+4, im->yorigin-1, im->xorigin+im->xsize+4, im->yorigin+1,graph_col[GRC_MGRID].i);
    gdImageLine(gif, im->xorigin+im->xsize+5, im->yorigin, im->xorigin+im->xsize+5, im->yorigin,graph_col[GRC_MGRID].i); */



}

void
grid_paint(
    image_desc_t   *im,
    gdImagePtr     gif
    )
{   
    long i;
    int boxH=8, boxV=8;
    int res=0;
    gdPoint polyPoints[4];	 /* points for filled graph and more*/

    /* draw 3d border */
    gdImageLine(gif,0,0,im->xgif-1,0,graph_col[GRC_SHADEA].i);
    gdImageLine(gif,1,1,im->xgif-2,1,graph_col[GRC_SHADEA].i);
    gdImageLine(gif,0,0,0,im->ygif-1,graph_col[GRC_SHADEA].i);
    gdImageLine(gif,1,1,1,im->ygif-2,graph_col[GRC_SHADEA].i);
    gdImageLine(gif,im->xgif-1,0,im->xgif-1,im->ygif-1,graph_col[GRC_SHADEB].i);
    gdImageLine(gif,0,im->ygif-1,im->xgif-1,im->ygif-1,graph_col[GRC_SHADEB].i);
    gdImageLine(gif,im->xgif-2,1,im->xgif-2,im->ygif-2,graph_col[GRC_SHADEB].i);
    gdImageLine(gif,1,im->ygif-2,im->xgif-2,im->ygif-2,graph_col[GRC_SHADEB].i);


    if (im->draw_x_grid == 1 )
      vertical_grid(gif, im);
    
    if (im->draw_y_grid == 1){
	if (im->extra_flags & ALTYMRTG)
	{res = horizontal_mrtg_grid(gif,im);
	}
	else
	if(im->logarithmic){
		res = horizontal_log_grid(gif,im);
	} else {
		res = horizontal_grid(gif,im);
	}

	/* dont draw horizontal grid if there is no min and max val */
	if (! res ) {
	  char *nodata = "No Data found";
	  gdImageString(gif, LargeFont,
			im->xgif/2 
			- (strlen(nodata)*LargeFont->w)/2,
			(2*im->yorigin-im->ysize) / 2,
			(unsigned char *)nodata, graph_col[GRC_FONT].i);
	}
    }

    /* yaxis description */
    gdImageStringUp(gif, SmallFont,
		    7,
		    (im->yorigin - im->ysize/2
		     +(strlen(im->ylegend)*SmallFont->w)/2 ),
		    (unsigned char *)im->ylegend, graph_col[GRC_FONT].i);
    

    /* graph title */
    gdImageString(gif, LargeFont,
		    im->xgif/2 
		    - (strlen(im->title)*LargeFont->w)/2,
		  8,
		    (unsigned char *)im->title, graph_col[GRC_FONT].i);
    
    /* graph labels */
    if( !(im->extra_flags & NOLEGEND) ) {
      for(i=0;i<im->gdes_c;i++){
	if(im->gdes[i].legend[0] =='\0')
	    continue;
	
	if(im->gdes[i].gf != GF_GPRINT && im->gdes[i].gf != GF_COMMENT){
	    
	    polyPoints[0].x = im->gdes[i].legloc.x;
	    polyPoints[0].y = im->gdes[i].legloc.y+1;
	    polyPoints[1].x = polyPoints[0].x+boxH;
	    polyPoints[2].x = polyPoints[0].x+boxH;
	    polyPoints[3].x = polyPoints[0].x;
	    polyPoints[1].y = polyPoints[0].y;
	    polyPoints[2].y = polyPoints[0].y+boxV;
	    polyPoints[3].y = polyPoints[0].y+boxV;
	    gdImageFilledPolygon(gif,polyPoints,4,im->gdes[i].col.i);
	    gdImagePolygon(gif,polyPoints,4,graph_col[GRC_FRAME].i);
	
	    gdImageString(gif, SmallFont,
			  polyPoints[0].x+boxH+6, 
			  polyPoints[0].y-1,
			  (unsigned char *)im->gdes[i].legend,
			  graph_col[GRC_FONT].i);
	} else {
	    polyPoints[0].x = im->gdes[i].legloc.x;
	    polyPoints[0].y = im->gdes[i].legloc.y;
	    
	    gdImageString(gif, SmallFont,
			  polyPoints[0].x, 
			  polyPoints[0].y,
			  (unsigned char *)im->gdes[i].legend,
			  graph_col[GRC_FONT].i);
	}
      }
    }
    
    
    gator(gif, (int) im->xgif-5, 5);

}


gdImagePtr
MkLineBrush(image_desc_t *im,long cosel, enum gf_en typsel){
  gdImagePtr brush;
  int pen;
  switch (typsel){
  case GF_LINE1:
    brush=gdImageCreate(1,1);
    break;
  case GF_LINE2:
    brush=gdImageCreate(2,2);
    break;
  case GF_LINE3:
    brush=gdImageCreate(3,3);
    break;
  default:
    return NULL;
  }

  gdImageColorTransparent(brush, 
			  gdImageColorAllocate(brush, 0, 0, 0));

  pen = gdImageColorAllocate(brush, 
			     im->gdes[cosel].col.red,
			     im->gdes[cosel].col.green,
			     im->gdes[cosel].col.blue);
    
  switch (typsel){
  case GF_LINE1:
    gdImageSetPixel(brush,0,0,pen);
    break;
  case GF_LINE2:
    gdImageSetPixel(brush,0,0,pen);
    gdImageSetPixel(brush,0,1,pen);
    gdImageSetPixel(brush,1,0,pen);
    gdImageSetPixel(brush,1,1,pen);
    break;
  case GF_LINE3:
    gdImageSetPixel(brush,1,0,pen);
    gdImageSetPixel(brush,0,1,pen);
    gdImageSetPixel(brush,1,1,pen);
    gdImageSetPixel(brush,2,1,pen);
    gdImageSetPixel(brush,1,2,pen);
    break;
  default:
    return NULL;
  }
  return brush;
}

void
copyImage(gdImagePtr gif, char *image, int copy_white)
{FILE *fi;
 if ((fi = fopen(image,"rb")) != (FILE *)NULL)
  {int i1, i2, c1, c2;
   gdImagePtr img;
   img = gdImageCreateFromGd(fi);		/* load image */
   fclose(fi);
   for (i1 = gdImageSX(img); --i1; )
    for (i2 = gdImageSY(img); --i2; )
     {c2 = gdImageGetPixel(img,i1,i2);	/* get pixel */
      if (copy_white || img->red[c2] != 255 || img->green[c2] != 255 || img->blue[c2] != 255)
       {if ((c1 = gdImageColorExact(gif,img->red[c2],img->green[c2],img->blue[c2])) < 0)
         if ((c1 = gdImageColorAllocate(gif,img->red[c2],img->green[c2],img->blue[c2])) < 0)
          c1 = gdImageColorClosest(gif,img->red[c2],img->green[c2],img->blue[c2]);
        gdImageSetPixel(gif,i1,i2,c1);
       }
     }
   gdImageDestroy(img);
  }
}

/*****************************************************
 * lazy check make sure we rely need to create this graph
 *****************************************************/

int lazy_check(image_desc_t *im){
    FILE *fd = NULL;
    int size = 1;
    struct stat  gifstat;
    
    if (im->lazy == 0) return 0; /* no lazy option */
    if (stat(im->graphfile,&gifstat) != 0) 
      return 0; /* can't stat */
    /* one pixel in the existing graph is more then what we would
       change here ... */
    if (time(NULL) - gifstat.st_mtime > 
	(im->end - im->start) / im->xsize) 
      return 0;
    if ((fd = fopen(im->graphfile,"rb")) == NULL) 
      return 0; /* the file does not exist */
    switch (im->imgformat) {
    case IF_GIF:
	size = GifSize(fd,&(im->xgif),&(im->ygif));
	break;
    case IF_PNG:
	size = PngSize(fd,&(im->xgif),&(im->ygif));
	break;
    case IF_GD:
	size = 0; /* for now - GdSize(fd,&(im->xgif),&(im->ygif)); */
	break;
    }
    fclose(fd);
    return size;
}

/* draw that picture thing ... */
int
graph_paint(image_desc_t *im, char ***calcpr)
{
    int i,ii;
    int lazy =     lazy_check(im);
    FILE  *fo;
    
    /* gif stuff */
    gdImagePtr	gif,brush;

    double areazero = 0.0;
    enum gf_en stack_gf = GF_PRINT;
    graph_desc_t *lastgdes = NULL;    
    gdPoint canvas[4], back[4];	 /* points for canvas*/

    /* if we are lazy and there is nothing to PRINT ... quit now */
    if (lazy && im->prt_c==0) return 0;
    
    /* pull the data from the rrd files ... */
    
    if(data_fetch(im)==-1)
	return -1;

    /* evaluate CDEF  operations ... */
    if(data_calc(im)==-1)
	return -1;

    /* calculate and PRINT and GPRINT definitions. We have to do it at
     * this point because it will affect the length of the legends
     * if there are no graph elements we stop here ... 
     * if we are lazy, try to quit ... 
     */
    i=print_calc(im,calcpr);
    if(i<0) return -1;
    if(i==0 || lazy) return 0;

    /* get actual drawing data and find min and max values*/
    if(data_proc(im)==-1)
	return -1;

    if(!im->logarithmic){si_unit(im);}        /* identify si magnitude Kilo, Mega Giga ? */

    if(!im->rigid && ! im->logarithmic)
	expand_range(im);   /* make sure the upper and lower limit are
			   sensible values */

    /* init xtr and ytr */
    /* determine the actual size of the gif to draw. The size given
       on the cmdline is the graph area. But we need more as we have
       draw labels and other things outside the graph area */


    im->xorigin = 10 + 9 * SmallFont->w+SmallFont->h;
    xtr(im,0); 

    im->yorigin = 14 + im->ysize;
    ytr(im,DNAN);

    if(im->title[0] != '\0')
	im->yorigin += (LargeFont->h+4);

    im->xgif=20+im->xsize + im->xorigin;
    im->ygif= im->yorigin+2*SmallFont->h;
    
    /* determine where to place the legends onto the graphics.
       and set im->ygif to match space requirements for text */
    if(leg_place(im)==-1)
     return -1;

    gif=gdImageCreate(im->xgif,im->ygif);

    gdImageInterlace(gif, im->interlaced);
    
    /* allocate colors for the screen elements */
    for(i=0;i<DIM(graph_col);i++)
	/* check for user override values */
	if(im->graph_col[i].red != -1)
	    graph_col[i].i = 
		gdImageColorAllocate( gif,
				      im->graph_col[i].red, 
				      im->graph_col[i].green, 
				      im->graph_col[i].blue);
	else
	    graph_col[i].i = 
		gdImageColorAllocate( gif,
				      graph_col[i].red, 
				      graph_col[i].green, 
				      graph_col[i].blue);
	
    
    /* allocate colors for the graph */
    for(i=0;i<im->gdes_c;i++)
	/* only for elements which have a color defined */
	if (im->gdes[i].col.red != -1)
	    im->gdes[i].col.i = 
		gdImageColorAllocate(gif,
				     im->gdes[i].col.red,
				     im->gdes[i].col.green,
				     im->gdes[i].col.blue);
    
    /* the actual graph is created by going through the individual
       graph elements and then drawing them */

  if (im->bkg_image)	/* background image added first */
   copyImage(gif,im->bkg_image,1);
  else
   {
    back[0].x = 0;
    back[0].y = 0;
    back[1].x = back[0].x+im->xgif;
    back[1].y = back[0].y;
    back[2].x = back[1].x;
    back[2].y = back[0].y+im->ygif;
    back[3].x = back[0].x;
    back[3].y = back[2].y;

    gdImageFilledPolygon(gif,back,4,graph_col[GRC_BACK].i);
   }

    canvas[0].x = im->xorigin;
    canvas[0].y = im->yorigin;
    canvas[1].x = canvas[0].x+im->xsize;
    canvas[1].y = canvas[0].y;
    canvas[2].x = canvas[1].x;
    canvas[2].y = canvas[0].y-im->ysize;
    canvas[3].x = canvas[0].x;
    canvas[3].y = canvas[2].y;

    gdImageFilledPolygon(gif,canvas,4,graph_col[GRC_CANVAS].i);

    if (im->minval > 0.0)
	areazero = im->minval;
    if (im->maxval < 0.0)
	areazero = im->maxval;

    axis_paint(im,gif);

    for(i=0;i<im->gdes_c;i++){	
        
	switch(im->gdes[i].gf){
	case GF_CDEF:
	case GF_DEF:
	case GF_PRINT:
	case GF_GPRINT:
	case GF_COMMENT:
	case GF_HRULE:
	case GF_VRULE:
	case GF_XPORT:
	  break;
	case GF_LINE1:
	case GF_LINE2:
	case GF_LINE3:
	case GF_AREA:
	    stack_gf = im->gdes[i].gf;
	case GF_STACK:	    
	    /* fix data points at oo and -oo */
	    for(ii=0;ii<im->xsize;ii++){
		if (isinf(im->gdes[i].p_data[ii])){
		    if (im->gdes[i].p_data[ii] > 0) {
			im->gdes[i].p_data[ii] = im->maxval ;
		    } else {
			im->gdes[i].p_data[ii] = im->minval ;
		    }		    
		
		}
	    }

	    if (im->gdes[i].col.i != -1){               
	       /* GF_LINE and frined */
	       if(stack_gf == GF_LINE1 || stack_gf == GF_LINE2 || stack_gf == GF_LINE3 ){
		   brush = MkLineBrush(im,i,stack_gf);
		   gdImageSetBrush(gif, brush);
		   for(ii=1;ii<im->xsize;ii++){
		       if (isnan(im->gdes[i].p_data[ii-1]) ||
			   isnan(im->gdes[i].p_data[ii]))
                            continue;
		       gdImageLine(gif,
                                    ii+im->xorigin-1,ytr(im,im->gdes[i].p_data[ii-1]),
                                    ii+im->xorigin,ytr(im,im->gdes[i].p_data[ii]),
                                    gdBrushed);
                        
                    }
                    gdImageDestroy(brush);
                }
                else 
                    /* GF_AREA STACK type*/
                    if (im->gdes[i].gf == GF_STACK )
                        for(ii=0;ii<im->xsize;ii++){
			    if(isnan(im->gdes[i].p_data[ii])){
				im->gdes[i].p_data[ii] = lastgdes->p_data[ii];
				continue;
			    }
			    
			    if (lastgdes->p_data[ii] == im->gdes[i].p_data[ii]){
				continue;
			    }
			    gdImageLine(gif,
					ii+im->xorigin,ytr(im,lastgdes->p_data[ii]),
					ii+im->xorigin,ytr(im,im->gdes[i].p_data[ii]),
					im->gdes[i].col.i);
			}
	       
		    else /* simple GF_AREA */
			for(ii=0;ii<im->xsize;ii++){
                            if (isnan(im->gdes[i].p_data[ii])) {
				im->gdes[i].p_data[ii] = 0;
                                continue;
			    }
                            gdImageLine(gif,
                                        ii+im->xorigin,ytr(im,areazero),
                                        ii+im->xorigin,ytr(im,im->gdes[i].p_data[ii]),
                                        im->gdes[i].col.i);
                        }
	   }
	   lastgdes = &(im->gdes[i]);	    	   
	   break;
	}
    }
    
    grid_paint(im,gif);

    /* the RULES are the last thing to paint ... */
    for(i=0;i<im->gdes_c;i++){	
        
	switch(im->gdes[i].gf){
	case GF_HRULE:
	    if(im->gdes[i].yrule >= im->minval
	       && im->gdes[i].yrule <= im->maxval)
	      gdImageLine(gif,
			  im->xorigin,ytr(im,im->gdes[i].yrule),
			  im->xorigin+im->xsize,ytr(im,im->gdes[i].yrule),
			  im->gdes[i].col.i); 
	    break;
	case GF_VRULE:
	  if(im->gdes[i].xrule >= im->start
	     && im->gdes[i].xrule <= im->end)
	    gdImageLine(gif,
			xtr(im,im->gdes[i].xrule),im->yorigin,
			xtr(im,im->gdes[i].xrule),im->yorigin-im->ysize,
			im->gdes[i].col.i); 
	  break;
	default:
	    break;
	}
    }

  if (im->ovl_image)	/* overlay image added last */
   copyImage(gif,im->ovl_image,0);

    if (strcmp(im->graphfile,"-")==0) {
#ifdef WIN32
        /* Change translation mode for stdout to BINARY */
        _setmode( _fileno( stdout ), O_BINARY );
#endif
        fo = stdout;
    } else {
	if ((fo = fopen(im->graphfile,"wb")) == NULL) {
	    rrd_set_error("Opening '%s' for write: %s",im->graphfile, strerror(errno));
	    return (-1);
	}
    }
    switch (im->imgformat) {
    case IF_GIF:
	gdImageGif(gif, fo);    
	break;
    case IF_PNG:
	gdImagePng(gif, fo);    
	break;
    case IF_GD:
	gdImageGd(gif, fo);    
	break;
    }
    if (strcmp(im->graphfile,"-") != 0)
	fclose(fo);
    gdImageDestroy(gif);

    return 0;
}


/*****************************************************
 * graph stuff 
 *****************************************************/

int
gdes_alloc(image_desc_t *im){

    long def_step = (im->end-im->start)/im->xsize;
    
    if (im->step > def_step) /* step can be increassed ... no decreassed */
      def_step = im->step;

    im->gdes_c++;
    
    if ((im->gdes = (graph_desc_t *) rrd_realloc(im->gdes, (im->gdes_c)
					   * sizeof(graph_desc_t)))==NULL){
	rrd_set_error("realloc graph_descs");
	return -1;
    }


    im->gdes[im->gdes_c-1].step=def_step; 
    im->gdes[im->gdes_c-1].start=im->start; 
    im->gdes[im->gdes_c-1].end=im->end; 
    im->gdes[im->gdes_c-1].vname[0]='\0'; 
    im->gdes[im->gdes_c-1].data=NULL;
    im->gdes[im->gdes_c-1].ds_namv=NULL;
    im->gdes[im->gdes_c-1].data_first=0;
    im->gdes[im->gdes_c-1].p_data=NULL;
    im->gdes[im->gdes_c-1].rpnp=NULL;
    im->gdes[im->gdes_c-1].col.red = -1;
    im->gdes[im->gdes_c-1].col.i=-1;
    im->gdes[im->gdes_c-1].legend[0]='\0';
    im->gdes[im->gdes_c-1].rrd[0]='\0';
    im->gdes[im->gdes_c-1].ds=-1;    
    im->gdes[im->gdes_c-1].p_data=NULL;    
    return 0;
}

/* copies input untill the first unescaped colon is found
   or until input ends. backslashes have to be escaped as well */
int
scan_for_col(char *input, int len, char *output)
{
    int inp,outp=0;
    for (inp=0; 
	 inp < len &&
	   input[inp] != ':' &&
	   input[inp] != '\0';
	 inp++){
      if (input[inp] == '\\' &&
	  input[inp+1] != '\0' && 
	  (input[inp+1] == '\\' ||
	   input[inp+1] == ':')){
	output[outp++] = input[++inp];
      }
      else {
	output[outp++] = input[inp];
      }
    }
    output[outp] = '\0';
    return inp;
}

int 
rrd_graph(int argc, char **argv, char ***prdata, int *xsize, int *ysize)
{
    
    image_desc_t   im;
    int            i;
    long           long_tmp;
    time_t	   start_tmp=0,end_tmp=0;
    char           scan_gtm[12],scan_mtm[12],scan_ltm[12],col_nam[12];
    char           symname[100];
    unsigned int            col_red,col_green,col_blue;
    long           scancount;
    int linepass = 0; /* stack can only follow directly after LINE* AREA or STACK */    
    struct time_value start_tv, end_tv;
    char *parsetime_error = NULL;
    int stroff;    

    (*prdata)=NULL;

#ifdef HAVE_TZSET
    tzset();
#endif
#ifdef HAVE_SETLOCALE
    setlocale(LC_TIME,"");
#endif
                                
                                
    parsetime("end-24h", &start_tv);
    parsetime("now", &end_tv);

    im.xlab_user.minsec = -1;
    im.xgif=0;
    im.ygif=0;
    im.xsize = 400;
    im.ysize = 100;
    im.step = 0;
    im.ylegend[0] = '\0';
    im.title[0] = '\0';
    im.minval = DNAN;
    im.maxval = DNAN;    
    im.interlaced = 0;
    im.unitsexponent= 9999;
    im.extra_flags= 0;
    im.rigid = 0;
    im.imginfo = NULL;
    im.lazy = 0;
    im.logarithmic = 0;
    im.ygridstep = DNAN;
    im.draw_x_grid = 1;
    im.draw_y_grid = 1;
    im.base = 1000;
    im.prt_c = 0;
    im.gdes_c = 0;
    im.gdes = NULL;
    im.imgformat = IF_GIF; /* we default to GIF output */

im.bkg_image = NULL; /* do not put anything to background per default */
im.ovl_image = NULL; /* do not use anything as overlay per default */
im.unit = 0; /* do not display unit if not set */

    for(i=0;i<DIM(graph_col);i++)
	im.graph_col[i].red=-1;
    
    
    while (1){
	static struct option long_options[] =
	{
	    {"start",      required_argument, 0,  's'},
	    {"end",        required_argument, 0,  'e'},
	    {"x-grid",     required_argument, 0,  'x'},
	    {"y-grid",     required_argument, 0,  'y'},
	    {"vertical-label",required_argument,0,'v'},
	    {"width",      required_argument, 0,  'w'},
	    {"height",     required_argument, 0,  'h'},
	    {"interlaced", no_argument,       0,  'i'},
	    {"upper-limit",required_argument, 0,  'u'},
	    {"lower-limit",required_argument, 0,  'l'},
	    {"rigid",      no_argument,       0,  'r'},
	    {"base",       required_argument, 0,  'b'},
	    {"logarithmic",no_argument,       0,  'o'},
	    {"color",      required_argument, 0,  'c'},
	    {"title",      required_argument, 0,  't'},
	    {"imginfo",    required_argument, 0,  'f'},
	    {"imgformat",  required_argument, 0,  'a'},
	    {"lazy",       no_argument,       0,  'z'},
	    {"no-legend",  no_argument,       0,  'g'},
	    {"alt-y-grid", no_argument,       0,   'Y'},
            {"no-minor",   no_argument,       0,   'N'},
 	    {"alt-autoscale", no_argument,    0,   'A'},
	    {"alt-autoscale-max", no_argument,    0,   'M'},
	    {"units-exponent",required_argument, 0,  'X'},
	    {"step",       required_argument, 0,  'S'},
	    {"alt-y-mrtg", no_argument,       0,  'R'},
	    {"background", required_argument, 0,  'B'},
	    {"overlay",    required_argument, 0,  'O'},
	    {"unit",       required_argument, 0,  'U'},
	    {0,0,0,0}};
	int option_index = 0;
	int opt;

	
	opt = getopt_long(argc, argv, 
			  "s:e:x:y:v:w:h:iu:l:rb:oc:t:f:a:zgYAMX:S:RB:O:U:",
			  long_options, &option_index);

	if (opt == EOF)
	    break;
	
	switch(opt) {
	case 'Y':
	    im.extra_flags |= ALTYGRID;
	    break;
        case 'N':
            im.extra_flags |= NOMINOR;
            break;
	case 'A':
	    im.extra_flags |= ALTAUTOSCALE;
	    break;
	case 'M':
	    im.extra_flags |= ALTAUTOSCALE_MAX;
	    break;
	case 'g':
	    im.extra_flags |= NOLEGEND;
	    break;
	case 'X':
	    im.unitsexponent = atoi(optarg);
	    break;
	case 'S':
	    im.step =  atoi(optarg);
	    break;
	case 'R':
	    im.extra_flags |= ALTYMRTG;
	    break;
	case 's':
	    if ((parsetime_error = parsetime(optarg, &start_tv))) {
	        rrd_set_error( "start time: %s", parsetime_error );
		return -1;
	    }
	    break;
	case 'e':
	    if ((parsetime_error = parsetime(optarg, &end_tv))) {
	        rrd_set_error( "end time: %s", parsetime_error );
		return -1;
	    }
	    break;
	case 'x':
	    if(strcmp(optarg,"none") == 0){
	      im.draw_x_grid=0;
	      break;
	    };
	        
	    if(sscanf(optarg,
		      "%10[A-Z]:%ld:%10[A-Z]:%ld:%10[A-Z]:%ld:%ld:%n",
		      scan_gtm,
		      &im.xlab_user.gridst,
		      scan_mtm,
		      &im.xlab_user.mgridst,
		      scan_ltm,
		      &im.xlab_user.labst,
		      &im.xlab_user.precis,
		      &stroff) == 7 && stroff != 0){
                strncpy(im.xlab_form, optarg+stroff, sizeof(im.xlab_form) - 1);
		if((im.xlab_user.gridtm = tmt_conv(scan_gtm)) == -1){
		    rrd_set_error("unknown keyword %s",scan_gtm);
		    return -1;
		} else if ((im.xlab_user.mgridtm = tmt_conv(scan_mtm)) == -1){
		    rrd_set_error("unknown keyword %s",scan_mtm);
		    return -1;
		} else if ((im.xlab_user.labtm = tmt_conv(scan_ltm)) == -1){
		    rrd_set_error("unknown keyword %s",scan_ltm);
		    return -1;
		} 
		im.xlab_user.minsec = 1;
		im.xlab_user.stst = im.xlab_form;
	    } else {
		rrd_set_error("invalid x-grid format");
		return -1;
	    }
	    break;
	case 'y':

	    if(strcmp(optarg,"none") == 0){
	      im.draw_y_grid=0;
	      break;
	    };

	    if(sscanf(optarg,
		      "%lf:%d",
		      &im.ygridstep,
		      &im.ylabfact) == 2) {
		if(im.ygridstep<=0){
		    rrd_set_error("grid step must be > 0");
		    return -1;
		} else if (im.ylabfact < 1){
		    rrd_set_error("label factor must be > 0");
		    return -1;
		} 
	    } else {
		rrd_set_error("invalid y-grid format");
		return -1;
	    }
	    break;
	case 'v':
	    strncpy(im.ylegend,optarg,150);
	    im.ylegend[150]='\0';
	    break;
	case 'u':
	    im.maxval = atof(optarg);
	    break;
	case 'l':
	    im.minval = atof(optarg);
	    break;
	case 'b':
	    im.base = atol(optarg);
	    if(im.base != 1024 && im.base != 1000 ){
		rrd_set_error("the only sensible value for base apart from 1000 is 1024");
		return -1;
	    }
	    break;
	case 'w':
	    long_tmp = atol(optarg);
	    if (long_tmp < 10) {
		rrd_set_error("width below 10 pixels");
		return -1;
	    }
	    im.xsize = long_tmp;
	    break;
	case 'h':
	    long_tmp = atol(optarg);
	    if (long_tmp < 10) {
		rrd_set_error("height below 10 pixels");
		return -1;
	    }
	    im.ysize = long_tmp;
	    break;
	case 'i':
	    im.interlaced = 1;
	    break;
	case 'r':
	    im.rigid = 1;
	    break;
	case 'f':
	    im.imginfo = optarg;
	    break;
    	case 'a':
	    if((im.imgformat = if_conv(optarg)) == -1) {
		rrd_set_error("unsupported graphics format '%s'",optarg);
		return -1;
	    }
	    break;
	case 'z':
	    im.lazy = 1;
	    break;
	case 'o':
	    im.logarithmic = 1;
	    if (isnan(im.minval))
		im.minval=1;
	    break;
	case 'c':
	    if(sscanf(optarg,
		      "%10[A-Z]#%2x%2x%2x",
		      col_nam,&col_red,&col_green,&col_blue) == 4){
		int ci;
		if((ci=grc_conv(col_nam)) != -1){
		    im.graph_col[ci].red=col_red;
		    im.graph_col[ci].green=col_green;
		    im.graph_col[ci].blue=col_blue;
		}  else {
		  rrd_set_error("invalid color name '%s'",col_nam);
		}
	    } else {
		rrd_set_error("invalid color def format");
		return -1;
	    }
	    break;	  
	case 't':
	    strncpy(im.title,optarg,150);
	    im.title[150]='\0';
	    break;
	case 'B':
	    im.bkg_image = optarg;
	    break;
	case 'O':
	    im.ovl_image = optarg;
	    break;
	case 'U':
	    im.unit = optarg;
	    break;

	case '?':
            if (optopt != 0)
                rrd_set_error("unknown option '%c'", optopt);
            else
                rrd_set_error("unknown option '%s'",argv[optind-1]);
            return -1;
	}
    }
    
    if (optind >= argc) {
       rrd_set_error("missing filename");
       return -1;
    }

    if (im.logarithmic == 1 && (im.minval <= 0 || isnan(im.minval))){
	rrd_set_error("for a logarithmic yaxis you must specify a lower-limit > 0");	
	return -1;
    }

    strncpy(im.graphfile,argv[optind],MAXPATH-1);
    im.graphfile[MAXPATH-1]='\0';

    if (proc_start_end(&start_tv,&end_tv,&start_tmp,&end_tmp) == -1){
	return -1;
    }  
    
    if (start_tmp < 3600*24*365*10){
	rrd_set_error("the first entry to fetch should be after 1980 (%ld)",start_tmp);
	return -1;
    }
    
    if (end_tmp < start_tmp) {
	rrd_set_error("start (%ld) should be less than end (%ld)", 
	       start_tmp, end_tmp);
	return -1;
    }
    
    im.start = start_tmp;
    im.end = end_tmp;

    
    for(i=optind+1;i<argc;i++){
	int   argstart=0;
	int   strstart=0;
	char  varname[30],*rpnex;
	gdes_alloc(&im);
	if(sscanf(argv[i],"%10[A-Z0-9]:%n",symname,&argstart)==1){
	    if((im.gdes[im.gdes_c-1].gf=gf_conv(symname))==-1){
		im_free(&im);
		rrd_set_error("unknown function '%s'",symname);
		return -1;
	    }
	} else {
	    rrd_set_error("can't parse '%s'",argv[i]);
	    im_free(&im);
	    return -1;
	}

	/* reset linepass if a non LINE/STACK/AREA operator gets parsed 
	
	   if (im.gdes[im.gdes_c-1].gf != GF_LINE1 &&
	   im.gdes[im.gdes_c-1].gf != GF_LINE2 &&
	   im.gdes[im.gdes_c-1].gf != GF_LINE3 &&
	   im.gdes[im.gdes_c-1].gf != GF_AREA &&
	   im.gdes[im.gdes_c-1].gf != GF_STACK) {
	   linepass = 0;
	   } 
	*/
	
	switch(im.gdes[im.gdes_c-1].gf){
	case GF_XPORT:
	  break;
	case GF_PRINT:
	    im.prt_c++;
	case GF_GPRINT:
	    if(sscanf(
		&argv[i][argstart],
		"%29[^#:]:" CF_NAM_FMT ":%n",
		varname,symname,&strstart) == 2){
		scan_for_col(&argv[i][argstart+strstart],FMT_LEG_LEN,im.gdes[im.gdes_c-1].format);
		if((im.gdes[im.gdes_c-1].vidx=find_var(&im,varname))==-1){
		    im_free(&im);
		    rrd_set_error("unknown variable '%s'",varname);
		    return -1;
		}	
		if((im.gdes[im.gdes_c-1].cf=cf_conv(symname))==-1){
		    im_free(&im);
		    return -1;
		}
		
	    } else {
		im_free(&im);
		rrd_set_error("can't parse '%s'",&argv[i][argstart]);
		return -1;
	    }
	    break;
	case GF_COMMENT:
	    if(strlen(&argv[i][argstart])>FMT_LEG_LEN) argv[i][argstart+FMT_LEG_LEN-3]='\0' ;
	    strcpy(im.gdes[im.gdes_c-1].legend, &argv[i][argstart]);
	    break;
	case GF_HRULE:
	    if(sscanf(
		&argv[i][argstart],
		"%lf#%2x%2x%2x:%n",
		&im.gdes[im.gdes_c-1].yrule,
		&col_red,&col_green,&col_blue,
		&strstart) >=  4){
		im.gdes[im.gdes_c-1].col.red = col_red;
		im.gdes[im.gdes_c-1].col.green = col_green;
		im.gdes[im.gdes_c-1].col.blue = col_blue;
		if(strstart <= 0){
		    im.gdes[im.gdes_c-1].legend[0] = '\0';
		} else { 
		    scan_for_col(&argv[i][argstart+strstart],FMT_LEG_LEN,im.gdes[im.gdes_c-1].legend);
		}
	    } else {
		im_free(&im);
		rrd_set_error("can't parse '%s'",&argv[i][argstart]);
		return -1;
	    } 
	    break;
	case GF_VRULE:
	    if(sscanf(
		&argv[i][argstart],
		"%lu#%2x%2x%2x:%n",
		&im.gdes[im.gdes_c-1].xrule,
		&col_red,
		&col_green,
		&col_blue,
		&strstart) >=  4){
		im.gdes[im.gdes_c-1].col.red = col_red;
		im.gdes[im.gdes_c-1].col.green = col_green;
		im.gdes[im.gdes_c-1].col.blue = col_blue;
		if(strstart <= 0){                    
		    im.gdes[im.gdes_c-1].legend[0] = '\0';
		} else { 
		    scan_for_col(&argv[i][argstart+strstart],FMT_LEG_LEN,im.gdes[im.gdes_c-1].legend);
		}
	    } else {
		im_free(&im);
		rrd_set_error("can't parse '%s'",&argv[i][argstart]);
		return -1;
	    }
	    break;
	case GF_STACK:
	    if(linepass == 0){
		im_free(&im);
		rrd_set_error("STACK must follow AREA, LINE or STACK");
		return -1; 
	    }		
	case GF_LINE1:
	case GF_LINE2:
	case GF_LINE3:
	case GF_AREA:
	    linepass = 1;
	    if((scancount=sscanf(
		&argv[i][argstart],
		"%29[^:#]#%2x%2x%2x:%n",
		varname,
		&col_red,
		&col_green,
		    &col_blue,
		&strstart))>=1){
		im.gdes[im.gdes_c-1].col.red = col_red;
		im.gdes[im.gdes_c-1].col.green = col_green;
		im.gdes[im.gdes_c-1].col.blue = col_blue;
		if(strstart <= 0){
		    im.gdes[im.gdes_c-1].legend[0] = '\0';
		} else { 
		    scan_for_col(&argv[i][argstart+strstart],FMT_LEG_LEN,im.gdes[im.gdes_c-1].legend);
		}
		if((im.gdes[im.gdes_c-1].vidx=find_var(&im,varname))==-1){
		    im_free(&im);
		    rrd_set_error("unknown variable '%s'",varname);
		    return -1;
		}		
		if (scancount < 4)
		    im.gdes[im.gdes_c-1].col.red = -1;		
		
	    } else {
		im_free(&im);
		rrd_set_error("can't parse '%s'",&argv[i][argstart]);
		return -1;
	    }
	    break;
	case GF_CDEF:
	    if((rpnex = malloc(strlen(&argv[i][argstart])*sizeof(char)))==NULL){
		rrd_set_error("malloc for CDEF");
		return -1;
	    }
	    if(sscanf(
		    &argv[i][argstart],
		    DEF_NAM_FMT "=%[^: ]",
		    im.gdes[im.gdes_c-1].vname,
		    rpnex) != 2){
		im_free(&im);
		free(rpnex);
		rrd_set_error("can't parse CDEF '%s'",&argv[i][argstart]);
		return -1;
	    }
	    /* checking for duplicate DEF CDEFS */
	    if(find_var(&im,im.gdes[im.gdes_c-1].vname) != -1){
		im_free(&im);
		rrd_set_error("duplicate variable '%s'",
			      im.gdes[im.gdes_c-1].vname);
		return -1; 
	    }	   
	    if((im.gdes[im.gdes_c-1].rpnp = str2rpn(&im,rpnex))== NULL){
		rrd_set_error("invalid rpn expression '%s'", rpnex);
		im_free(&im);		
		return -1;
	    }
	    free(rpnex);
	    break;
	case GF_DEF:
	    if (sscanf(
		&argv[i][argstart],
		DEF_NAM_FMT "=%n",
		im.gdes[im.gdes_c-1].vname,
		&strstart)== 1 && strstart){ /* is the = did not match %n returns 0 */ 
		if(sscanf(&argv[i][argstart
				  +strstart
				  +scan_for_col(&argv[i][argstart+strstart],
						MAXPATH,im.gdes[im.gdes_c-1].rrd)],
			  ":" DS_NAM_FMT ":" CF_NAM_FMT,
			  im.gdes[im.gdes_c-1].ds_nam,
			  symname) != 2){
		    im_free(&im);
		    rrd_set_error("can't parse DEF '%s' -2",&argv[i][argstart]);
		    return -1;
		}
	    } else {
		im_free(&im);
		rrd_set_error("can't parse DEF '%s'",&argv[i][argstart]);
		return -1;
	    }
	    
	    /* checking for duplicate DEF CDEFS */
	    if (find_var(&im,im.gdes[im.gdes_c-1].vname) != -1){
		im_free(&im);
		rrd_set_error("duplicate variable '%s'",
			  im.gdes[im.gdes_c-1].vname);
		return -1; 
	    }	   
	    if((im.gdes[im.gdes_c-1].cf=cf_conv(symname))==-1){
		im_free(&im);
		rrd_set_error("unknown cf '%s'",symname);
		return -1;
	    }
	    break;
	}
	
    }

    if (im.gdes_c==0){
	rrd_set_error("can't make a graph without contents");
	im_free(&im);
	return(-1); 
    }
    
	/* parse rest of arguments containing information on what to draw*/
    if (graph_paint(&im,prdata)==-1){
	im_free(&im);
	return -1;
    }
    
    *xsize=im.xgif;
    *ysize=im.ygif;
    if (im.imginfo){
      char *filename;
      if (! (*prdata)) {	
	/* maybe prdata is not allocated yet ... lets do it now */
	if((*prdata = calloc(2,sizeof(char *)))==NULL){
	  rrd_set_error("malloc imginfo");
	  return -1; 
	};
      }
      if(((*prdata)[0] = malloc((strlen(im.imginfo)+200+strlen(im.graphfile))*sizeof(char)))
	 ==NULL){
	rrd_set_error("malloc imginfo");
	return -1;
      }
      filename=im.graphfile+strlen(im.graphfile);      
      while(filename > im.graphfile){
	if (*(filename-1)=='/' || *(filename-1)=='\\' ) break;
	filename--;
      }
      
      sprintf((*prdata)[0],im.imginfo,filename,im.xgif,im.ygif);
    }
    im_free(&im);
    return 0;
}

int bad_format(char *fmt) {
     char *ptr;
     int n=0;
     ptr = fmt;
     while (*ptr != '\0') {
          if (*ptr++ == '%') {
 
             /* line cannot end with percent char */
             if (*ptr == '\0') return 1;
 
             /* '%s', '%S' and '%%' are allowed */
             if (*ptr == 's' || *ptr == 'S' || *ptr == '%') ptr++;
     
             /* or else '% 6.2lf' and such are allowed */
             else {
   
                 /* optional padding character */
                 if (*ptr == ' ' || *ptr == '+' || *ptr == '-') ptr++;
    
                 /* This should take care of 'm.n' with all three optional */
                 while (*ptr >= '0' && *ptr <= '9') ptr++;
                 if (*ptr == '.') ptr++;
                 while (*ptr >= '0' && *ptr <= '9') ptr++;
     
                 /* Either 'le', 'lf' or 'lg' must follow here */
                 if (*ptr++ != 'l') return 1;
                 if (*ptr == 'e' || *ptr == 'f' || *ptr == 'g') ptr++;
                 else return 1;
                 n++;
             }
          }
      }
      return (n!=1);
}
