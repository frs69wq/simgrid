/* Copyright (c) 2009, 2010, 2011. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "surf_routing_private.h"

/* Global vars */
extern routing_global_t global_routing;
extern routing_component_t current_routing;
extern model_type_t current_routing_model;

XBT_LOG_NEW_DEFAULT_SUBCATEGORY(surf_route_full, surf, "Routing part of surf");

#define TO_ROUTE_FULL(i,j) routing->routing_table[(i)+(j)*table_size]

/* Routing model structure */

typedef struct {
  s_routing_component_t generic_routing;
  route_extended_t *routing_table;
} s_routing_component_full_t, *routing_component_full_t;

/* Business methods */
static xbt_dynar_t full_get_onelink_routes(routing_component_t rc)
{
  xbt_dynar_t ret = xbt_dynar_new(sizeof(onelink_t), xbt_free);

  routing_component_full_t routing = (routing_component_full_t) rc;
  size_t table_size = xbt_dict_length(routing->generic_routing.to_index);
  xbt_dict_cursor_t c1 = NULL, c2 = NULL;
  char *k1, *d1, *k2, *d2;
  xbt_dict_foreach(routing->generic_routing.to_index, c1, k1, d1) {
    xbt_dict_foreach(routing->generic_routing.to_index, c2, k2, d2) {
      int *src_id = xbt_dict_get_or_null(routing->generic_routing.to_index, k1);
      int *dst_id = xbt_dict_get_or_null(routing->generic_routing.to_index, k2);
      xbt_assert(src_id
                  && dst_id,
                  "Ask for route \"from\"(%s)  or \"to\"(%s) no found in the local table",
                  k1, k2);
      route_extended_t route = TO_ROUTE_FULL(*src_id, *dst_id);
      if (route) {
        if (xbt_dynar_length(route->generic_route.link_list) == 1) {
          void *link =
              *(void **) xbt_dynar_get_ptr(route->generic_route.link_list,
                                           0);
          onelink_t onelink = xbt_new0(s_onelink_t, 1);
          onelink->link_ptr = link;
          if (routing->generic_routing.hierarchy == SURF_ROUTING_BASE) {
            onelink->src = xbt_strdup(k1);
            onelink->dst = xbt_strdup(k2);
          } else if (routing->generic_routing.hierarchy ==
                     SURF_ROUTING_RECURSIVE) {
            onelink->src = xbt_strdup(route->src_gateway);
            onelink->dst = xbt_strdup(route->dst_gateway);
          }
          xbt_dynar_push(ret, &onelink);
        }
      }
    }
  }
  return ret;
}

static route_extended_t full_get_route(routing_component_t rc,
                                       const char *src, const char *dst)
{
  xbt_assert(rc && src
              && dst,
              "Invalid params for \"get_route\" function at AS \"%s\"",
              rc->name);

  /* set utils vars */
  routing_component_full_t routing = (routing_component_full_t) rc;
  size_t table_size = xbt_dict_length(routing->generic_routing.to_index);

  int *src_id = xbt_dict_get_or_null(routing->generic_routing.to_index, src);
  int *dst_id = xbt_dict_get_or_null(routing->generic_routing.to_index, dst);
  xbt_assert(src_id
              && dst_id,
              "Ask for route \"from\"(%s)  or \"to\"(%s) no found in the local table",
              src, dst);

  route_extended_t e_route = NULL;
  route_extended_t new_e_route = NULL;
  void *link;
  unsigned int cpt = 0;

  e_route = TO_ROUTE_FULL(*src_id, *dst_id);

  if (e_route) {
    new_e_route = xbt_new0(s_route_extended_t, 1);
    new_e_route->src_gateway = xbt_strdup(e_route->src_gateway);
    new_e_route->dst_gateway = xbt_strdup(e_route->dst_gateway);
    new_e_route->generic_route.link_list =
        xbt_dynar_new(global_routing->size_of_link, NULL);
    xbt_dynar_foreach(e_route->generic_route.link_list, cpt, link) {
      xbt_dynar_push(new_e_route->generic_route.link_list, &link);
    }
  }
  return new_e_route;
}

