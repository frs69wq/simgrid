/* Copyright (c) 2012. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include <msg/msg.h>

int master(int argc, char *argv[]);

//dump function to create and execute a task
static void create_and_execute_task (void)
{
  m_task_t task = MSG_task_create("task", 1000000, 0, NULL);
  MSG_task_execute (task);
  MSG_task_destroy (task);
}

int master(int argc, char *argv[])
{
  int i;

  //Set initial values for the link user variables
  //
  //This example uses source and destination where
  //source and destination are the name of hosts in
  //the platform file.
  //
  //The functions will set/change the value of the variable
  //for all links in the route between source and destination.

  //Set the Link_Capacity variable
  TRACE_link_srcdst_variable_set("Tremblay", "Bourassa", "Link_Capacity", 12.34);
  TRACE_link_srcdst_variable_set("Fafard", "Ginette", "Link_Capacity", 56.78);

  //Set the Link_Utilization variable
  TRACE_link_srcdst_variable_set("Tremblay", "Bourassa", "Link_Utilization", 1.2);
  TRACE_link_srcdst_variable_set("Fafard", "Ginette", "Link_Utilization", 3.4);

  //run the simulation, update my variables accordingly
  for (i = 0; i < 10; i++) {
    create_and_execute_task ();

    //Add to link user variables
    TRACE_link_srcdst_variable_add ("Tremblay", "Bourassa", "Link_Utilization", 5.6);
    TRACE_link_srcdst_variable_add ("Fafard", "Ginette", "Link_Utilization", 7.8);
  }

  for (i = 0; i < 10; i++) {
    create_and_execute_task ();

    //Subtract from link user variables
    TRACE_link_srcdst_variable_sub ("Tremblay", "Bourassa", "Link_Utilization", 3.4);
    TRACE_link_srcdst_variable_sub ("Fafard", "Ginette", "Link_Utilization", 5.6);
  }

  return 0;
}

/** Main function */
int main(int argc, char *argv[])
{
  MSG_global_init(&argc, argv);
  if (argc < 3) {
    printf("Usage: %s platform_file deployment_file\n", argv[0]);
    exit(1);
  }

  char *platform_file = argv[1];
  char *deployment_file = argv[2];
  MSG_create_environment(platform_file);

  //declaring link user variables (one without, another with a RGB color)
  TRACE_link_variable_declare("Link_Capacity");
  TRACE_link_variable_declare_with_color ("Link_Utilization", "0.9 0.1 0.1");

  //register "master" and "slave" functions and launch deployment
  MSG_function_register("master", master);
  MSG_function_register("slave", master);
  MSG_launch_application(deployment_file);

  MSG_main();
  MSG_clean();
  return 0;
}