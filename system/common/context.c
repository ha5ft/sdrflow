/*******************************************************************************
 * 							Common context
 * *****************************************************************************
 * 	Filename:		context.c
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

#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#define	_CONTEXT_

#include	"objects.h"
#include	"pointer.h"
#include	"words.h"
#include	"hash_table.h"
#include	"scanner.h"
#include	"io.h"
#include	"data.h"
#include	"context.h"
#include	"src_lang.h"
#include	"location.h"

//#define		CONTEXT_ALLOC_SIZE	32

//typedef struct	_link
//{
//	struct _link	*prev;
//	struct _link	*next;
//}link_t;
//
//typedef struct _object_set
//{
//	int		type_index;
//	link_t	*first;
//	link_t	*last;
//} object_set_t;

static unsigned int	context_type_index = (unsigned int) OBJECTS_TYPE_CONTEXT;
static int	context_initialized = 0;
static char	*context_error_description[] =
{
	[0] = "No error",	
	[1] = "Scanner error",	
	[2] = "Only composite or primitive definition or EOL is allowed here",	
	[3] = "Identifier is expected",	
	[4] = "EOL is expected",	
	[5] = "Can not create new input data object",	
	[6] = "Can not parse input definition",	
	[7] = "Can not create new output data object",	
	[8] = "Can not parse output definition",	
	[9] = "Only input or output definition or EOL is allowed here",	
	[10] = "Can not create new parameter data object",	
	[11] = "Can not parse parameter definition",	
	[12] = "Can not create new state data object",	
	[13] = "Can not parse state definition",	
	[14] = "Can not create new temp data object",	
	[15] = "Can not parse temp definition",	
	[16] = "Only input, output, parameter, state or temp definition or EOL is allowed here",	
	[17] = "Only output, parameter, state or temp definition or EOL is allowed here",	
	[18] = "Only parameter, state or temp definition or EOL is allowed here",	
	[19] = "Only temp definition or EOL is allowed here",	
	[20] = "Only end or EOL is allowed here",	
	[21] = "Only EOL is allowed here",	
	[22] = "Identifier is already defined",
	[23] = "Can not get identifier's name",
	[24] = "Can not parse context details"
};
static void	*locations = NULL;

static int	_context_lcm(int a, int b);
static int	_context_set_compute_rep_denom_lcm(void *set, int sub);
static int	_context_set_clear_repetition(void *set, int sub);
static int	_context_sort_isolated_actors(context_t *act, void *ptrset);

/*******************************************************************************
 * 						Public functions
 * ****************************************************************************/

// ************************** Object management functions **********************

int		context_initialize(void)
{
	if(context_initialized != 0) return -1;
	if (objects_is_initialized() == 0)
	{
		if (objects_init() != 0) return -1;
	}
//	if ((context_type_index = objects_register_type(sizeof(context_t), CONTEXT_ALLOC_SIZE)) < 0) return -1;
	context_type_index = (unsigned int) OBJECTS_TYPE_CONTEXT;
	if ((locations = location_new_set()) == NULL) return -1;
	if (location_add(locations, IO_CHAN_TYPE_FILE, "./") == -1)
	{
		location_delete_set(locations);
		return -1;
	}
	context_initialized = -1;
	return 0;	
}

int context_set_name(void *ctx, char *name)
{
	context_t	*c = (context_t *) ctx;

	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_set_name(%s): Context class not initialized\n", name);
		return -1;
	}
	if (strlen(name) > WORD_NAME_LENGTH)
	{
		fprintf(stderr, "error: context_set_name(%s): Name too long\n", name);
		return -1;
	}
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_set_name(%s): Invalid context object\n", name);
		return -1;		
	}
	strcpy(c->name, name);
	return 0;
}

int	context_add_location(int type, char *path)
{
	return location_add(locations, type, path);
}


void	*context_new_set(void)
{
	if(context_initialized == 0) return NULL;
	return objects_new_set(context_type_index);			
}

void	*context_new(void *set)
{
	context_t	*ctx;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_new(): Not initialized\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: context_new(): Actor set is NULL\n");
		return NULL;
	}
	ctx = (context_t *) objects_new(set, sizeof(context_t));
	if (ctx != NULL)
	{
		ctx->name[0] = 0;
		ctx->inputs = NULL;
		ctx->outputs = NULL;
		ctx->params = NULL;
		ctx->state = NULL;
		ctx->temp = NULL;
		ctx->type = CONTEXT_TYPE_UNDEF;
		ctx->root = NULL;
		ctx->visited = 0;
		ctx->subgraph = 0;
		ctx->sorted = 0;
		ctx->fire_count = 0;
		ctx->max_fire_count = 0;
	}
	return ctx;
}

void	*context_make_instance(void *ctx, void *instance_set, char *name, void *hash)
{
	context_t	*c = (context_t *) ctx;
	context_t	*instance = NULL;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_make_instance(): Not initialized\n");
		return NULL;
	}
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_make_instance(): Actor is NULL\n");
		return NULL;
	}
	if ((c->type != CONTEXT_TYPE_PRIMITIVE) && (c->type != CONTEXT_TYPE_COMPOSITE))
	{
		fprintf(stderr, "error: context_make_instance(): Invalid actor type\n");
		fprintf(stderr, "\[actor: %s] [type: %d]\n", c->name, c->type);
		return NULL;
	}
	if (strlen(name) > WORD_NAME_LENGTH - 1)
	{
		fprintf(stderr, "error: context_make_instance(): Instance name too long\n");
		fprintf(stderr, "\[actor: %s] [instance name: %s]\n", c->name, name);
		return NULL;
	}
	if ((instance = (context_t *)context_new(instance_set)) == NULL)
	{
		fprintf(stderr, "error: context_make_instance(): Can not create new instance object\n");
		fprintf(stderr, "\[actor: %s]\n", c->name);
		return NULL;
	}
	strcpy(instance->name, name);
	if (c->inputs != NULL)
	{
		if ((instance->inputs = data_clone_set(c->inputs, instance, name, hash)) == NULL)
		{
			fprintf(stderr, "error: context_make_instance(): Can not clone input ports\n");
			fprintf(stderr, "\[actor: %s]\n", c->name);
			context_delete(instance_set, instance);
			return NULL;
		}
	}
	if (c->outputs != NULL)
	{
		if ((instance->outputs = data_clone_set(c->outputs, instance, name, hash)) == NULL)
		{
			fprintf(stderr, "error: context_make_instance(): Can not clone output ports\n");
			fprintf(stderr, "\[actor: %s]\n", c->name);
			context_delete(instance_set, instance);
			return NULL;
		}
	}
	if (c->params != NULL)
	{
		if ((instance->params = data_clone_set(c->params, instance, name, hash)) == NULL)
		{
			fprintf(stderr, "error: context_make_instance(): Can not clone parameter ports\n");
			fprintf(stderr, "\[actor: %s]\n", c->name);
			context_delete(instance_set, instance);
			return NULL;
		}
	}
	if (c->state != NULL)
	{
		if ((instance->state = data_clone_set(c->state, instance, name, hash)) == NULL)
		{
			fprintf(stderr, "error: context_make_instance(): Can not clone state ports\n");
			fprintf(stderr, "\[actor: %s]\n", c->name);
			context_delete(instance_set, instance);
			return NULL;
		}
	}
	if (c->temp != NULL)
	{
		if ((instance->temp = data_clone_set(c->temp, instance, name, hash)) == NULL)
		{
			fprintf(stderr, "error: context_make_instance(): Can not clone temporary ports\n");
			fprintf(stderr, "\[actor: %s]\n", c->name);
			context_delete(instance_set, instance);
			return NULL;
		}
	}
	if (c->type == CONTEXT_TYPE_PRIMITIVE)
		instance->type = CONTEXT_TYPE_PRIM_INSTANCE;
	else
		instance->type = CONTEXT_TYPE_COMP_INSTANCE;
	instance->root = c;
	return instance;
}

int		context_delete(void *set, void *ctx)
{
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_delete(): Not initialized\n");
		return -1;
	}
	if (objects_verify_object(set, ctx) != 0)
	{
		fprintf(stderr, "error: context_delete(): Can not verify object\n");
		return -1;
	}
	data_delete_set(((context_t *)ctx)->inputs);
	data_delete_set(((context_t *)ctx)->outputs);
	data_delete_set(((context_t *)ctx)->params);
	data_delete_set(((context_t *)ctx)->state);
	data_delete_set(((context_t *)ctx)->temp);		
	return objects_delete(set, ctx);	
}

