/*******************************************************************************
 * 							Common program
 * *****************************************************************************
 * 	Filename:		program.c
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

#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<pthread.h>
#include	<time.h>
#include	"objects.h"
#include	"program.h"
#include	"instructions.h"
#include	"composite.h"
#include	"obj_format.h"
#include	"../../include/primitive_interface.h"

/****************************** Local constants *******************************/

#define		THIS_PLATFORM		0
#define		CALL_FRAME_SIZE		3
#define		PROGRAM_ALLOC_SIZE	16
#define		PROGRAM_STACK_SIZE	2048

/******************************** Local types *********************************/

struct _program
{
	void			*top_composite;
	void			*top_data_segment;
	int				state;
	int				fireing_in_thread;
	pthread_t		fireing_thread;
	int				cancel_req;
	int				cancel_ack;
	int				entry_to_run;
	int				cycle_count;
	int				cycle_limit;
	void			*data;
	void			*context;
	int32_t			*ip;
	void			**sp;
	int32_t			*breakpoint;
	int32_t			*ip_save;
	int32_t			instr_save;
	int				error;
	char			top_composite_name[MAX_NAME_LENGTH + 1];
	char			name[MAX_NAME_LENGTH + 1];
	void			*stack[PROGRAM_STACK_SIZE + 1];
};

typedef struct _program	program_t;

// emulate top level context

float	i1[30];
float	o1[30];
int		p1[2];
int		p2[5];

struct _M1ctx
{
	float	*i1;
	float	*o1;
	int		*p1;
	int		*p2;
}__attribute__((packed)) M1ctx;



/*************************** Local variables **********************************/


static int instruction_size[INSTR_COUNT] =
{
	[EXIT_INSTR] = sizeof(exit_instr_t),
	[PAUSE_INSTR] = sizeof(pause_instr_t),
	[END_INSTR] = sizeof(end_instr_t),
	[ENDCYCLE_INSTR] = sizeof(endcycle_instr_t),
	[BREAK_INSTR] = sizeof(break_instr_t),
	[GOTO_INSTR] = sizeof(goto_instr_t),
	[DO_INSTR] = sizeof(do_instr_t),
	[LOOP_INSTR] = sizeof(loop_instr_t),
	[FIRE_INSTR] = sizeof(call_instr_t),
	[INIT_INSTR] = sizeof(call_instr_t),
	[CLEANUP_INSTR] = sizeof(call_instr_t),
	[RET_INSTR] = sizeof(ret_instr_t),
	[PRIM_FIRE_INSTR] = sizeof(call_prim_instr_t),
	[PRIM_INIT_INSTR] = sizeof(call_prim_instr_t),
	[PRIM_CLEANUP_INSTR] = sizeof(call_prim_instr_t),
	[CPPTR_INSTR] = sizeof(cp_var_instr_t),
	[INCPTR_INSTR] = sizeof(add_lint_instr_t),
	[CPCTXPTR_INSTR] = sizeof(cp_var_instr_t)
};

static int	program_initialized = 0;
static void	*programs = NULL;

/******************* Externally accessible functions **************************/

int	program_init(void)
{
	if (program_initialized != 0) return -1;
	if((programs = objects_new_set(OBJECTS_TYPE_PROGRAM)) == NULL)
			return -1;
	program_initialized = -1;
	return 0;
}

void *program_new(char *comp_name, char *name)
{
	program_t	*p;
	
	if (program_initialized == 0) program_init();
	if (strlen(name) > MAX_NAME_LENGTH)
	{
		fprintf(stderr, "program_new(): program name [%s] is too long\r\n", name);
		return NULL;
	}
	if (strlen(comp_name) > MAX_NAME_LENGTH)
	{
		fprintf(stderr, "program_new(): top composite name [%s] is too long\r\n", comp_name);
		return NULL;
	}
	if ((p = (program_t *) objects_new(programs, sizeof(program_t))) == NULL)
	{
		fprintf(stderr, "program_new(): could not create program object\r\n");
		return NULL;
	}

	strcpy(&p->top_composite_name[0], comp_name);
	strcpy(&p->name[0],name);
	
	if ((p->top_composite = composite_load(comp_name)) == NULL)
	{ 
		fprintf(stderr, "program_new(%s, %s): could not load top level composite\r\n", comp_name, name);
		return NULL;
	}
	
	if (composite_init_instance(p->top_composite, name, &p->top_data_segment) != 0)
	{
		fprintf(stderr, "program_new(%s, %s): could not make top composit instance\r\n", comp_name, name);
		if (composite_delete(comp_name) != 0)
			fprintf(stderr, "program_new(%s, %s): error in deleting top level composite\r\n", comp_name, name);
		return NULL;
	}
	
	p->cancel_req = 0;
	p->cancel_ack = -1;
	p->ip_save = NULL;
	p->instr_save = EXIT_INSTR;
	p->state = PROGRAM_STATE_CREATED;
	p->fireing_in_thread = 0;
	p->breakpoint = NULL;
	p->cycle_count = 0;
	p->cycle_limit = -1;
	p->data = p->top_data_segment;
	p->context = NULL;
	p->error = 0;
	p->context = NULL; //(void *) &M1ctx;
//	M1ctx.i1 = &i1[0];
//	M1ctx.o1 = &o1[0];
//	M1ctx.p1 = &p1[0];
//	M1ctx.p2 = &p2[0];

	program_run_entry((void *) p, ENTRY_INIT, 1);
	return (void *)p;
}

