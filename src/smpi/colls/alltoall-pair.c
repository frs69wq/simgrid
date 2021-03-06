/* Copyright (c) 2013-2014. The SimGrid Team.
 * All rights reserved.                                                     */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#include "colls_private.h"

/*****************************************************************************

 * Function: alltoall_pair

 * Return: int

 * Inputs:
    send_buff: send input buffer
    send_count: number of elements to send
    send_type: data type of elements being sent
    recv_buff: receive output buffer
    recv_count: number of elements to received
    recv_type: data type of elements being received
    comm: communicator

 * Descrp: Function works when P is power of two. In each phase of P - 1
           phases, nodes in pair communicate their data.

 * Auther: Ahmad Faraj

 ****************************************************************************/

int smpi_coll_tuned_alltoall_pair_rma(void *send_buff, int send_count, MPI_Datatype send_type,
                  void *recv_buff, int recv_count, MPI_Datatype recv_type,
                  MPI_Comm comm)
{

  MPI_Aint send_chunk, recv_chunk;
  MPI_Win win;
  int assert = 0;
  int i, dst, rank, num_procs;

  char *send_ptr = (char *) send_buff;

  rank = smpi_comm_rank(comm);
  num_procs = smpi_comm_size(comm);
  send_chunk = smpi_datatype_get_extent(send_type);
  recv_chunk = smpi_datatype_get_extent(recv_type);

  win=smpi_mpi_win_create(recv_buff, num_procs * recv_chunk * send_count, recv_chunk, 0,
                 comm);
  send_chunk *= send_count;
  recv_chunk *= recv_count;

  smpi_mpi_win_fence(assert, win);
  for (i = 0; i < num_procs; i++) {
    dst = rank ^ i;
    smpi_mpi_put(send_ptr + dst * send_chunk, send_count, send_type, dst,
            rank /* send_chunk*/, send_count, send_type, win);
  }
  smpi_mpi_win_fence(assert, win);
  smpi_mpi_win_free(&win);
  return 0;
}


int smpi_coll_tuned_alltoall_pair(void *send_buff, int send_count,
                                  MPI_Datatype send_type,
                                  void *recv_buff, int recv_count,
                                  MPI_Datatype recv_type, MPI_Comm comm)
{

  MPI_Aint send_chunk, recv_chunk;
  MPI_Status s;
  int i, src, dst, rank, num_procs;
  int tag = COLL_TAG_ALLTOALL;
  char *send_ptr = (char *) send_buff;
  char *recv_ptr = (char *) recv_buff;

  rank = smpi_comm_rank(comm);
  num_procs = smpi_comm_size(comm);

  if((num_procs&(num_procs-1)))
    THROWF(arg_error,0, "alltoall pair algorithm can't be used with non power of two number of processes ! ");

  send_chunk = smpi_datatype_get_extent(send_type);
  recv_chunk = smpi_datatype_get_extent(recv_type);

  send_chunk *= send_count;
  recv_chunk *= recv_count;

  for (i = 0; i < num_procs; i++) {
    src = dst = rank ^ i;
    smpi_mpi_sendrecv(send_ptr + dst * send_chunk, send_count, send_type, dst, tag,
		 recv_ptr + src * recv_chunk, recv_count, recv_type, src, tag,
		 comm, &s);
  }

  return MPI_SUCCESS;
}
