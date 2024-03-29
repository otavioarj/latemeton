/*****************************************************************************
 * RRDtool 1.0.42  Copyright Tobias Oetiker, 1997,1998, 1999
 *****************************************************************************
 * rrdlib.h   Public header file for librrd
 *****************************************************************************
 * $Id: rrd.h,v 1.2 2003/05/05 12:19:21 deri Exp $
 * $Log: rrd.h,v $
 * Revision 1.2  2003/05/05 12:19:21  deri
 * Added patch for OSX
 *
 * Revision 1.1  2003/04/22 17:02:29  deri
 * Local RRD tree
 *
 * Revision 1.1.1.1  2002/02/26 10:21:37  oetiker
 * Intial Import
 *
 *****************************************************************************/
#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _RRDLIB_H
#define _RRDLIB_H

#include <time.h>

/* Transplanted from rrd_format.h */
typedef double       rrd_value_t;         /* the data storage type is
                                           * double */
/* END rrd_format.h */

/* main function blocks */
int    rrd_create(int, char **);
int    rrd_update(int, char **);
int    rrd_graph(int, char **, char ***, int *, int *);
int    rrd_fetch(int, char **, time_t *, time_t *, unsigned long *,
		 unsigned long *, char ***, rrd_value_t **);
int    rrd_restore(int, char **);
int    rrd_dump(int, char **);
int    rrd_tune(int, char **);
time_t rrd_last(int, char **);
int    rrd_resize(int, char **);
int    rrd_xport(int, char **, int *, time_t *, time_t *,
		 unsigned long *, unsigned long *,
		 char ***, rrd_value_t **);

/* Transplanted from parsetime.h */
typedef enum {
        ABSOLUTE_TIME,
        RELATIVE_TO_START_TIME, 
        RELATIVE_TO_END_TIME
} timetype;

#define TIME_OK NULL

#if !defined(DARWIN)
struct time_value {
  timetype type;
  long offset;
  struct tm tm;
};
#endif

char *parsetime(char *spec, struct time_value *ptv);
/* END parsetime.h */

int proc_start_end (struct time_value *,  struct time_value *, time_t *, time_t *);

/* HELPER FUNCTIONS */
void rrd_set_error(char *,...);
void rrd_clear_error(void);
int  rrd_test_error(void);
char *rrd_get_error(void);
int  LockRRD(FILE *);

#endif /* _RRDLIB_H */

#ifdef  __cplusplus
}
#endif