void	*program_find(char *name)
{
	program_t	*prog;

	if (name == NULL) return NULL;
	prog = (program_t *) objects_get_first(programs);
	while (prog != NULL)
	{
		if (strcmp(name, prog->name) == 0)
			break;
		prog = (program_t *) objects_get_next(programs, prog);
	}
	return (void *) prog;
}

int		program_get_state(void *prog)
{
	program_t	*program = (program_t *) prog;

	return program->state;
}

void	program_set_cancel_req(void *prog, int cancel_req)
{
	((program_t *)prog)->cancel_req = cancel_req;
}

int	program_get_cancel_req(void *prog)
{
	return ((program_t *)prog)->cancel_req;
}

void	program_set_cancel_ack(void *prog, int ack)
{
	((program_t *)prog)->cancel_ack = ack;
}

int	program_get_cancel_ack(void *prog)
{
	return ((program_t *)prog)->cancel_ack;
}

void program_set_entry(void *prog, int entry, int cycle_limit)
{
	program_t	*p = ((program_t *)prog);
	
//	fprintf(stderr,"program_set_entry(entry=%d, limit=%d): started\r\n", entry, cycle_limit);
	p->stack[PROGRAM_STACK_SIZE] = NULL;
	p->ip = composite_get_entry(p->top_composite, entry);
	p->data = p->top_data_segment;
	p->sp = &p->stack[PROGRAM_STACK_SIZE];
	*(--p->sp) = p->data;
	*(--p->sp) = p->context;
	*(--p->sp) = (void *)(p->ip - 1);
	switch(entry)
	{
		case	ENTRY_INIT:
			p->state = PROGRAM_STATE_READY_TO_INIT;
			break;
		case	ENTRY_FIRE:
			p->state = PROGRAM_STATE_READY_TO_FIRE;
			break;
		case	ENTRY_CLEANUP:
			p->state = PROGRAM_STATE_READY_TO_CLEAN;
			break;
		default:
			p->state = PROGRAM_STATE_CREATED;
	}
	p->entry_to_run = entry;
	p->fireing_in_thread = 0;
	p->cycle_count = 0;
	p->cycle_limit = cycle_limit;
	p->breakpoint = NULL;
	p->cancel_req = 0;
}

int program_get_entry(void *prog)
{
	return ((program_t *)prog)->entry_to_run;
}

int	program_get_parameter(char **path, int path_count, void *value, int value_length)
{
	program_t	*program = NULL;
	void		*buffer = NULL;
	int			buffer_length = 0;
	int			error = 0;

	fprintf(stderr,"program_get_parameter\r\n");
	if ((program = (program_t *) program_find(path[0])) != NULL)
	{
		if (composite_find_buffer(path, 1, path_count - 1, program->top_data_segment, &buffer, &buffer_length) == 0)
		{
			if (value_length >= buffer_length)			
				memcpy(value, buffer, buffer_length);
			else
				error = -1;
		}
		else
		{
			error = -1;
		}
	}
	else
	{
		error = -1;
	}
	return error;
}

int	program_set_parameter(char **path, int path_count, void *value, int value_length)
{
	program_t	*program = NULL;
	void		*buffer = NULL;
	int			buffer_length = 0;
	int			error = 0;

//	fprintf(stderr,"program_set_parameter\r\n");
	if ((program = (program_t *) program_find(path[0])) != NULL)
	{
		if (composite_find_buffer(path, 1, path_count - 1, program->top_data_segment, &buffer, &buffer_length) == 0)
		{
			if (value_length <= buffer_length)			
				memcpy(buffer, value, value_length);
			else
				error = -1;
		}
		else
		{
			error = -1;
		}
	}
	else
	{
		error = -1;
	}
	return error;
}