int		context_delete_set(void *set)
{
	context_t	*ctx;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_delete_set(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: context_delete_set(): Actor set is NULL\n");
		return -1;
	}
	ctx = (context_t *) objects_get_first(set);
	while (ctx != NULL)
	{
		data_delete_set(((context_t *)ctx)->inputs);
		data_delete_set(((context_t *)ctx)->outputs);
		data_delete_set(((context_t *)ctx)->params);
		data_delete_set(((context_t *)ctx)->state);
		data_delete_set(((context_t *)ctx)->temp);		
		ctx = (context_t *) objects_get_next(set, (void *)ctx);
	}
	return objects_delete_set(set);	
}

int		context_cleanup(void)
{
	void		*set;
	context_t	*ctx;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_cleanup(): Not initialized\n");
		return -1;
	}
	set = objects_get_first_set(context_type_index);
	while (set != NULL)
	{
		ctx = (context_t *) objects_get_first(set);
		while (ctx != NULL)
		{
			data_delete_set(((context_t *)ctx)->inputs);
			data_delete_set(((context_t *)ctx)->outputs);
			data_delete_set(((context_t *)ctx)->params);
			data_delete_set(((context_t *)ctx)->state);
			data_delete_set(((context_t *)ctx)->temp);		
			ctx = (context_t *) objects_get_next(set, (void *) ctx);
		}
		set = objects_get_next_set(context_type_index,set);
	}
	objects_delete_type(context_type_index);
//	context_type_index = -1;
	context_initialized = 0;
	return 0;	
}

// *********************** Utility functions ***********************************

int	context_gcd(int a, int b)
{
	if (a == 0) return 0;
	while (b != 0)
	{
		if (a > b)
		{
			a = a-b;
		}
		else
		{
			b = b -a;
		}
	}
	return a;
}

char	*context_get_name(void *ctx)
{
	if(context_initialized == 0) return "";
	if (ctx == NULL) return "";
	return ((context_t *) ctx)->name;
}

int	context_get_subgraph(void *ctx)
{
	if(context_initialized == 0) return 0;
	if (ctx == NULL) return 0;
	return ((context_t *) ctx)->subgraph;
}

int	context_get_rep_num(void *ctx)
{
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_get_rep_num(): Actor is NULL\n");
		return 0;
	}
	return ((context_t *)ctx)->rep_num;
}

int	context_get_type(void *ctx)
{
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_get_type(): Actor is NULL\n");
		return 0;
	}
	return ((context_t *)ctx)->type;
}

void	context_set_type(void *ctx, int type)
{
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_get_type(): Actor is NULL\n");
		return;
	}
	((context_t *)ctx)->type = type;
}

int	context_set_vector_count(void *ctx)
{
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_set_vector_count(): Not initialized\n");
		return -1;
	}
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_set_vector_count(): Actor is NULL\n");
		return -1;
	}
	if (dataset_set_vector_count(((context_t *) ctx)->inputs) != 0)
	{
		fprintf(stderr, "error: context_set_vector_count(): Can not set input signal vector count\n");
		fprintf(stderr, "\t[actor: %s]\n", ((context_t *) ctx)->name);
		return -1;
	}
	if (dataset_set_vector_count(((context_t *) ctx)->outputs) != 0)
	{
		fprintf(stderr, "error: context_set_vector_count(): Can not set output signal vector count\n");
		fprintf(stderr, "\t[actor: %s]\n", ((context_t *) ctx)->name);
		return -1;
	}
	return 0;
}

void *context_new_actor_token(void *set, void *ctx)
{
	word_t *w;

	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_new_actor_token(): Not initialized\n");
		return NULL;
	}
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_new_actor_token(): Actor is NULL\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: context_new_actor_token(): Token set is NULL\n");
		fprintf(stderr, "\t[actor: %s]\n", ((context_t *) ctx)->name);
		return NULL;
	}
	w = words_new(set);
	if (w == NULL) return NULL;
	w->type = WORD_TYPE_COMP_ACTOR_NAME;
	w->value_type = VALUE_TYPE_PTR;
	w->value.p = ctx;
	words_store_name(w, ((context_t *) ctx)->name);
	return (void *) w;
}

void	context_increment_max_fire_count(void *ctx, int increment)
{
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_increment_max_fire_count(): Not initialized\n");
		return;
	}
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_increment_max_fire_count(): Actor is NULL\n");
		return;
	}
	((context_t *) ctx)->max_fire_count += increment;
	return;
}

int		context_get_max_fire_count(void *ctx)
{
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_get_max_fire_count(): Not initialized\n");
		return -1;
	}
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_get_max_fire_count(): Actor is NULL\n");
		return -1;
	}
	return ((context_t *) ctx)->max_fire_count;
}

void	*context_find(void *set, char *name)
{
	context_t	*c;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_find(): Not initialized\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: context_find(): Actor set is NULL\n");
		return NULL;
	}
	c = (context_t  *) objects_get_first(set);
	while(c != NULL)
	{
		if (strcmp(c->name, name) == 0) return c;
		c = (context_t *) objects_get_next(set, c);
	}
	return NULL;
}

void	contextset_clear_visited(void *set)
{
	context_t	*act = NULL;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_clear_visited(): Not initialized\n");
		return;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_clear_visited(): Actor set is NULL\n");
		return;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		act->visited = 0;
		act = (context_t *) objects_get_next(set, act);
	}
}

void	contextset_clear_fire_count(void *set)
{
	context_t	*act = NULL;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_clear_fire_count(): Not initialized\n");
		return;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_clear_fire_count(): Actor set is NULL\n");
		return;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		act->fire_count = 0;
		act = (context_t *) objects_get_next(set, act);
	}
}

void	contextset_clear_max_fire_count(void *set)
{
	context_t	*act = NULL;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_clear_max_fire_count(): Not initialized\n");
		return;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_clear_max_fire_count(): Actor set is NULL\n");
		return;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		act->max_fire_count = 0;
		act = (context_t *) objects_get_next(set, act);
	}
}

void	contextset_clear_subgraph(void *set)
{
	context_t	*act = NULL;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_clear_subgraph(): Not initialized\n");
		return;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_clear_subgraph(): Actor set is NULL\n");
		return;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		act->subgraph = 0;
		act = (context_t *) objects_get_next(set, act);
	}	
}

int	contextset_set_vector_count(void *set, int sub)
{
	context_t	*act = NULL;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_set_vector_count(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_set_vector_count(): Actor set is NULL\n");
		return -1;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (sub == 0)
		{
			if (context_set_vector_count(act) != 0)
			{
				fprintf(stderr, "error: contextset_set_vector_count(): Can not actor vector count\n");
				fprintf(stderr, "\t[actor: %s]\n", act->name);
				return -1;
			}
		}
		else
		{
			if (act->subgraph == sub)
			{
				if (context_set_vector_count(act) != 0)
				{
					fprintf(stderr, "error: contextset_set_vector_count(): Can not actor vector count\n");
					fprintf(stderr, "\t[actor: %s]\n", act->name);
					return -1;
				}
			}
		}
		act = (context_t *) objects_get_next(set, act);
	}
	return 0;		
}

// *********************** Checking functions **********************************

int	context_check_ports_readers(void *ctx)
{
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_check_ports_readers(): Not initialized\n");
		return -1;
	}
	if (ctx == NULL)
	{
		fprintf(stderr,"error: context_check_ports_readers(): Actor set is NULL\n");
		return -1;
	}
	switch (((context_t *)ctx)->type)
	{
		case	CONTEXT_TYPE_COMPOSITE:
			if (dataset_check_readers(((context_t *)ctx)->inputs) != 0)
			{
				fprintf(stderr,"error: context_check_ports_readers(): In checking input readers\n");
				fprintf(stderr, "\t[actor: %s]\n", ((context_t *)ctx)->name);
				return -1;
			}
			if (dataset_check_readers(((context_t *)ctx)->params) != 0)
			{
				fprintf(stderr,"error: context_check_ports_readers(): In checking parameter readers\n");
				fprintf(stderr, "\t[actor: %s]\n", ((context_t *)ctx)->name);
				return -1;
			}
			break;
		default:
			return 0;
	}
	return 0;
}