static void full_finalize(routing_component_t rc)
{
  routing_component_full_t routing = (routing_component_full_t) rc;
  size_t table_size = xbt_dict_length(routing->generic_routing.to_index);
  int i, j;
  if (routing) {
    /* Delete routing table */
    for (i = 0; i < table_size; i++)
      for (j = 0; j < table_size; j++)
        generic_free_extended_route(TO_ROUTE_FULL(i, j));
    xbt_free(routing->routing_table);
    /* Delete bypass dict */
    xbt_dict_free(&rc->bypassRoutes);
    /* Delete index dict */
    xbt_dict_free(&rc->to_index);
    /* Delete structure */
    xbt_free(rc);
  }
}

/* Creation routing model functions */

void *model_full_create(void)
{
  routing_component_full_t new_component =
      xbt_new0(s_routing_component_full_t, 1);
  new_component->generic_routing.set_processing_unit =
      generic_set_processing_unit;
  new_component->generic_routing.set_autonomous_system =
      generic_set_autonomous_system;
  new_component->generic_routing.set_route = model_full_set_route;
  new_component->generic_routing.set_ASroute = model_full_set_route;
  new_component->generic_routing.set_bypassroute = generic_set_bypassroute;
  new_component->generic_routing.get_route = full_get_route;
  new_component->generic_routing.get_latency = generic_get_link_latency;
  new_component->generic_routing.get_onelink_routes =
      full_get_onelink_routes;
  new_component->generic_routing.get_bypass_route =
      generic_get_bypassroute;
  new_component->generic_routing.finalize = full_finalize;
  new_component->generic_routing.to_index = xbt_dict_new();
  new_component->generic_routing.bypassRoutes = xbt_dict_new();
  new_component->generic_routing.get_network_element_type = get_network_element_type;
  return new_component;
}

void model_full_load(void)
{
  /* use "surfxml_add_callback" to add a parse function call */
}

void model_full_unload(void)
{
  /* use "surfxml_del_callback" to remove a parse function call */
}

void model_full_end(void)
{
  unsigned int i;
  route_extended_t e_route;

  /* set utils vars */
  routing_component_full_t routing =
      ((routing_component_full_t) current_routing);
  size_t table_size = xbt_dict_length(routing->generic_routing.to_index);

  /* Create table if necessary */
  if(!routing->routing_table)
	  routing->routing_table = xbt_new0(route_extended_t, table_size * table_size);

  /* Add the loopback if needed */
  if (current_routing->hierarchy == SURF_ROUTING_BASE) {
    for (i = 0; i < table_size; i++) {
      e_route = TO_ROUTE_FULL(i, i);
      if (!e_route) {
        e_route = xbt_new0(s_route_extended_t, 1);
        e_route->src_gateway = NULL;
        e_route->dst_gateway = NULL;
        e_route->generic_route.link_list =
            xbt_dynar_new(global_routing->size_of_link, NULL);
        xbt_dynar_push(e_route->generic_route.link_list,
                       &global_routing->loopback);
        TO_ROUTE_FULL(i, i) = e_route;
      }
    }
  }
}