int32_t *program_get_ip(void *prog)
{
	return ((program_t *)prog)->ip;
}

int program_fire_in_thread(char *name)
{
	program_t	*p;
	int			res;
	int			error = 0;

	p = (program_t *) program_find(name);
	if (p != 0)
	{
		if ((p->state == PROGRAM_STATE_INITIALIZED) || (p->state == PROGRAM_STATE_END_CYCLE))
		{
			program_set_entry(p, ENTRY_FIRE, -1);
			res = pthread_create(&p->fireing_thread, NULL, program_run, p);
			if (res != 0)
			{
				fprintf(stderr,"Error: Thread creation failed\r\n");
				error = -1;
			}
			else
			{
				p->fireing_in_thread = -1;
			}
		}
		else
		{
			fprintf(stderr,"Error: Program in wrong state for fireing\r\n");
			error = -1;
		}
	}
	else
	{
		fprintf(stderr,"Cannot find program\r\n");
		error = -1;
	}
	return error;
}

int	program_kill_fireing(char *name)
{
	program_t	*p;
	int			res;
	int			error = 0;
	void		*result;

	p = (program_t *) program_find(name);
	if (p != 0)
	{
		p->cancel_req = -1;
		res = pthread_join(p->fireing_thread, &result);
		if (res != 0)
		{
			fprintf(stderr, "Thread join failed\r\n");
			p->state = PROGRAM_STATE_ERROR;
		}
		p->fireing_in_thread = 0;
		p->cancel_req = 0;
	}
	else
	{
		fprintf(stderr,"Cannot find program\r\n");
		error = -1;
	}
	return error;
}

int program_fire(char *name, int cycle_limit)
{
	program_t	*p;
	void		*ret __attribute__((unused));
	int			error = 0;

	p = (program_t *) program_find(name);
	if (p != 0)
	{
		if ((p->state == PROGRAM_STATE_INITIALIZED) || (p->state == PROGRAM_STATE_END_CYCLE))
		{
			if (cycle_limit >= 0)
			{
				program_set_entry(p, ENTRY_FIRE, cycle_limit);
				ret = program_run(p);
			}
			else
			{
				fprintf(stderr,"Error: Negative cycle limit\r\n");
				error = -1;
			}
		}
		else
		{
			fprintf(stderr,"Error: Program in wrong state for fireing\r\n");
			error = -1;
		}
	}
	else
	{
		fprintf(stderr,"Cannot find program\r\n");
		error = -1;
	}
	return error;
}

int program_benchmark(char *name, int cycle_limit, double *time_ms)
{
	program_t	*p;
	void		*ret __attribute__((unused));
	int			error = 0;
	struct timespec	start, end;

	p = (program_t *) program_find(name);
	if (p != 0)
	{
		if ((p->state == PROGRAM_STATE_INITIALIZED) || (p->state == PROGRAM_STATE_END_CYCLE))
		{
			if (cycle_limit >= 0)
			{
				program_set_entry(p, ENTRY_FIRE, cycle_limit);
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
				ret = program_run(p);
				clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
				*time_ms = 1000.0 * (end.tv_sec - start.tv_sec) +
							(end.tv_nsec - start.tv_nsec) / 1000000.0;
			}
			else
			{
				fprintf(stderr,"Error: Negative cycle limit\r\n");
				error = -1;
			}
		}
		else
		{
			fprintf(stderr,"Error: Program in wrong state for fireing\r\n");
			error = -1;
		}
	}
	else
	{
		fprintf(stderr,"Cannot find program\r\n");
		error = -1;
	}
	return error;
}


void program_run_entry(void *prog, int entry, int cycle_limit)
{
	void	*ret __attribute__((unused));

	program_set_entry(prog, entry, cycle_limit);
	ret = program_run(prog);
}

void	program_step(void *prog)
{
	program_t	*p = (program_t *) prog;
	void		*ret __attribute__((unused));
	
	if (*p->ip == RET_INSTR)
	{
		p->ip_save = (int32_t *) *p->sp;
	}
	else
	{
		p->ip_save = (int32_t *)(((void *)(p->ip)) + instruction_size[*p->ip]);
	}
	p->instr_save = *p->ip_save;
	*p->ip_save = BREAK_INSTR;
	ret = program_run(prog);
}