int	context_check_ports_writer(void *ctx)
{
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_check_ports_writer(): Not initialized\n");
		return -1;
	}
	if (ctx == NULL)
	{
		fprintf(stderr,"error: context_check_ports_writer(): NULL pointer\n");
		return -1;
	}
	switch (((context_t *)ctx)->type)
	{
		case	CONTEXT_TYPE_COMPOSITE:
			if (dataset_check_writer(((context_t *)ctx)->outputs) != 0)
			{
				fprintf(stderr,"error: context_check_ports_writer(): Can not check outputs writers\n");
				fprintf(stderr, "\t[actor: %s]\n", ((context_t *)ctx)->name);
				return -1;
			}
			break;
		default:
			return 0;
	}
	return 0;
}

int	context_check_ports_connection(void *ctx)
{
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_check_ports_connection(): Not initialized\n");
		return -1;
	}
	if (ctx == NULL)
	{
		fprintf(stderr,"error: context_check_ports_connection(): NULL pointer\n");
		return -1;
	}
	switch (((context_t *)ctx)->type)
	{
		case	CONTEXT_TYPE_COMPOSITE:
			if (dataset_check_connection(((context_t *)ctx)->outputs) != 0) return -1;
			break;
		case	CONTEXT_TYPE_PRIM_INSTANCE:
		case	CONTEXT_TYPE_COMP_INSTANCE:
			if (dataset_check_connection(((context_t *)ctx)->inputs) != 0) return -1;
			if (dataset_check_connection(((context_t *)ctx)->outputs) != 0) return -1;
			if (dataset_check_connection(((context_t *)ctx)->params) != 0) return -1;
			if (dataset_check_connection(((context_t *)ctx)->state) != 0) return -1;
			if (dataset_check_connection(((context_t *)ctx)->temp) != 0) return -1;
			break;
		default:
			return 0;
	}
	return 0;
}

int	context_check_ports_parent(void *ctx)
{
	if (ctx == NULL)
	{
		fprintf(stderr,"error: context_check_ports_parent(): NULL pointer\n");
		return -1;
	}
	if (dataset_check_parent(((context_t *)ctx)->inputs) != 0) return -1;
	if (dataset_check_parent(((context_t *)ctx)->outputs) != 0) return -1;
	if (dataset_check_parent(((context_t *)ctx)->params) != 0) return -1;
	if (dataset_check_parent(((context_t *)ctx)->state) != 0) return -1;
	if (dataset_check_parent(((context_t *)ctx)->temp) != 0) return -1;
	return 0;
}

int	contextset_check_ports_connection(void *set)
{
	context_t	*act;
	
	if (set == NULL)
	{
		fprintf(stderr,"error: contextset_check_ports_connection(): NULL pointer\n");
		return -1;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (context_check_ports_connection(act) != 0) return -1;
		act = (context_t *) objects_get_next(set, act);
	}
	return 0;
}

int	contextset_check_ports_parent(void *set)
{
	context_t	*act;
	
	if (set == NULL)
	{
		fprintf(stderr,"error: contextset_check_ports_parent(): NULL pointer\n");
		return -1;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (context_check_ports_parent(act) != 0) return -1;
		act = (context_t *) objects_get_next(set, act);
	}
	return 0;
}

int contextset_check_schedule_execution(void *set, int sub)
{
	context_t	*act;
	
	if(context_initialized == 0)
	{
		fprintf(stderr,"error: contextset_check_schedule_execution(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr,"error: contextset_check_schedule_execution(): NULL pointer\n");
		return -1;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (act->subgraph == sub)
		{
			if (act->fire_count != act->max_fire_count)
			{
				fprintf(stderr, "error: contextset_check_schedule_execution(): Not enough fireing\n");
				fprintf(stderr, "\t[actor: %s] [fire count=%d] [repetition count=%d]\n", act->name, act->fire_count, act->max_fire_count);
				return -1;
			}
		}
		act = objects_get_next(set, act);
	}
	return 0;
}

int	contextset_check_max_fire_count(void *set, int sub)
{
	context_t	*act;
	
	if (set == NULL) return -1;
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (sub != 0)
		{
			if (act->subgraph == sub)
			{
				if (act->max_fire_count == 0)
				{
					fprintf(stderr, "error: context_check_max_fire_count(): Repetition count is 0\n");
					fprintf(stderr, "\t[actor: %s]\n", act->name);
					return -1;
				}
			}
		}
		else
		{
			if (act->max_fire_count == 0)
			{
				fprintf(stderr, "error: context_check_max_fire_count(): Repetition count is 0\n");
				fprintf(stderr, "\t[actor: %s]\n", act->name);
				return -1;
			}
		}
		act = objects_get_next(set, act);
	}
	return 0;
}

int	contextset_check_vector_count(void *set, int sub)
{
	context_t	*act;
	
	if (set == NULL) return -1;
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (sub != 0)
		{
			if (act->subgraph == sub)
			{
				if (dataset_check_vector_count(act->inputs) != 0)
				{
					fprintf(stderr, "error: contextset_check_vector_count(): Vector count missmatch at input\n");
					fprintf(stderr, "\t[actor: %s]\n", act->name);
					return -1;
				}
				if (dataset_check_vector_count(act->outputs) != 0)
				{
					fprintf(stderr, "error: contextset_check_vector_count(): Vector count missmatch at output\n");
					fprintf(stderr, "\t[actor: %s]\n", act->name);
					return -1;
				}
			}
		}
		else
		{
			if (dataset_check_vector_count(act->inputs) != 0)
			{
				fprintf(stderr, "error: contextset_check_vector_count(): Vector count missmatch at input\n");
				fprintf(stderr, "\t[actor: %s]\n", act->name);
				return -1;
			}
			if (dataset_check_vector_count(act->outputs) != 0)
			{
				fprintf(stderr, "error: contextset_check_vector_count(): Vector count missmatch at output\n");
				fprintf(stderr, "\t[actor: %s]\n", act->name);
				return -1;
			}
		}
		act = objects_get_next(set, act);
	}
	return 0;
}

// ************************* Parsing functions *********************************

