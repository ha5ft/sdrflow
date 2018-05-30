/*******************************************************************************
 * 							Common context
 * *****************************************************************************
 * 	Filename:		context.h
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

#ifndef	CONTEXT_H
#define	CONTEXT_H

#include	"words.h"

#define		CONTEXT_TYPE_UNDEF			0
#define		CONTEXT_TYPE_PRIMITIVE		1
#define		CONTEXT_TYPE_COMPOSITE			2
#define		CONTEXT_TYPE_PRIM_INSTANCE	3
#define		CONTEXT_TYPE_COMP_INSTANCE	4
#define		CONTEXT_TOP_COMP_INSTANCE	5

#define		CONTEXT_LOAD_NO_NAME		0
#define		CONTEXT_LOAD_NAME			-1

#ifdef	_CONTEXT_

typedef struct	_context
{
	char	name[WORD_NAME_LENGTH+1];
	void	*inputs;
	void	*outputs;
	void	*params;
	void	*state;
	void	*temp;
	int		type;
	int		visited;
	int		subgraph;
	int		sorted;
	int		fire_count;
	int		max_fire_count;
	int		rep_num;
	int		rep_denom;
	void	*root;
} context_t;

#endif

int		context_initialize(void);
void	*context_new_set(void);
void	*context_new(void *set);
void	*context_make_instance(void *ctx, void *instance_set, char *name, void *hash);
int		context_delete(void *set, void *ctx);
int		context_delete_set(void *set);
int		context_cleanup(void);
int		context_add_location(int type, char *path);
int 	context_set_name(void *ctx, char *name);

char	*context_get_name(void *ctx);
int		context_get_subgraph(void *ctx);
int		context_get_rep_num(void *ctx);
int		context_get_type(void *ctx);
void	context_set_type(void *ctx, int type);
int		context_set_vector_count(void *ctx);
int		contextset_set_vector_count(void *set, int sub);
void	*context_new_actor_token(void *set, void *ctx);
int		context_get_max_fire_count(void *ctx);
void	*context_find(void *set, char *name);
void	contextset_clear_visited(void *set);
void	contextset_clear_fire_count(void *set);
void	contextset_clear_max_fire_count(void *set);
void	contextset_clear_subgraph(void *set);
void	context_increment_max_fire_count(void *ctx, int increment);
int		context_set_repetition(void *ctx, int num, int denom);
int		context_gcd(int a, int b);

int		context_check_ports_readers(void *ctx);
int		context_check_ports_writer(void *ctx);
int		context_check_ports_connection(void *ctx);
int		context_check_ports_parent(void *ctx);
int		contextset_check_ports_connection(void *set);
int		contextset_check_ports_parent(void *set);
int 	contextset_check_schedule_execution(void *set, int sub);
int		contextset_check_max_fire_count(void *set, int sub);
int		contextset_check_vector_count(void *set, int sub);

void	*context_parse(void *ctx_set, char *name);
int		context_parse_details(void *ctx, void *inp, void *scan, void *hash);
//int		context_load_hash(void *ctx, void *hash, int flag);

void	context_print_set(void *set, char *prefix, void *out);
void	context_print_set_connections(void *set, char *prefix, void *out);
void	context_print_connections(void *ctx, char *prefix, void *out);
void	context_print(void *ctx, char *prefix, void *out);
void 	context_print_details(void *ctx, char *prefix, void *out);
void 	contextset_print_repetition(void *set, int sub);

int		contextset_compute_repetition(void *set, int sub);
int		contextset_set_repetition(void *set, int sub, int rep);
int 	contextset_init_ports_cur_count(void *set, int sub);
void	*contextset_compute_fireing(void *set, int sub);
void	*contextset_compute_isolated_fireing(void *set, int sub);
int		context_mark_connected(void *act, int sub);
int		contextset_mark_subgraphs(void *set);
int 	contextset_mark_isolated_actors(void *set);
int 	context_fire(void *ctx);

int		context_set_generate_asm_actor_name_table(void *set, void *out);
int		context_generate_asm_port_name_table(void *ctx, void *out);
int 	context_set_generate_asm_load_script(void *set, void *out);
int 	context_set_generate_asm_make_script(void *set, void *out);
int		context_set_generate_asm_actor_port_init(void *set, void *out);
int		context_set_generate_asm_actor_init_call(void *set, void *out);
int		context_set_generate_asm_actor_port_update(void *set, void *out);
int		context_generate_asm_actor_fire_call(void *ctx, void *out);
int		context_set_generate_asm_actor_clean_call(void *ctx, void *out);
int		context_set_generate_asm_actor_instances(void *set, void *out);
int		context_generate_asm_actor_port_update(void *ctx, void *out);
int		context_generate_asm_port_pointers(void *ctx, void *out);

void 	context_generate_c_header(void *ctx, void *out);

#endif