void	program_step_into(void *prog)
{
	program_t		*p = (program_t *) prog;
	comp_instance_t	*comp_instance;
	void		*ret __attribute__((unused));
	
	switch (*p->ip)
	{
		case	FIRE_INSTR:
			comp_instance = (comp_instance_t*)(p->data + *p->ip);
			p->ip_save = ((data_entry_table_t *) (comp_instance->data))->fire;
			break;
		case	INIT_INSTR:
			comp_instance = (comp_instance_t*)(p->data + *p->ip);
			p->ip_save = ((data_entry_table_t *) (comp_instance->data))->init;
			break;
		case	CLEANUP_INSTR:
			comp_instance = (comp_instance_t*)(p->data + *p->ip);
			p->ip_save = ((data_entry_table_t *) (comp_instance->data))->cleanup;
			break;
		case	RET_INSTR:
			p->ip_save = (int32_t *) *p->sp;
			break;
		default:
			p->ip_save = p->ip + instruction_size[*p->ip];
			break;
	}
	p->instr_save = *p->ip_save;
	*p->ip_save = BREAK_INSTR;
	ret = program_run(prog);
}

int program_delete(void *prog)
{
	int			ret = 0;
	program_t	*p = (program_t *) prog;
	
	if (objects_verify_object(programs, prog) != 0)
	{
		fprintf(stderr, "program_delete(): invalid program object\r\n");
		return -1;
	}
	program_run_entry((void *) p, ENTRY_CLEANUP, 1);
	if (composite_cleanup_instance(p->top_data_segment) != 0)
		fprintf(stderr, "program_delete(%s): error in cleaning data segments\r\n", p->name);
	if (composite_delete(p->top_composite_name) != 0)
		fprintf(stderr, "program_delete(%s): error in deleting top composite\r\n", p->name);
	objects_delete(programs, prog);
	return ret;
}

int program_cleanup(void)
{
	program_t	*p, *n;
	int			ret = 0;

//	fprintf(stderr,"program_cleanup()\r\n");
	p = (program_t *)objects_get_first(programs);
	while (p != NULL)
	{
		n = (program_t *) objects_get_next(programs, p);
		program_delete(p);
		p = n;
	}
	if (objects_delete_set(programs) != 0) ret = -1;
	programs = NULL;
	program_initialized = 0;
	return ret;
}