void	*context_parse(void *ctx_set, char *name)
{
	void		*inp = NULL;
	void		*scan = NULL;
	void		*hash = NULL;
	word_t		*w = NULL;
	sdferr_t	*scan_err = NULL;
	context_t	*ctx = NULL;
	int			state = 0;
	int			stop = 0;
	int			cur_type = 0;
	int			errnr = 0;
	int			lineno = 0;
	char		inp_name[IO_NAME_LENGTH + 1];
	char		inp_line[SCANNER_LINE_LENGTH + 1];
	char		*inp_path = NULL;
	int			inp_type = 0;

	if (location_find(locations, name, IO_ITEM_TYPE_CONTEXT, &inp_type, &inp_path) != 0)
	{
		fprintf(stderr, "error: context_parse(%s): Can not find context file\n", name);		
		return NULL;
	}
	if (inp_type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: context_parse(%s): Context is not a file\n", name);		
		return NULL;
	}
	if ((inp = io_new_channel(IO_CHAN_TYPE_FILE,inp_path,name,IO_ITEM_TYPE_CONTEXT)) == NULL)
	{
		fprintf(stderr, "error: context_parse(%s): Can not create channel for the context file\n", name);		
		return NULL;
	}
	if (io_open_for_read(inp) != 0)
	{
		fprintf(stderr, "error: context_parse(%s): Can not open channel for the context file\n", name);		
		io_delete_channel(inp);
		return NULL;
	}
	if ((ctx = (context_t *) context_new(ctx_set)) == NULL)
	{
		fprintf(stderr, "error: context_parse(%s): Can not create context object\n", name);		
		io_close(inp);
		io_delete_channel(inp);
		return NULL;
	}
	if ((hash = hash_table_new()) == NULL)
	{
		fprintf(stderr, "error: context_parse(%s): Can not create hash table object\n", name);		
		io_close(inp);
		io_delete_channel(inp);
		context_delete(ctx_set, ctx);
		return NULL;
	}		
	if(src_lang_init_hash_table(hash) != 0)
	{
		fprintf(stderr, "error: context_parse(%s): Can not initialize hash table object\n", name);		
		hash_table_delete(hash);
		io_close(inp);
		io_delete_channel(inp);
		context_delete(ctx_set, ctx);
		return NULL;
	}
	if ((scan = scanner_new()) == NULL)
	{
		fprintf(stderr, "error: context_parse(%s): Can not create scanner object\n", name);		
		hash_table_delete(hash);
		io_close(inp);
		io_delete_channel(inp);
		context_delete(ctx_set, ctx);
		return NULL;
	}
	while (stop == 0)
	{
		if ((w = scanner_scan(scan, inp, hash)) == NULL)
		{
			scan_err = scanner_error(scan);
			switch(scan_err->errnr)
			{
				case	SCANNER_ERROR_EOL:
					cur_type = WORD_TYPE_EOL;
					break;
				case	SCANNER_ERROR_EOF:
					cur_type = WORD_TYPE_EOF;						
					break;
				default:
					errnr = 1;
					stop = -1;
					break;
			}
		}
		else
		{
			cur_type = w->type;
		}
		if (errnr == 0)
		{
			switch(state)
			{
				case	0:
					if ((cur_type == WORD_TYPE_PRIMITIVE))
						{state = 1; ctx->type = CONTEXT_TYPE_PRIMITIVE; break;}
					if ((cur_type == WORD_TYPE_COMPOSITE))
						{state = 2; ctx->type = CONTEXT_TYPE_COMPOSITE; break;}
					if ((cur_type == WORD_TYPE_EOL)) {break;}
					stop = -1;
					errnr = 2;
					break;
				case	1:
					if ((cur_type != WORD_TYPE_LABEL)) {errnr = 3;	stop = -1; break;}
					if (w->valid != 0) { errnr = 22; stop = -1; break;}
					if (words_get_name(w, &ctx->name[0], WORD_NAME_LENGTH + 1) != 0)
						{errnr = 23; stop = -1; break;}
					w->type = WORD_TYPE_PRIM_NAME;
					w->value_type = VALUE_TYPE_PTR;
					w->value.p = (void *) ctx;
					w->valid = -1;
					state = 3;
					break;
				case	2:
					if ((cur_type != WORD_TYPE_LABEL)) {errnr = 3;	stop = -1; break;}
					if (w->valid != 0) { errnr = 17; stop = -1; break;}
					if (words_get_name(w, &ctx->name[0], WORD_NAME_LENGTH + 1) != 0)
							{errnr = 23; stop = -1; break;}
					w->type = WORD_TYPE_COMPOSITE_NAME;
					w->value_type = VALUE_TYPE_PTR;
					w->value.p = (void *) ctx;
					w->valid = -1;
					state = 3;
					break;
				case	3:
					if (cur_type != WORD_TYPE_EOL){stop = -1; errnr = 4;}
					state = 4;
					break;
				case	4:
					if (cur_type == WORD_TYPE_EOL){break;}
					if (cur_type == WORD_TYPE_CONTEXT)
					{
						scanner_push_back_word(scan, w);
						if (context_parse_details((void *)ctx, inp, scan, hash) != 0)
							{ stop = -1; errnr = 24; lineno = scanner_lineno(scan); break;}
					}
					state = 5;
					break;
				case	5:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_END:
							state = 6;
							break;
						default:
							stop = -1;
							errnr = 20;
							break;
					}
					break;
				case	6:
					if ((cur_type == WORD_TYPE_EOL)) { stop = -1;}
					else {stop = -1; errnr = 21;}
					break;

			}
		}
	}
	if (errnr != 0)
	{
		lineno = scanner_lineno(scan);
		if (io_get_name(inp,inp_name, IO_NAME_LENGTH + 1) != 0)
			inp_name[0] = 0;
		if (scanner_get_line(scan, inp_line, SCANNER_LINE_LENGTH + 1) != 0)
			inp_line[0] = 0;
		fprintf(stderr, "[input:%s] [line:%d] [context parse error:%d]\n", inp_name, lineno, errnr);
		fprintf(stderr, "[source] %s\n", inp_line);
		fprintf(stderr, "[description] %s\n", context_error_description[errnr]);
		context_delete(ctx_set, ctx);
		ctx = NULL;
	}
	scanner_delete(scan);
	hash_table_delete(hash);
	io_close(inp);
	io_delete_channel(inp);
	return ctx;
}

int		context_parse_details(void *ctx, void *inp, void *scan, void *hash)
{
	context_t	*c = (context_t *) ctx;
	word_t		*w = NULL;
	sdferr_t	*scan_err = NULL;
	void		*dat = NULL;
	int			state = 0;
	int			stop = 0;
	int			cur_type = 0;
	int			errnr = 0;
	int			lineno = 0;
	char		inp_name[IO_NAME_LENGTH + 1];
	char		inp_line[SCANNER_LINE_LENGTH + 1];
	
	if ((c->inputs = data_new_set()) == NULL) return -1;
	if ((c->outputs = data_new_set()) == NULL) return -1;
	if ((c->params = data_new_set()) == NULL) return -1;
	if ((c->state = data_new_set()) == NULL) return -1;
	if ((c->temp = data_new_set()) == NULL) return -1;
	while (stop == 0)
	{
		if ((w = scanner_scan(scan, inp, hash)) == NULL)
		{
			scan_err = scanner_error(scan);
			switch(scan_err->errnr)
			{
				case	SCANNER_ERROR_EOL:
					cur_type = WORD_TYPE_EOL;
					break;
				case	SCANNER_ERROR_EOF:
					cur_type = WORD_TYPE_EOF;						
					break;
				default:
					errnr = 1;
					stop = -1;
					break;
			}
		}
		else
		{
			cur_type = w->type;
		}
		if (errnr == 0)
		{
			switch(state)
			{
				case	0:	
					if (cur_type == WORD_TYPE_EOL){break;}
					if ((cur_type == WORD_TYPE_CONTEXT)){state = 1; break;}
					stop = -1;
					errnr = 25;
					break;
				case	1:
					if (cur_type == WORD_TYPE_EOL){state = 4;}
					else {stop = -1; errnr = 4;}
					break;
// There is no state value of 2 or 3 due to some code modification
				case	4:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_INPUT:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->inputs)) == NULL)
								{stop = -1; errnr = 5;	break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 6; break;}
							data_set_parent(dat, ctx);
							state = 5;
							break;
						case	WORD_TYPE_OUTPUT:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->outputs)) == NULL)
								{stop = -1; errnr = 7;	break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 8; break;}
							data_set_parent(dat, ctx);
							state = 6;
							break;
						case	WORD_TYPE_END:
							state = 10;
							break;
						default:
							stop = -1;
							errnr = 9;
							break;
					}
					break;
				case	5:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_INPUT:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->inputs)) == NULL)
								{stop = -1; errnr = 5;	break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 6; break;}
							data_set_parent(dat, ctx);
							state = 5;
							break;
						case	WORD_TYPE_OUTPUT:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->outputs)) == NULL)
								{stop = -1; errnr = 7;	break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 8; break;}
							data_set_parent(dat, ctx);
							state = 6;
							break;
						case	WORD_TYPE_PARAM:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->params)) == NULL)
								{stop = -1; errnr = 10;	break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 11; break;}
							data_set_parent(dat, ctx);
							state = 7;
							break;
						case	WORD_TYPE_STATE:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->state)) == NULL)
								{stop = -1; errnr = 12; break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 13; break;}
							data_set_parent(dat, ctx);
							state = 8;
							break;
						case	WORD_TYPE_TEMP:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->temp)) == NULL)
								{stop = -1; errnr = 14; break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 15; break;}
							data_set_parent(dat, ctx);
							state = 9;
							break;
						case	WORD_TYPE_END:
							state = 10;
							break;
						default:
							stop = -1;
							errnr = 16;
							break;
					}
					break;
				case	6:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_OUTPUT:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->outputs)) == NULL)
								{stop = -1; errnr = 7;	break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 8; break;}
							data_set_parent(dat, ctx);
							state = 6;
							break;
						case	WORD_TYPE_PARAM:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->params)) == NULL)
								{stop = -1; errnr = 10;	break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 11; break;}
							data_set_parent(dat, ctx);
							state = 7;
							break;
						case	WORD_TYPE_STATE:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->state)) == NULL)
								{stop = -1; errnr = 12; break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 13; break;}
							data_set_parent(dat, ctx);
							state = 8;
							break;
						case	WORD_TYPE_TEMP:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->temp)) == NULL)
								{stop = -1; errnr = 14; break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 15; break;}
							data_set_parent(dat, ctx);
							state = 9;
							break;
						case	WORD_TYPE_END:
							state = 10;
							break;
						default:
							stop = -1;
							errnr = 17;
							break;
					}
					break;
				case	7:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_PARAM:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->params)) == NULL)
								{stop = -1; errnr = 10;	break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 11; break;}
							data_set_parent(dat, ctx);
							state = 7;
							break;
						case	WORD_TYPE_STATE:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->state)) == NULL)
								{stop = -1; errnr = 12; break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 13; break;}
							data_set_parent(dat, ctx);
							state = 8;
							break;
						case	WORD_TYPE_TEMP:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->temp)) == NULL)
								{stop = -1; errnr = 14; break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 15; break;}
							data_set_parent(dat, ctx);
							state = 9;
							break;
						case	WORD_TYPE_END:
							state = 10;
							break;
						default:
							stop = -1;
							errnr = 18;
							break;
					}
					break;
				case	8:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_TEMP:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(c->temp)) == NULL)
								{stop = -1; errnr = 14; break;}
							if (data_parse(dat, inp, scan, hash) != 0)
								{stop = -1; errnr = 15; break;}
							data_set_parent(dat, ctx);
							state = 9;
							break;
						case	WORD_TYPE_END:
							state = 10;
							break;
						default:
							stop = -1;
							errnr = 19;
							break;
					}
					break;
				case	9:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_END:
							state = 10;
							break;
						default:
							stop = -1;
							errnr = 20;
							break;
					}
					break;
				case	10:
					if (cur_type == WORD_TYPE_EOL) { stop = -1;}
					else {stop = -1; errnr = 21;}
					break;
			}
		}
	}
	if (errnr == 0)
	{
		return 0;
	}
	else
	{
		lineno = scanner_lineno(scan);
		if (io_get_name(inp,inp_name, IO_NAME_LENGTH + 1) != 0)
			inp_name[0] = 0;
		if (scanner_get_line(scan, inp_line, SCANNER_LINE_LENGTH + 1) != 0)
			inp_line[0] = 0;
		fprintf(stderr, "context_parse_details(): got source line\n");
		fprintf(stderr, "[input:%s] [line:%d] [context parse details error:%d]\n", inp_name, lineno, errnr);
		fprintf(stderr, "[source] %s\n", inp_line);
		fprintf(stderr, "[description] %s\n", context_error_description[errnr]);
		return -1;
	}
}
/*
int		context_load_hash(void *ctx, void *hash, int flag)
{
	context_t	*c = (context_t *) ctx;
	word_t		*w;
	
	if (ctx == NULL) return -1;
	if(context_initialized == 0) return -1;
	
	if (flag == -1)
	{
		w = hash_table_lookup(hash, c->name);
		if (w == NULL) return -1;
		if (w->type != WORD_TYPE_LABEL) return -1;
		if (w->valid != 0) return -1;
	}
	switch (c->type)
	{
		case	CONTEXT_TYPE_PRIMITIVE:
			w->type = WORD_TYPE_PRIM_NAME;
			break;
		case	CONTEXT_TYPE_COMPOSITE:
			w->type = WORD_TYPE_COMPOSITE_NAME;
			break;
		case	CONTEXT_TYPE_PRIM_INSTANCE:
			w->type = WORD_TYPE_PRIM_ACTOR_NAME;
			break;
		case	CONTEXT_TYPE_COMP_INSTANCE:
			w->type = WORD_TYPE_COMP_ACTOR_NAME;
			break;
		default:
			return -1;
	}
	w->value_type = VALUE_TYPE_PTR;
	w->value.p = (void *) c;

	if (data_set_load_hash(c->inputs, hash) != 0) return -1;
	if (data_set_load_hash(c->outputs, hash) != 0) return -1;
	if (data_set_load_hash(c->params, hash) != 0) return -1;
	if (data_set_load_hash(c->state, hash) != 0) return -1;
	if (data_set_load_hash(c->temp, hash) != 0) return -1;

	return 0;
}
*/
// *********************** Source code listing functions ***********************

