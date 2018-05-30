/*******************************************************************************
 * 							Common program
 * *****************************************************************************
 * 	Filename:		program.h
 * 	Platform:		ubuntu 16.04 64 bit
 * 	Author:			Copyright (C) Selmeczi János, original version
 *******************************************************************************
 *							Licensing
 *******************************************************************************
 *  This file is part of sdrflow.
 *
 *  Sdrflow is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Sdrflow is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 *							Revision history
 *******************************************************************************
 *	Author				Date		Comment
 *******************************************************************************
 *	Selmeczi János		23-04-2018	Original version
 *
 ******************************************************************************/

#ifndef	PROGRAM_H
#define	PROGRAM_H

/********************************* Constants **********************************/

#define		PROGRAM_STATE_CREATED			0
#define		PROGRAM_STATE_READY_TO_INIT		1
#define		PROGRAM_STATE_INITIALIZING		2
#define		PROGRAM_STATE_INITIALIZED		3
#define		PROGRAM_STATE_READY_TO_FIRE		4
#define		PROGRAM_STATE_FIREING			5
#define		PROGRAM_STATE_END_CYCLE			6
#define		PROGRAM_STATE_PAUSE				7
#define		PROGRAM_STATE_BREAK				8
#define		PROGRAM_STATE_READY_TO_CLEAN	9
#define		PROGRAM_STATE_CLEANING			10
#define		PROGRAM_STATE_CLEANED			11
#define		PROGRAM_STATE_ERROR				12


#define		PROGRAM_THREAD_STATE_STOP	0
#define		PROGRAM_THREAD_STATE_RUN	1

/***************************** Function prototypes ****************************/

int		program_init(void);
void 	*program_new(char *comp_name, char *name);
void 	*program_find(char *name);
int		program_get_state(void *prog);
void	program_set_cancel_req(void *prog, int cancel);
void	program_set_cancel_ack(void *prog, int akn);
int		program_get_cancel_ack(void *prog);
void 	program_set_entry(void *prog, int entry, int cycle_limit);
int 	program_get_entry(void *prog);
int32_t *program_get_ip(void *prog);
int		program_get_parameter(char **path, int path_count, void *value, int value_length);
int		program_set_parameter(char **path, int path_count, void *value, int value_length);
void 	*program_run(void *prog);
void 	program_run_entry(void *prog, int entry, int cycle_limit);
void	program_step(void *prog);
void	program_step_into(void *prog);
int 	program_delete(void *prog);
int 	program_cleanup(void);
int		program_fire_in_thread(char *name);
int		program_fire(char *name, int cycle_limit);
int		program_kill_fireing(char *name);
int 	program_benchmark(char *name, int cycle_limit, double *time_ms);

#endif
