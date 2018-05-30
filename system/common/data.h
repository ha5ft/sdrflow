/*******************************************************************************
 * 							Common data
 * *****************************************************************************
 * 	Filename:		data.h
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

#ifndef	DATA_H
#define	DATA_H

#include	"words.h"

#define	DATA_TYPE_UNDEF		0
#define	DATA_TYPE_STREAM	1
#define	DATA_TYPE_VAR		2
#define	DATA_TYPE_CONST		3
#define	DATA_TYPE_INPUT		4
#define	DATA_TYPE_OUTPUT	5
#define	DATA_TYPE_PARAM		6
#define	DATA_TYPE_STATE		7
#define	DATA_TYPE_TEMP		8

#ifdef	_DATA_

typedef struct	_data
{
	char	name[WORD_NAME_LENGTH + 1];
	int		type;
	int		vector_size;
	int		vector_count;
	int		delay;
	int		value_type;
	int		value_count;
	void	*values;
	void	*connection;
	void	*readers;
	int		not_for_scheduling;
	int		connected_to_signal;
        int     not_for_update;
//	int		visited;
	int		cur_count;
	void	*parent;
} data_t;

#endif

int		data_initialize(void);
void	*data_new_set(void);
void	*data_clone_set(void *set, void *parent, char *prefix, void *hash);
void	*data_new(void *set);
char	*data_get_name(void *dat);
void	data_set_parent(void *dat, void *ctx);
int		data_get_vector_count(void *dat);
void	*data_get_connection(void *dat);

int		data_set_generate_asm_name_table(void *set, void *out);
int		data_set_generate_asm_make_script(void *set, void *out);
int		data_set_generate_asm_actor_port_init(void *set, void *out);
int		data_set_generate_asm_actor_port_update(void *set, void *out);
int		data_set_generate_asm_signal_pointer(void *set, void *out);
int		data_set_generate_asm_signals(void *set, void *out);
int		data_set_generate_asm_port_pointer(void *set, void *out);

int 	dataset_init_cur_count(void *set);
int		dataset_is_input_fireable(void *set);
int		dataset_check_input_fireability(void *set);
int 	dataset_increment_cur_count(void *set);
int		dataset_set_vector_count(void *set);
int 	dataset_mark_connected(void *set, int sub);
int 	dataset_is_not_for_scheduling(void *set);
int 	dataset_is_not_connected_to_signal(void *set);
int		data_get_actor_connected_to_input_through_var(void *dat, void **ctx);
int		data_is_input_connected_to_var(void *dat);
int		dataset_is_output_not_connected_in_sub(void *set, int sub);
int		dataset_set_output_repetition(void *set, int num, int denom);
int		dataset_set_input_repetition(void *set, int num, int denom);

int		dataset_check_connection(void *set);
int		dataset_check_parent(void *set);
int		dataset_check_readers(void *set);
int		dataset_check_writer(void *set);
int		dataset_check_vector_count(void *dat);

void	data_set_generate_c_header(void *set, void *out);
void	data_generate_c_header(void *dat, void *out);

void	data_print_set(void *set, char *prefix, void *out);
void	data_print_set_connections(void *set,char *prefix, void *out);
void	data_print(void *dat, char *prefix, void *out);
void	data_print_connection(void *dat, char *prefix, void *out);

int		data_set_load_hash(void *set, void *hash);
int		data_parse(void *dat, void *inp, void *scan, void *hash);
int		data_parse_topology(void *inp, void *scan, void *hash);

int		data_delete(void *set, void *d);
int		data_delete_set(void *set);
int		data_cleanup(void);

#endif