void	context_print_set(void *set, char *prefix, void *out)
{
	context_t	*ctx;
	
	if (set == NULL) return;
	if (strlen(prefix) > 32) return;
	if(context_initialized == 0) return;
	ctx = objects_get_first(set);
	while (ctx != NULL)
	{
		context_print(ctx, prefix, out);
		ctx = objects_get_next(set, ctx);
	}
	return;
}

void	context_print_set_connections(void *set, char *prefix, void *out)
{
	context_t	*ctx;
	
	if (set == NULL) return;
	if (strlen(prefix) > 32) return;
	if(context_initialized == 0) return;
	ctx = objects_get_first(set);
	while (ctx != NULL)
	{
		context_print_connections(ctx, prefix, out);
		ctx = objects_get_next(set, ctx);
	}
	return;	
}

void	context_print_connections(void *ctx, char *prefix, void *out)
{
	context_t	*c = (context_t *) ctx;
	
	if (ctx == NULL) return;
	if(context_initialized == 0) return;
	data_print_set_connections(c->inputs, prefix, out);
	data_print_set_connections(c->outputs, prefix, out);
	data_print_set_connections(c->params, prefix, out);
	data_print_set_connections(c->state, prefix, out);
	data_print_set_connections(c->temp, prefix, out);
}

void	context_print(void *ctx, char *prefix, void *out)
{
	context_t	*c = (context_t *) ctx;
	char		line[SCANNER_LINE_LENGTH + 1];
	char		pf[35];
	int			l;
	
	if (ctx == NULL) return;
	if ((l = strlen(prefix)) > 32) return;
	if(context_initialized == 0) return;
	strcpy(pf, prefix);
	strcpy(&pf[l], "\t");
//	fprintf(stderr, "context_print() : type=%d, name=%s\n", c->type, c->name);
	switch (c->type)
	{
		case	CONTEXT_TYPE_PRIMITIVE:
			sprintf(line, "%sprimitive\t%s", prefix, c->name);
			if (io_put(out,line) != 0) return;
			break;
		case	CONTEXT_TYPE_COMPOSITE:
			sprintf(line, "%scomposite\t%s", prefix, c->name);
			if (io_put(out,line) != 0) return;
			break;
		case	CONTEXT_TYPE_PRIM_INSTANCE:
			sprintf(line, "%sprimitive\t%s\tinstance\t%s", prefix, ((context_t *) (c->root))->name, c->name);
			if (io_put(out,line) != 0) return;
			break;
		case	CONTEXT_TYPE_COMP_INSTANCE:
			sprintf(line, "%scomposite\t%s\tinstance\t%s", prefix, ((context_t *) (c->root))->name, c->name);
			if (io_put(out,line) != 0) return;
			break;
	}
	context_print_details(ctx, pf, out);
	sprintf(line, "%send", prefix);
	if (io_put(out,line) != 0) return;
}

void context_print_details(void *ctx, char *prefix, void *out)
{
	context_t	*c = (context_t *) ctx;
	char		line[SCANNER_LINE_LENGTH + 1];
	char		pf[35];
	int			l;
	
	if (ctx == NULL) return;
	if ((l = strlen(prefix)) > 32) return;
	if(context_initialized == 0) return;
	strcpy(pf, prefix);
	strcpy(&pf[l], "\t");
	sprintf(line, "%scontext", prefix);
	if (io_put(out,line) != 0) return;
	data_print_set(c->inputs, pf, out);
	data_print_set(c->outputs, pf, out);
	data_print_set(c->params, pf, out);
	data_print_set(c->state, pf, out);
	data_print_set(c->temp, pf, out);
	sprintf(line, "%send", prefix);
	if (io_put(out,line) != 0) return;
}

void contextset_print_repetition(void *set, int sub)
{
	context_t	*act;
	
	if (set == NULL) return;
	fprintf(stderr, "repetion\n");
	act = (context_t *) objects_get_first(set);
	while(act != NULL)
	{
		if (sub != 0)
		{
			if (act->subgraph == sub)
			{
				fprintf(stderr, "\t%s\t%d\t%d/%d\n", act->name,act->max_fire_count,act->rep_num, act->rep_denom);
			}
		}
		else
		{
			fprintf(stderr, "\t%s\t%d\t%d/%d\n", act->name,act->max_fire_count,act->rep_num, act->rep_denom);
		}
		act = objects_get_next(set, act);
	}
	fprintf(stderr, "end repetion\n");
}

// *********************** Computing schedule ********************************