void model_full_set_route(routing_component_t rc, const char *src,
		const char *dst, name_route_extended_t route)
{
	int *src_id, *dst_id;
	src_id = xbt_dict_get_or_null(rc->to_index, src);
	dst_id = xbt_dict_get_or_null(rc->to_index, dst);
	routing_component_full_t routing = ((routing_component_full_t) rc);
	size_t table_size = xbt_dict_length(routing->generic_routing.to_index);

	xbt_assert(src_id
			  && dst_id, "Network elements %s or %s not found", src, dst);

	xbt_assert(xbt_dynar_length(route->generic_route.link_list) > 0,
			  "Invalid count of links, must be greater than zero (%s,%s)",
			  src, dst);

	if(!routing->routing_table)
		routing->routing_table = xbt_new0(route_extended_t, table_size * table_size);

	if(TO_ROUTE_FULL(*src_id, *dst_id))
	{
		char * link_name;
		unsigned int i;
		xbt_dynar_t link_route_to_test = xbt_dynar_new(global_routing->size_of_link, NULL);
		xbt_dynar_foreach(route->generic_route.link_list,i,link_name)
		{
			void *link = xbt_lib_get_or_null(link_lib, link_name, SURF_LINK_LEVEL);
			xbt_assert(link,"Link : '%s' doesn't exists.",link_name);
			xbt_dynar_push(link_route_to_test,&link);
		}
		xbt_assert(!xbt_dynar_compare(
			  (void*)TO_ROUTE_FULL(*src_id, *dst_id)->generic_route.link_list,
			  (void*)link_route_to_test,
			  (int_f_cpvoid_cpvoid_t) surf_pointer_resource_cmp),
			  "The route between \"%s\" and \"%s\" already exists. If you are trying to define a reverse route, you must set the symmetrical=no attribute to your routes tags.", src,dst);
	}
	else
	{
		  if(!route->dst_gateway && !route->src_gateway)
			  XBT_DEBUG("Load Route from \"%s\" to \"%s\"", src, dst);
		  else{
			  XBT_DEBUG("Load ASroute from \"%s(%s)\" to \"%s(%s)\"", src,
			         route->src_gateway, dst, route->dst_gateway);
			  if(global_routing->get_network_element_type((const char*)route->dst_gateway) == SURF_NETWORK_ELEMENT_NULL)
				  xbt_die("The dst_gateway '%s' does not exist!",route->dst_gateway);
			  if(global_routing->get_network_element_type((const char*)route->src_gateway) == SURF_NETWORK_ELEMENT_NULL)
				  xbt_die("The src_gateway '%s' does not exist!",route->src_gateway);
		  }
	      TO_ROUTE_FULL(*src_id, *dst_id) = generic_new_extended_route(rc->hierarchy,route,1);
	      xbt_dynar_shrink(TO_ROUTE_FULL(*src_id, *dst_id)->generic_route.link_list, 0);
	}

	if( A_surfxml_route_symmetrical == A_surfxml_route_symmetrical_YES
		|| A_surfxml_ASroute_symmetrical == A_surfxml_ASroute_symmetrical_YES )
	{
		if(route->dst_gateway && route->src_gateway)
		{
                  char *gw_tmp ;
                  gw_tmp = route->src_gateway;
                  route->src_gateway = route->dst_gateway;
                  route->dst_gateway = gw_tmp;
		}
		if(TO_ROUTE_FULL(*dst_id, *src_id))
		{
			char * link_name;
			unsigned int i;
			xbt_dynar_t link_route_to_test = xbt_dynar_new(global_routing->size_of_link, NULL);
			for(i=xbt_dynar_length(route->generic_route.link_list) ;i>0 ;i--)
			{
				link_name = xbt_dynar_get_as(route->generic_route.link_list,i-1,void *);
				void *link = xbt_lib_get_or_null(link_lib, link_name, SURF_LINK_LEVEL);
				xbt_assert(link,"Link : '%s' doesn't exists.",link_name);
				xbt_dynar_push(link_route_to_test,&link);
			}
			xbt_assert(!xbt_dynar_compare(
				  (void*)TO_ROUTE_FULL(*dst_id, *src_id)->generic_route.link_list,
			      (void*)link_route_to_test,
				  (int_f_cpvoid_cpvoid_t) surf_pointer_resource_cmp),
				  "The route between \"%s\" and \"%s\" already exists", src,dst);
		}
		else
		{
			  if(!route->dst_gateway && !route->src_gateway)
				  XBT_DEBUG("Load Route from \"%s\" to \"%s\"", dst, src);
			  else
				  XBT_DEBUG("Load ASroute from \"%s(%s)\" to \"%s(%s)\"", dst,
				         route->src_gateway, src, route->dst_gateway);
		      TO_ROUTE_FULL(*dst_id, *src_id) = generic_new_extended_route(rc->hierarchy,route,0);
		      xbt_dynar_shrink(TO_ROUTE_FULL(*dst_id, *src_id)->generic_route.link_list, 0);
		}
	}

	if (rc->hierarchy == SURF_ROUTING_BASE) generic_free_route((route_t)route) ;
	else generic_free_extended_route((route_extended_t)route);
}