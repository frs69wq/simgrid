/* platf_interface.h - Internal interface to the SimGrid platforms          */

/* Copyright (c) 2004, 2005, 2006, 2007, 2009, 2010, 2011. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SG_PLATF_INTERFACE_H
#define SG_PLATF_INTERFACE_H

#include "simgrid/platf.h" /* public interface */

/* Module management functions */
void sg_platf_init(void);
void sg_platf_exit(void);

/* Managing the parsing callbacks */

typedef void (*sg_platf_host_cb_t)(sg_platf_host_cbarg_t);
typedef void (*sg_platf_router_cb_t)(sg_platf_router_cbarg_t);
void sg_platf_host_add_cb(sg_platf_host_cb_t);
void sg_platf_router_add_cb(sg_platf_router_cb_t);




#endif                          /* SG_PLATF_INTERFACE_H */