int	contextset_compute_repetition(void *set, int sub)
{
	context_t	*act;
	int	l;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_compute_repetition(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_compute_repetition(): Actor set is NULL\n");
		return -1;
	}
	if (sub <= 0)
	{
		fprintf(stderr, "error: contextset_compute_repetition(): Invalid subgraph index\n");
		fprintf(stderr, "\t[subgraph: %d]\n", sub);
		return -1;
	}
	if (_context_set_clear_repetition(set, sub) != 0)
	{
		fprintf(stderr, "error: contextset_compute_repetition(): In clearing repetition vector\n");
		return -1;
	}
	act = (context_t *)objects_get_first(set);
	while (act != NULL)
	{
		if (act->subgraph == sub) break;
		act = (context_t *) objects_get_next(set, act);
	}
	if (act == NULL)
	{
		fprintf(stderr, "error: contextset_compute_repetition(): Subgraph is empty\n");
		return -1;
	}
	if (context_set_repetition(act, 1, 1) != 0)
	{
		fprintf(stderr, "error: contextset_compute_repetition(): In setting actor's repetition count\n");
		fprintf(stderr, "\t[actor: %s]\n", act->name);
		return -1;
	}
	if ((l = _context_set_compute_rep_denom_lcm(set, sub)) == 0)
	{
		fprintf(stderr, "error: contextset_compute_repetition(): Can not compute LCM\n");
		return -1;
	}
	while(act != NULL)
	{
		if (act->subgraph == sub)
		{
			act->max_fire_count = act->rep_num * (l / act->rep_denom);
		}
		act = (context_t *)objects_get_next(set, act);
	}
	return 0;
}

int	contextset_set_repetition(void *set, int sub, int rep)
{
	context_t	*act;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_set_repetition(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_set_repetition(): Actor set is NULL\n");
		return -1;
	}
	act = (context_t *)objects_get_first(set);
	while(act != NULL)
	{
		if (act->subgraph == sub)
		{
			act->max_fire_count = rep;
		}
		act = (context_t *)objects_get_next(set, act);
	}
	
	return 0;
}


int	context_set_repetition(void *ctx, int num, int denom)
{
	context_t	*act = (context_t *) ctx;
	
	act->rep_num = num;
	act->rep_denom = denom;
	if (dataset_set_output_repetition(act->outputs, num, denom) != 0)
	{
		fprintf(stderr, "error: context_set_repetition(): Cannot set repetition count on outputs\n");
		fprintf(stderr, "\t[actor: %s]\n", act->name);
		return -1;
	}
	if (dataset_set_input_repetition(act->inputs, num, denom) != 0)
	{
		fprintf(stderr, "error: context_set_repetition(): Cannot set repetition count on inputs\n");
		fprintf(stderr, "\t[actor: %s]\n", act->name);
		return -1;
	}
	return 0;
}

int contextset_init_ports_cur_count(void *set, int sub)
{
	context_t	*act;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_init_ports_cur_count(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_init_ports_cur_count(): Actor set is NULL\n");
		return -1;
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (sub != 0)
		{
			if (act->subgraph == sub)
			{
				act->fire_count = 0;
				if (dataset_init_cur_count(act->inputs) != 0)
				{
					fprintf(stderr, "error: contextset_init_ports_cur_count(): Can not initialize inputs fire count\n");
					fprintf(stderr, "\t[actor: %s]\n", act->name);
					return -1;
				}
				if (dataset_init_cur_count(act->outputs) != 0)
				{
					fprintf(stderr, "error: contextset_init_ports_cur_count(): Can not initialize outputs fire count\n");
					fprintf(stderr, "\t[actor: %s]\n", act->name);
					return -1;
				}
			}
		}
		else
		{
			act->fire_count = 0;
			if (dataset_init_cur_count(act->inputs) != 0) return -1;
			{
				fprintf(stderr, "error: contextset_init_ports_cur_count(): Can not initialize inputs data count\n");
				fprintf(stderr, "\t[actor: %s]\n", act->name);
				return -1;
			}
			if (dataset_init_cur_count(act->outputs) != 0) return -1;
			{
				fprintf(stderr, "error: contextset_init_ports_cur_count(): Can not initialize outputs data count\n");
				fprintf(stderr, "\t[actor: %s]\n", act->name);
				return -1;
			}
		}
		act = objects_get_next(set, act);
	}
	return 0;
}

void *contextset_compute_fireing(void *set, int sub)
{
	context_t	*act;
	void		*ptr;
	void		*ptrset;
	int			fireable, one_fired;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_compute_fireing(): Not initialized\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_compute_fireing(): Actor set is NULL\n");
		return NULL;
	}
	if ((ptrset = pointer_new_set()) == NULL)
	{
		fprintf(stderr, "error: contextset_compute_fireing(): Can not create pointer set\n");
		return NULL;
	}
	if (contextset_init_ports_cur_count(set, sub) != 0)
	{
		objects_delete_set(ptrset);
		fprintf(stderr, "error: contextset_compute_fireing(): Can not initialize ports data counts\n");
		return NULL;
	}
	while (1)
	{
		one_fired = 0;
		act = (context_t *) objects_get_first(set);
		while (act != 0)
		{
			if (act->subgraph == sub)
			{
				if (act->fire_count < act->max_fire_count)
				{
					if ((fireable = dataset_is_input_fireable(act->inputs)) == -1)
					{
						fprintf(stderr, "error: contextset_compute_fireing(): Can not check if inputs has enough data for fireing\n");
						fprintf(stderr, "\t[actor: %s]\n", act->name);
						objects_delete_set(ptrset);
						return NULL;
					}
					if (fireable == 1)
					{
						if (dataset_increment_cur_count(act->inputs) != 0)
						{
							fprintf(stderr, "error: contextset_compute_fireing(): Can not increment inputs data count\n");
							fprintf(stderr, "\t[actor: %s]\n", act->name);
							objects_delete_set(ptrset);
							return NULL;
						}
						if (dataset_increment_cur_count(act->outputs) != 0)
						{
							fprintf(stderr, "error: contextset_compute_fireing(): Can not increment outputs data count\n");
							fprintf(stderr, "\t[actor: %s]\n", act->name);
							objects_delete_set(ptrset);
							return NULL;
						}
						if ((ptr = pointer_new(ptrset, act)) == NULL)
						{
							fprintf(stderr, "error: contextset_compute_fireing(): Can not create new pointer object\n");
							fprintf(stderr, "\t[actor: %s]\n", act->name);
							objects_delete_set(ptrset);
							return NULL;
						}
						act->fire_count++;
						one_fired = -1;
					}
				}
			}
			act = objects_get_next(set, act);
		}
		if (one_fired == 0) break;
	}
	return ptrset;
}

int context_fire(void *ctx)
{
	context_t	*act = (context_t *) ctx;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_fire(): Not initialized\n");
		return -1;
	}
	if (ctx == NULL)
	{
		fprintf(stderr, "error: context_fire(): Actor is NULL\n");
		return -1;
	}
	if (act->fire_count < act->max_fire_count)
	{
		if (dataset_check_input_fireability(act->inputs) != 0)
		{
			fprintf(stderr,"error: context_fire(): Actor could not be fired");
			fprintf(stderr,"\t[actor: %s] [fire count: %d]\n", act->name, act->fire_count);
			return -1;
		}
		if (dataset_increment_cur_count(act->inputs) != 0)
		{
			fprintf(stderr, "error: context_fire(): Can not increment inputs data count\n");
			fprintf(stderr, "\t[actor: %s]\n", act->name);
			return -1;
		}
		if (dataset_increment_cur_count(act->outputs) != 0)
		{
			fprintf(stderr, "error: context_fire(): Can not increment outputs data count\n");
			fprintf(stderr, "\t[actor: %s]\n", act->name);
			return -1;
		}
		act->fire_count++;
	}
	return 0;
}

int	context_mark_connected(void *act, int sub)
{
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: context_mark_connected(): Not initialized\n");
		return -1;
	}
	if (act == NULL)
	{
		fprintf(stderr, "error: context_mark_connected(): Actor is NULL\n");
		return -1;
	}
	if (((context_t *)act)->visited != 0) return 0;
	((context_t *)act)->visited = -1;
	((context_t *)act)->subgraph = sub;
	if (((context_t *)act)->inputs != NULL)
	{
		if (dataset_mark_connected(((context_t *)act)->inputs, sub) != 0)
		{
			fprintf(stderr, "error: context_mark_connected(): In marking actors connected to input ports\n");
			fprintf(stderr, "\t[actor: %s]\n", ((context_t *)act)->name);
			return -1;
		}
	}
	if (((context_t *)act)->outputs != NULL)
	{	
		if (dataset_mark_connected(((context_t *)act)->outputs, sub) != 0)
		{
			fprintf(stderr, "error: context_mark_connected(): In marking actors connected to output ports\n");
			fprintf(stderr, "\t[actor: %s]\n", ((context_t *)act)->name);
			return -1;
		}
	}
	return 0;
}