void *program_run(void *program)
{
	int			*ip = ((program_t *)program)->ip;
	void		**sp = ((program_t *)program)->sp;
	void		*data = ((program_t *)program)->data;
	void		*context = ((program_t *)program)->context;
//	void		**itab;
	void		**p2p;
	program_t	*p = (program_t *) program;
	
//	This table should be declared here, becouse the labels used for
//	the initialization could not be used outside of this function.
//	The table is initialized only once.
	
	static void	*instruction_table[18] =
	{
		[EXIT_INSTR] = &&instr_exit,
		[PAUSE_INSTR] = &&instr_pause,
		[END_INSTR] = &&instr_exit,
		[ENDCYCLE_INSTR] = &&instr_end_cycle,
		[BREAK_INSTR] = &&instr_exit,
		[GOTO_INSTR] = &&instr_goto,
		[DO_INSTR] = &&instr_do,
		[LOOP_INSTR] = &&instr_loop,
		[FIRE_INSTR] = &&instr_fire,
		[INIT_INSTR] = &&instr_init,
		[CLEANUP_INSTR] = &&instr_cleanup,
		[RET_INSTR] = &&instr_ret,
		[PRIM_FIRE_INSTR] = &&instr_prim_fire,
		[PRIM_INIT_INSTR] = &&instr_prim_init,
		[PRIM_CLEANUP_INSTR] = &&instr_prim_cleanup,
		[CPPTR_INSTR] = &&instr_cpptr,
		[INCPTR_INSTR] = &&instr_inc_ptr,
		[CPCTXPTR_INSTR] = &&instr_cpctxptr
	};

//	itab = &instruction_table[0];
	switch(p->state)
	{
		case PROGRAM_STATE_READY_TO_INIT:
			p->state = PROGRAM_STATE_INITIALIZING;
			break;
		case PROGRAM_STATE_READY_TO_FIRE:
			p->state = PROGRAM_STATE_FIREING;
			break;
		case PROGRAM_STATE_READY_TO_CLEAN:
			p->state = PROGRAM_STATE_CLEANING;
			break;
		default:
			p->state = PROGRAM_STATE_ERROR;
			return NULL;
			break;
	}

//	fprintf(stderr,"ip=%ld instruction=%d\r\n",(long)ip, *ip);
//	fprintf(stderr,"run start sp=%p\r\n", sp);
	goto *instruction_table[*ip++];
	return NULL;

instr_exit:
	switch(p->state)
	{
		case PROGRAM_STATE_INITIALIZING:
			p->state = PROGRAM_STATE_INITIALIZED;
			break;
		case PROGRAM_STATE_CLEANING:
			p->state = PROGRAM_STATE_CLEANED;
			break;
		default:
			p->state = PROGRAM_STATE_ERROR;
			break;
	}
	return NULL;

instr_pause:
	p->ip = ip;
	p->sp = sp;
	p->data = data;
	p->context = context;
	p->state = PROGRAM_STATE_PAUSE;
	return NULL;

instr_end_cycle:
//	fprintf(stderr,"cycle end\r\n");
	if (((++p->cycle_count >= p->cycle_limit) && (p->cycle_limit >= 0)) || (p->cancel_req != 0))
	{
		sp -= CALL_FRAME_SIZE;
		p->ip = ip;
		p->sp = sp;
		p->data = data;
		p->context = context;
		p->state = PROGRAM_STATE_END_CYCLE;
		return NULL;
	}
	else
	{
		sp -= CALL_FRAME_SIZE;
	}
	goto *instruction_table[*ip++];

instr_goto:
	ip += *ip;
	goto *instruction_table[*ip++];

instr_do:
//	fprintf(stderr,"do %d\r\n",*ip);
	*(--sp) = (void *)((long) *ip++);
	goto *instruction_table[*ip++];

instr_loop:
//	fprintf(stderr,"loop %ld\r\n",((long)*sp)-1);
	if ((long)--*sp <= 0)
	{
		sp++;
		ip++;
	}
	else
	{
		ip = (int *)(((void *)ip) + *ip);
	}
	goto *instruction_table[*ip++];

instr_fire:
//	fprintf(stderr,"fire composite\r\n");
	p2p = (void **)(data + *ip++);
	*(--sp) = data;
	*(--sp) = context;
	*(--sp) = (void *) ip;
	ip = *(*((int ***) p2p++));
	data = *p2p++;
	context = (void *) p2p;
	goto *instruction_table[*ip++];

instr_init:
	p2p = (void **)(data + *ip++);
	*(--sp) = data;
	*(--sp) = context;
	*(--sp) = (void *) ip;
	ip = *(*((int ***) p2p++) + 1);
	data = *p2p++;
	context = (void *) p2p;
	goto *instruction_table[*ip++];

instr_cleanup:
	p2p = (void **)(data + *ip++);
	*(--sp) = data;
	*(--sp) = context;
	*(--sp) = (void *) ip;
	ip = *(*((int ***) p2p++) + 2);
	data = *p2p++;
	context = (void *) p2p;
	goto *instruction_table[*ip++];

instr_ret:
//	fprintf(stderr,"return sp=%p\r\n",sp);
	ip = (int *)*sp++;
	context = *sp++;
	data = *sp++;
//	fprintf(stderr,"return ip=%p\r\n", ip);
	goto *instruction_table[*ip++];

instr_prim_fire:
//	fprintf(stderr,"fire primitive\r\n");
	p2p = (void **) (data + *ip++);
	p->error = (***((primitive_entry_t **) p2p))((void *)(p2p + 1));
	goto *instruction_table[*ip++];

instr_prim_init:
//	fprintf(stderr,"init primitive\r\n");
	p2p = (void **) (data + *ip++);
	p->error = (**(*((primitive_entry_t **) p2p) + 1))((void *)(p2p + 1));
//	fprintf(stderr,"init primitive returned\r\n");
	goto *instruction_table[*ip++];

instr_prim_cleanup:
	p2p = (void **) (data + *ip++);
	p->error = (**(*((primitive_entry_t **) p2p) + 2))((void *)(p2p + 1));
	goto *instruction_table[*ip++];

instr_cpptr:
//	fprintf(stderr,"copy pointer\r\n");
	p2p = (void **)(data + *ip++);
	*p2p = *((void **) (data + *ip++));
	goto *instruction_table[*ip++];

instr_cpctxptr:
//	fprintf(stderr,"copy context pointer\r\n");
	p2p = (void **)(data + *ip++);
//	fprintf(stderr,"data=%ld,p2p=%ld,offset=%ld\r\n",(long) data, (long) p2p, (long)p2p - (long)data);
//	fprintf(stderr,"M1ctx=%ld,context=%ld,i1=%ld\r\n",(long)&M1ctx,(long)context,(long)(context +*ip));
	*p2p = *((void **) (context + *ip++));
//	fprintf(stderr,"poiter copied\r\n");
	goto *instruction_table[*ip++];

instr_inc_ptr:
//	fprintf(stderr,"increment ptr\r\n");
	p2p = (void **)(data + *ip++);
	*p2p += *ip++;
	goto *instruction_table[*ip++];
	
	return NULL;
}
