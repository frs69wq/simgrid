/* Copyright (c) 2008, 2009, 2010. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

//teshsuite/simdag/platforms/evaluate_parse_time ../examples/platforms/nancy.xml

#include <stdio.h>
#include <stdlib.h>
#include "simdag/simdag.h"
#include "surf/surf_private.h"
#include <time.h>

#define BILLION  1000000000L;
extern routing_global_t global_routing;

int main(int argc, char **argv)
{
	struct timespec start, stop;
	double accum;

	/* initialisation of SD */
	SD_init(&argc, argv);

#if _POSIX_TIMERS > 0
	if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
	perror( "clock gettime" );
	return EXIT_FAILURE;
	}
#else
      struct timeval tv;
      gettimeofday(&tv, NULL);
      start.tv_sec = tv.tv_sec;
      start.tv_nsec = tv.tv_usec*1000;
#endif
	/* creation of the environment */
	SD_create_environment(argv[1]);

#if _POSIX_TIMERS > 0
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
	perror( "clock gettime" );
	return EXIT_FAILURE;
	}
#else
      gettimeofday(&tv, NULL);
      stop.tv_sec = tv.tv_sec;
      stop.tv_nsec = tv.tv_usec*1000;
#endif

	accum = ( stop.tv_sec - start.tv_sec )
		   + (double)( stop.tv_nsec - start.tv_nsec )
			 / (double)BILLION;

	printf( "%lf\n", accum );

	sleep(20);

	SD_exit();

	return 0;
}