int contextset_mark_isolated_actors(void *set)
{
	context_t	*act;
	int			ret, found;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_mark_isolated_actors(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_mark_isolated_actors(): Actor set is NULL\n");
		return -1;
	}
	found = 0;
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		ret = dataset_is_not_for_scheduling(act->inputs);
		if (ret == -1)
		{
			fprintf(stderr, "error: contextset_mark_isolated_actors(): In checking inputs connectivity\n");
			fprintf(stderr, "\t[actor: %s]\n", act->name);
			return -1;
		}
		if (ret != 0)
		{
			ret = dataset_is_not_for_scheduling(act->outputs);
			if (ret == -1)
			{
				fprintf(stderr, "error: contextset_mark_isolated_actors(): In checking outputs connectivity\n");
				fprintf(stderr, "\t[actor: %s]\n", act->name);
				return -1;
			}
			if (ret != 0)
			{
				act->subgraph = -1;
				found++;
			}
		}
		act = (context_t *) objects_get_next(set, act);
	}
	return found;
}

int	contextset_mark_subgraphs(void *set)
{
	context_t	*act;
	int			ret, found, sub;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_mark_subgraphs(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_mark_subgraphs(): Actor set is NULL\n");
		return -1;
	}
	contextset_clear_visited(set);
	found = -1;
	sub = 0;
	while (found != 0)
	{
		found = 0;
		act = (context_t *) objects_get_first(set);
		while (act != NULL)
		{
			if (act->subgraph == 0)
			{
				if (act->visited == 0)
				{
					found = -1;
					sub++;
					ret = context_mark_connected(act, sub);
					if (ret != 0)
					{
						fprintf(stderr, "error: contextset_mark_subgraphs(): Error in marking connected actors\n");
						fprintf(stderr, "\t[actor: %s]\n", act->name);
						return -1;
					}
				}
			}
			act = (context_t *) objects_get_next(set, act);
		}
	}
	return sub;
}

void	*contextset_compute_isolated_fireing(void *set, int sub)
{
	void		*ptrset;
	context_t	*act;
	int			ret;
	
	if(context_initialized == 0)
	{
		fprintf(stderr, "error: contextset_compute_isolated_fireing(): Not initialized\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: contextset_compute_isolated_fireing(): Actor set is NULL\n");
		return NULL;
	}
	contextset_clear_visited(set);
	if ((ptrset = pointer_new_set()) == NULL)
	{
		fprintf(stderr, "error: contextset_compute_isolated_fireing(): Can not create pointer set\n");
		return NULL;		
	}
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (act->subgraph == sub)
		{
			ret = dataset_is_output_not_connected_in_sub(act->outputs, -1);
			if (ret == -1)
			{
				fprintf(stderr, "error: contextset_compute_isolated_fireing(): In checking output connectivity\n");
				objects_delete_set(ptrset);
				return NULL;		
			}
			if (ret == 1)
			{
				ret = _context_sort_isolated_actors(act, ptrset);
				if (ret != 0) 
				{
					objects_delete_set(ptrset);
					fprintf(stderr, "error: contextset_compute_isolated_fireing(): In sorting isolated actors\n");
					fprintf(stderr, "\t[actor: %s]\n", act->name);
					return NULL;		
				}
			}
		}
		act = (context_t *) objects_get_next(set, act);
	}
	return ptrset;
}

//************************ Generate asm code ***********************************

int	context_set_generate_asm_actor_name_table(void *set, void *out)
{
	context_t	*act;
	char		line[SCANNER_LINE_LENGTH + 1];
	
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		sprintf(line, src_lang_get_asm_line(L_DEF_NAME),act->name, act->name);
		if (io_put(out,line) != 0) return -1;
		act = (context_t *) objects_get_next(set, act);
	}
	return 0;
}

int	context_generate_asm_port_name_table(void *ctx, void *out)
{
	if (data_set_generate_asm_name_table(((context_t *)ctx)->inputs, out) != 0) return -1;
	if (data_set_generate_asm_name_table(((context_t *)ctx)->outputs, out) != 0) return -1;
	if (data_set_generate_asm_name_table(((context_t *)ctx)->params, out) != 0) return -1;
	if (data_set_generate_asm_name_table(((context_t *)ctx)->state, out) != 0) return -1;
	if (data_set_generate_asm_name_table(((context_t *)ctx)->temp, out) != 0) return -1;
	return 0;
}

int	context_generate_asm_port_pointers(void *ctx, void *out)
{
	if (data_set_generate_asm_port_pointer(((context_t *)ctx)->inputs, out) != 0) return -1;
	if (data_set_generate_asm_port_pointer(((context_t *)ctx)->outputs, out) != 0) return -1;
	if (data_set_generate_asm_port_pointer(((context_t *)ctx)->params, out) != 0) return -1;
	if (data_set_generate_asm_port_pointer(((context_t *)ctx)->state, out) != 0) return -1;
	if (data_set_generate_asm_port_pointer(((context_t *)ctx)->temp, out) != 0) return -1;
	return 0;
}

int context_set_generate_asm_load_script(void *set, void *out)
{
	context_t	*act;
	char		line[SCANNER_LINE_LENGTH + 1];
	
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (act->type == CONTEXT_TYPE_PRIMITIVE)
//		if (act->type == CONTEXT_TYPE_PRIM_INSTANCE)
		{
			sprintf(line, src_lang_get_asm_line(L_LOAD_PRIM),act->name);
//			sprintf(line, src_lang_get_asm_line(L_LOAD_PRIM),((context_t *)act->root)->name,act->name);
			if (io_put(out,line) != 0) return -1;
		}
		if (act->type == CONTEXT_TYPE_COMPOSITE)
//		if (act->type == CONTEXT_TYPE_COMP_INSTANCE)
		{
			sprintf(line, src_lang_get_asm_line(L_LOAD_COMP),act->name);
//			sprintf(line, src_lang_get_asm_line(L_LOAD_COMP),((context_t *)act->root)->name,act->name);
			if (io_put(out,line) != 0) return -1;
		}
		act = (context_t *) objects_get_next(set, act);
	}
	
	return 0;
}

int context_set_generate_asm_make_script(void *set, void *out)
{
	context_t	*act;
	char		line[SCANNER_LINE_LENGTH + 1];
	
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (act->type == CONTEXT_TYPE_PRIM_INSTANCE)
		{
			sprintf(line, src_lang_get_asm_line(L_MAKE_PRIM),((context_t *)act->root)->name,act->name,act->name);
			if (io_put(out,line) != 0) return -1;
		}
		if (act->type == CONTEXT_TYPE_COMP_INSTANCE)
		{
			sprintf(line, src_lang_get_asm_line(L_MAKE_COMP),((context_t *)act->root)->name,act->name,act->name);
			if (io_put(out,line) != 0) return -1;
		}
		act = (context_t *) objects_get_next(set, act);
	}
	
	return 0;
}

int	context_set_generate_asm_actor_port_init(void *set, void *out)
{
	context_t	*act;
	
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (data_set_generate_asm_actor_port_init(act->inputs, out) != 0) return -1;
		if (data_set_generate_asm_actor_port_init(act->outputs, out) != 0) return -1;
		if (data_set_generate_asm_actor_port_init(act->params, out) != 0) return -1;
		if (data_set_generate_asm_actor_port_init(act->state, out) != 0) return -1;
		if (data_set_generate_asm_actor_port_init(act->temp, out) != 0) return -1;
		act = objects_get_next(set, act);
	}
	return 0;
}


int	context_generate_asm_actor_port_update(void *ctx, void *out)
{
	context_t	*act = (context_t *) ctx;
	
	if (data_set_generate_asm_actor_port_update(act->inputs, out) != 0) return -1;
	if (data_set_generate_asm_actor_port_update(act->outputs, out) != 0) return -1;
	return 0;
}


int	context_set_generate_asm_actor_init_call(void *set, void *out)
{
	context_t	*act;
	char		line[SCANNER_LINE_LENGTH + 1];

	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (act->type == CONTEXT_TYPE_PRIM_INSTANCE)
		{
			sprintf(line,src_lang_get_asm_line(L_INIT_PRIM), act->name);
			if (io_put(out,line) != 0) return -1;
		}
		if (act->type == CONTEXT_TYPE_COMP_INSTANCE)
		{
			sprintf(line,src_lang_get_asm_line(L_INIT_COMP), act->name);
			if (io_put(out,line) != 0) return -1;
		}
		act = objects_get_next(set, act);		
	}
	return 0;
}

