#include <stdio.h>																								// per NULL
#include <sys/time.h>																							// per gettimeofday

#define MAX_TIMERS 32

struct timeval bench_timer[MAX_TIMERS];																	// timeval structure, needed for bench

int bench(int i) {
	struct timeval t, tmp;
	double t_old, t_new;
	
	if (i >= MAX_TIMERS)	return -1;																			// if asked timer don't exist, return
	
	t = bench_timer[i];																							// easy pointer to old timeval value
	
	if (gettimeofday(&tmp,NULL)==-1) return -1;															// take current time
	
	t_new = (double) ((tmp.tv_sec * 1000000) + tmp.tv_usec);											// calculate ms of current time
	t_old = (double) ((t.tv_sec * 1000000) + t.tv_usec);												// calculate ms of old time
	
	if (gettimeofday(&bench_timer[i],NULL)==-1) return -1;											// save new timeval value before return
	
	return (int) (t_new - t_old);
}