int	context_generate_asm_actor_fire_call(void *ctx, void *out)
{
	context_t	*act = (context_t *)ctx;
	char		line[SCANNER_LINE_LENGTH + 1];
	
	if (act->type == CONTEXT_TYPE_PRIM_INSTANCE)
	{
		sprintf(line,src_lang_get_asm_line(L_FIRE_PRIM), act->name);
		if (io_put(out,line) != 0) return -1;
	}
	if (act->type == CONTEXT_TYPE_COMP_INSTANCE)
	{
		sprintf(line,src_lang_get_asm_line(L_FIRE_COMP), act->name);
		if (io_put(out,line) != 0) return -1;
	}
	return 0;
}

int	context_set_generate_asm_actor_clean_call(void *set, void *out)
{
	context_t	*act;
	char		line[SCANNER_LINE_LENGTH + 1];
	
	act = (context_t *) objects_get_first(set);
	while (act != NULL)
	{
		if (act->type == CONTEXT_TYPE_PRIM_INSTANCE)
		{
			sprintf(line,src_lang_get_asm_line(L_CLEAN_PRIM), act->name);
			if (io_put(out,line) != 0) return -1;
		}
		if (act->type == CONTEXT_TYPE_COMP_INSTANCE)
		{
			sprintf(line,src_lang_get_asm_line(L_CLEAN_COMP), act->name);
			if (io_put(out,line) != 0) return -1;
		}
		act = objects_get_next(set, act);		
	}
	return 0;
}

int	context_set_generate_asm_actor_instances(void *set, void *out)
{
	context_t	*act;
	char		line[SCANNER_LINE_LENGTH + 1];
	int			ret;
	int			port_count = 0;

	act = objects_get_first(set);
	while (act != NULL)
	{
		sprintf(line,src_lang_get_asm_line(L_DEF_ACT), act->name);
		if (io_put(out,line) != 0) return -1;
		sprintf(line,src_lang_get_asm_line(L_DEF_ACT), "");
		if (io_put(out,line) != 0) return -1;
		if ((ret = objects_get_set_size(act->inputs)) > 0) port_count += ret;
		if ((ret = objects_get_set_size(act->outputs)) > 0) port_count += ret;
		if ((ret = objects_get_set_size(act->params)) > 0) port_count += ret;
		if ((ret = objects_get_set_size(act->state)) > 0) port_count += ret;
		if ((ret = objects_get_set_size(act->temp)) > 0) port_count += ret;
		if (port_count == 0)
		{
			sprintf(line,src_lang_get_asm_line(L_DEF_ACT), "");
			if (io_put(out,line) != 0) return -1;
		}
		else
		{
			if (data_set_generate_asm_port_pointer(act->inputs, out) != 0) return -1;
			if (data_set_generate_asm_port_pointer(act->outputs, out) != 0) return -1;
			if (data_set_generate_asm_port_pointer(act->params, out) != 0) return -1;
			if (data_set_generate_asm_port_pointer(act->state, out) != 0) return -1;
			if (data_set_generate_asm_port_pointer(act->temp, out) != 0) return -1;
		}
		act = objects_get_next(set, act);
	}	
	return 0;
}

// **************************** Generating c code ******************************

void context_generate_c_header(void *ctx, void *out)
{
	context_t	*c = (context_t *) ctx;
	char		buf[128];
	
	if (ctx == NULL) return;
	sprintf(buf, "//****************** %s context header *********************\n\n", c->name);
	io_write(out, buf, strlen(buf));
	sprintf(buf, "#ifndef\t%s_CONTEX_H\n", c->name);
	io_write(out, buf, strlen(buf));
	sprintf(buf, "#define\t%s_CONTEX_H\n\n", c->name);
	io_write(out, buf, strlen(buf));
	sprintf(buf, "typedef struct _context\n");
	io_write(out, buf, strlen(buf));
	sprintf(buf, "{\n");
	io_write(out, buf, strlen(buf));
	sprintf(buf, "// Inputs\n");
	io_write(out, buf, strlen(buf));
	data_set_generate_c_header(c->inputs, out);
	sprintf(buf, "// Outputs\n");
	io_write(out, buf, strlen(buf));
	data_set_generate_c_header(c->outputs, out);
	sprintf(buf, "// Parameters\n");
	io_write(out, buf, strlen(buf));
	data_set_generate_c_header(c->params, out);
	sprintf(buf, "// State\n");
	io_write(out, buf, strlen(buf));
	data_set_generate_c_header(c->state, out);
	sprintf(buf, "// Temp storage\n");
	io_write(out, buf, strlen(buf));
	data_set_generate_c_header(c->temp, out);
	sprintf(buf, "} context_t;\n\n");
	io_write(out, buf, strlen(buf));
	sprintf(buf, "void %s_initialize(const context_t * const ctx);\n", c->name);
	io_write(out, buf, strlen(buf));
	sprintf(buf, "void %s_fire(const context_t * const ctx);\n", c->name);
	io_write(out, buf, strlen(buf));
	sprintf(buf, "void %s_cleanup(const context_t * const ctx);\n\n", c->name);
	io_write(out, buf, strlen(buf));
	sprintf(buf, "#endif\n");
	io_write(out, buf, strlen(buf));
}

/* *****************************************************************************
 *					Private functions
 ******************************************************************************/
 
static int _context_lcm(int a, int b)
{
	int	g;
	
	g = context_gcd(a,b);
	if (g == 0) return 0;
	return (a * b)/g;
}

static int _context_set_compute_rep_denom_lcm(void *set, int sub)
{
	int l;
	context_t	*act;
	
	if (set == NULL) return 0;
	l = 1;
	act = objects_get_first(set);
	while (act != NULL)
	{
		if (act->subgraph == sub)
		{
			l = _context_lcm(l, act->rep_denom);
		}
		act = (context_t *)objects_get_next(set, act);
	}
	return l;
}

static int	_context_set_clear_repetition(void *set, int sub)
{
	context_t	*act;
	
	if (set == NULL)
	{
		fprintf(stderr, "error: context_set_clear_repetition(): Actor set is NULL\n");
		return -1;
	}
	act = (context_t *)objects_get_first(set);
	while(act != NULL)
	{
		if (act->subgraph == sub)
		{
			act->rep_num = 0;
			act->rep_denom = 1;
			act->max_fire_count = 0;
		}
		act = (context_t *)objects_get_next(set, act);
	}
	return 0;
}

static int	_context_sort_isolated_actors(context_t *act, void *ptrset)
{
	void	*port, *ctx;
	int		ret;
	
	if (act->visited != 0)
	{
		fprintf(stderr, "error: _context_sort_isolated_actors(): Subgraph has a loop\n");
		fprintf(stderr, "\t[actor: %s]\n", act->name);
		return -1;
	}
	if (act->sorted == 0)
	{
		act->visited = -1;
		if (act->inputs == NULL)
		{
			fprintf(stderr, "error: _context_sort_isolated_actors(): Input port set is NULL\n");
			fprintf(stderr, "\t[actor: %s]\n", act->name);
			return -1;
		}
		port = objects_get_first(act->inputs);
		while (port != NULL)
		{
			ret = data_is_input_connected_to_var(port);
			if (ret == -1)
			{
				fprintf(stderr, "error: _context_sort_isolated_actors(): In checking signal type connected to input\n");
				fprintf(stderr, "\t[actor: %s]\n", act->name);
				return -1;
			}
			if (ret == 1)
			{
				ctx = NULL;
				ret = data_get_actor_connected_to_input_through_var(port, &ctx);
				if (ret != 0)
				{
					fprintf(stderr, "error: _context_sort_isolated_actors(): In getting actor connected to input\n");
					fprintf(stderr, "\t[actor: %s] [port: %s]\n", act->name, data_get_name(port));
					return -1;
				}
				if (ctx == NULL)
				{
					fprintf(stderr, "error: _context_sort_isolated_actors(): Actor connected to input is NULL\n");
					fprintf(stderr, "\t[actor: %s] [port: %s]\n", act->name, data_get_name(port));
					return -1;
				}
				ret = _context_sort_isolated_actors((context_t *) ctx, ptrset);
				if (ret != 0)
				{
					fprintf(stderr, "error: _context_sort_isolated_actors(): In recursive sorting subgraph\n");
					fprintf(stderr, "\t[actor: %s] [next actor: %s]\n", act->name, ((context_t *) ctx)->name);
					return -1;
				}
			}
			port = objects_get_next(act->inputs, port);
		}
		act->visited = 0;
		act->sorted = -1;
		if (pointer_new(ptrset, act) == NULL)
		{
			fprintf(stderr, "error: _context_sort_isolated_actors(): Can not create pointer object\n");
			fprintf(stderr, "\t[actor: %s]\n", act->name);
			return -1;
		}
	}
	return 0;
}

