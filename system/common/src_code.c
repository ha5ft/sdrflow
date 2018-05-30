/*******************************************************************************
 * 							Common src_code
 * *****************************************************************************
 * 	Filename:		src_code.c
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
#include	"objects.h"
#include	"words.h"
#include	"hash_table.h"
#include	"scanner.h"
#include	"io.h"
#include	"context.h"
#include	"data.h" 
#include	"pointer.h"
#include	"src_lang.h"
#include	"common.h"

#define		_SRC_CODE_

#include	"src_code.h"

//#define		SRC_CODE_ALLOC_SIZE	4
#define		SRC_CODE_ERR_STR_LENGTH	255

#define		SRC_CODE_STATE_CREATED	0
#define		SRC_CODE_STATE_PARSED		1
#define		SRC_CODE_STATE_ERROR		2

static unsigned int	src_code_type_index = (unsigned int) OBJECTS_TYPE_SRC_CODE;
static int	src_code_initialized = 0;
static void	*src_code_set = NULL;
static char	*src_code_error_description[] =
{
	[0] = "No error",
	[1] = "Scanner error",
	[2] = "Only use, implementation or EOL is allowed here",
	[3] = "Only identifier is allowed here",
	[4] = "Identifier is not defined",
	[5] = "Only EOL is allowed here",
	[6] = "Can not parse context",
	[7] = "Identifier is not a composite name",
	[8] = "Only signals or EOL is allowed here",
	[9] = "Can not create signal object",
	[10] = "Can not parse signal definition",
	[11] = "Only stream, var, const, end or EOL is allowed here",
	[12] = "Only actors is allowed here",
	[13] = "Only primitive, composite, end or EOL is allowed here",
	[14] = "Only primitive name is allowed here",
	[15] = "Can not create primitive instance",
	[16] = "Only composite's name is allowed here",
	[17] = "Can not create composite instance",
	[18] = "Only topology or EOL is allowed here",
	[19] = "Only context or EOL is allowed here",
	[20] = "Can not create context object",
	[21] = "Can not scan composite's implementation context",
	[22] = "Only actor port name, end or EOL is allowed here",
	[23] = "Port already connected",
	[24] = "Only in or deleyed in operator is allowed here",
	[25] = "Only stream, var, const, input port or param port name allowed here",
	[26] = "Only out operator is allowed here",
	[27] = "Only stream, var or output port name allowed here",
	[28] = "Only in operator is allowed here",
	[29] = "Only var, const, or param port name allowed here",
	[30] = "Only inout operator is allowed here",
	[31] = "Only var name allowed here",
	[32] = "Can not create readers list",
	[33] = "Can not put a new reader into the readers list",
	[34] = "Signal already has a driver",
	[35] = "Signal and port has different data type",
	[36] = "Signal and port hash different vector size",
	[37] = "signal and port has different data count",
	[38] = "State and temp port can not share signal",
	[39] = "Error in checking if all ports are connected",
	[40] = "Error in checking signal readers",
	[41] = "Error in checking signal writers",
	[42] = "Delay must be positive",
	[43] = "Delay allowed only with streams",
	[44] = "Only schedule or EOL is allowed here",
	[45] = "Loop has no corresponding do",
	[46] = "Empty schedule is not allowed",
	[47] = "Only do, actor name, loop or end is allowed here",
	[48] = "Only literal is allowed here",
	[49] = "Only integer type is allowed here",
	[50] = "Cycle count should be positive",
	[51] = "Illegal state",
	[52] = "Delayed input is allowed only with steams",
	[53] = "Only manual or auto is allowed here",
	[54] = "Only end is allowed here",
	[55] = "Can not generate auto schedule",
	[56] = "Can not set streams and ports vector count",
	[57] = "Failed checking the topology",
	[58] = "Can not execut the manual schedule",
	[59] = "Failed marking the isolated actors",
	[60] = "Failed marking the subgraphs",
	[61] = "Only actor name is allowed here",
	[62] = "Invalid subgraph",
	[63] = "Checking vector count failed",
	[64] = "Failed initializing cur_count",
	[65] = "Checking max_fire_count failed",
	[66] = "Failed computing the repetition vector",
	[67] = "Failed computing the fireing sequence",
	[68] = "Failed generating the looped schedule",
	[69] = "Failed setting isolated actor's repetition count",
	[70] = "Failed computing isolated actor's fireing sequence",
	[71] = "Failed generating raw schedule",
	[72] = "Failed parsing the topology"
};

static int	_src_code_check_schedule_execution(src_code_t *src, int sub);

static int 	_src_code_generate_loop_schedule(src_code_t *src, void *ptrset);
static int	_src_code_find_loops(src_code_t *src, void **a, int N);
static int	_src_code_generate_raw_schedule(src_code_t *src, void *ptrset);
static void	_src_code_print_schedule(void *sched, char *prefix, void *out);

/* *****************************************************************************
 * 							Public functions
 * ****************************************************************************/

// *************************** Managing objects ********************************

int		src_code_initialize(void)
{
	if(src_code_initialized != 0) return -1;
	if (objects_is_initialized() == 0)
	{
		if (objects_init() != 0) return -1;
	}							
//	if ((src_code_type_index = objects_register_type(sizeof(src_code_t), SRC_CODE_ALLOC_SIZE)) < 0) return -1;
	src_code_type_index = (unsigned int) OBJECTS_TYPE_SRC_CODE;
	if ((src_code_set = objects_new_set(src_code_type_index)) == NULL)
	{
		objects_delete_type(src_code_type_index);
//		src_code_type_index = -1;
		return -1;
	}
	src_code_initialized = -1;
	return 0;	
}

void	*src_code_new(char *name, void *inp, int buffer_type)
{

	src_code_t	*src;
	
	if(src_code_initialized == 0) return NULL;
	if (name == NULL) return NULL;
	if (inp == NULL) return NULL;
	if (io_state(inp) != IO_CHAN_OPEN_RD) return NULL;
	if ((buffer_type != COMMON_BUFFER_TYPE_STATIC) && (buffer_type != COMMON_BUFFER_TYPE_STATIC))
		return NULL;
	src = (src_code_t *) objects_new(src_code_set, sizeof(src_code_t));
	if (src != NULL)
	{
		src->name[0] = 0;
		
		src->components = NULL;
		src->implementation = NULL;
		src->context = NULL;
		src->streams = NULL;
		src->variables = NULL;
		src->constants = NULL;
		src->schedule = NULL;
		src->inp = NULL;

		if ((src->components = context_new_set()) == NULL)
			{ src_code_delete(src); return NULL;}
		if ((src->implementation = context_new_set()) == NULL)
			{ src_code_delete(src); return NULL;}
		src->context = NULL;
		if ((src->streams = data_new_set()) == NULL)
			{ src_code_delete(src); return NULL;}
		if ((src->variables = data_new_set()) == NULL)
			{ src_code_delete(src); return NULL;}
		if ((src->constants = data_new_set()) == NULL)
			{ src_code_delete(src); return NULL;}
		if ((src->actors = context_new_set()) == NULL)
			{ src_code_delete(src); return NULL;}
		if ((src->schedule = words_new_set()) == NULL)
			{ src_code_delete(src); return NULL;}
//		if ((src->inp = io_new_channel(IO_CHAN_TYPE_FILE,"./",name,IO_ITEM_TYPE_SRC_CODE)) == NULL)
//			{ src_code_delete(src); return NULL;}
		src->inp = inp;
		src->buffer_type = buffer_type;	
		src->subgraph_count = 0;
		src->isolated_count = 0;
		src->depth = 0;
		src->schedule_depth = 0;
		src->initialized_for_fireing = 0;
		src->state = SRC_CODE_STATE_CREATED;
	}
	return src;
}

int		src_code_delete(void *src)
{
	if(src_code_initialized == 0) return -1;
	if (src == NULL) return -1;
	if (objects_verify_object(src_code_set, src) != 0) return -1;
	context_delete_set(((src_code_t *)src)->components);
	context_delete_set(((src_code_t *)src)->implementation);
	context_delete_set(((src_code_t *)src)->actors);
	data_delete_set(((src_code_t *)src)->streams);		
	data_delete_set(((src_code_t *)src)->variables);		
	data_delete_set(((src_code_t *)src)->constants);
	words_delete_set(((src_code_t *)src)->schedule);
//	io_delete_channel(((src_code_t *)src)->inp);
	return objects_delete(src_code_set, src);	
}

int		src_code_cleanup(void)
{
	src_code_t	*src;
	
	if(src_code_initialized == 0) return -1;
	src = (src_code_t *) objects_get_first(src_code_set);
	while (src != NULL)
	{
		//fprintf(stderr,"Deleting the first src_code\n");
		context_delete_set(((src_code_t *)src)->components);
		//fprintf(stderr,"Components set deleted\n");
		context_delete_set(((src_code_t *)src)->implementation);
		//fprintf(stderr,"Implementation set deleted\n");
		context_delete_set(((src_code_t *)src)->actors);
		//fprintf(stderr,"Actors set deleted\n");
		data_delete_set(((src_code_t *)src)->streams);
		//fprintf(stderr,"Sreams set deleted\n");		
		data_delete_set(((src_code_t *)src)->variables);
		//fprintf(stderr,"Variables set deleted\n");
		data_delete_set(((src_code_t *)src)->constants);
		//fprintf(stderr,"Constants set deleted\n");
		words_delete_set(((src_code_t *)src)->schedule);
		//fprintf(stderr,"Schedule set deleted\n");
//		io_delete_channel(((src_code_t *)src)->inp);
		//fprintf(stderr,"io channel deleted\n");
		src = (src_code_t *) objects_get_next(src_code_set, (void *) src);
	}
	//fprintf(stderr,"Deleting src_code type\n");
	objects_delete_type(src_code_type_index);
//	src_code_type_index = -1;
	src_code_initialized = 0;
	return 0;	
}

// *************************** Parsing the source code *************************


int	src_code_parse(void *src)
{
	src_code_t	*m = (src_code_t *) src;
	
	void		*scan, *hash;
	void		*ctx = NULL, *dat = NULL, *ctx_inst = NULL, *ptrset = NULL;
	int			state = 0;
	int			stop = 0;
	word_t		*w = NULL, *w_ctx_name = NULL, *last_do = NULL;
	sdferr_t	*scan_err = NULL;
	int			cur_type;
	int			cur_cycle_count = 1;
	int			cur_depth = 0;
	int			schedule_item_count = 0;
	int			cur_subgraph = 0;
	int			errnr = 0;
	int			lineno = 0;
	char		inp_name[IO_NAME_LENGTH + 1];
	char		inp_line[SCANNER_LINE_LENGTH + 1];

	if (objects_verify_object(src_code_set, src) != 0)
	{
		fprintf(stderr, "error: src_code_parse(): Invalid src_code object\n");
		return -1;
	}

	if (m->state != SRC_CODE_STATE_CREATED)
	{
		fprintf(stderr, "error: src_code_parse(): Bad state\n");
		return -1;
	}
//	if (io_open_for_read(m->inp) != 0)
//	{
//		fprintf(stderr, "error: src_code_parse(): Can not open source code\n");
//		return -1;
//	}
	if ((hash = hash_table_new()) == NULL)
	{
		fprintf(stderr, "error: src_code_parse(): Can not create hash table\n");
		hash_table_delete(hash);
		return -1;
	}
	if(src_lang_init_hash_table(hash) != 0)
	{
		fprintf(stderr, "error: src_code_parse(): Can not initialize hash table\n");
		hash_table_delete(hash);
		return -1;
	}
	if ((scan = scanner_new()) == NULL)
	{
		fprintf(stderr, "error: src_code_parse(): Can not create scanner\n");
		hash_table_delete(hash);
		return -1;
	}
	while (stop == 0)
	{
		if ((w = scanner_scan(scan, m->inp, hash)) == NULL)
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
					stop = -1;
					errnr = 1;
					break;
			}
		}
		else
		{
			cur_type = w->type;
		}
		if (errnr == 0)
		{
			switch (state)
			{
				case	0: //STATE_TOP_LEVEL
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							state = 0;
							break;
						case	WORD_TYPE_USE:
							state = 1;
							break;
						case	WORD_TYPE_COMPOSITE:
							state = 3;
							break;
						default:
							errnr = 2;
							stop = -1;
							break;
					}
					break;
					
// *********************** Parsing used component's context ********************

				case	1: //STATE_USE_IDENT
					if ((cur_type != WORD_TYPE_LABEL)) {errnr = 3;	stop = -1; break;}
					if (w->valid != 0) { errnr = 4; stop = -1; break;}
					w_ctx_name = w; // Storing the name for the next state
					state = 2;
					break;
				case	2: //STATE_USE_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					if ((ctx = context_parse(m->components, w_ctx_name->name)) == NULL)
						{errnr = 6; stop = -1; break;}
					if (context_get_type(ctx) == CONTEXT_TYPE_PRIMITIVE)
						w_ctx_name->type = WORD_TYPE_PRIM_NAME;
					else
						w_ctx_name->type = WORD_TYPE_COMPOSITE_NAME;
					w_ctx_name->value_type = VALUE_TYPE_PTR;
					w_ctx_name->value.p = ctx;
					w_ctx_name->valid = -1;
					state = 0;
					break;
					
// ************************** Parsing implementation ***************************

				case	3: //STATE_COMPOSITE_IDENT
					if ((cur_type != WORD_TYPE_LABEL)) {errnr = 7; stop = -1; break;}
//					fprintf(stderr, "parsing component name %s\n", w->name);
					strcpy(m->name, w->name);
					w->type = WORD_TYPE_COMP_IMP_NAME;
					ctx = context_new(m->implementation);
					if (ctx == NULL) {stop = -1; errnr = 20; break;}
					context_set_type(ctx, CONTEXT_TYPE_COMPOSITE);
					if (context_set_name(ctx, w->name) != 0) {stop = -1; errnr = 20; break;}
					w->value_type = VALUE_TYPE_PTR;
					w->value.p = ctx;
					w->valid = -1;
					m->context = ctx;
//					fprintf(stderr, "src_code_parse() : context name=%s\n", ((context_t *)ctx)->name);
					state = 4;
					break;
				case	4: //STATE_COMPOSITE_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					state = 5;
					break;
				case	5: //STATE_CONTEXT
					if (cur_type == WORD_TYPE_EOL) {break;}
					if (cur_type != WORD_TYPE_CONTEXT) { stop = -1; errnr = 19; break;}
					scanner_push_back_word(scan, w);
					if (context_parse_details(ctx, m->inp, scan, hash) != 0)
						{errnr = 21; stop = -1; break;}
//					fprintf(stderr, "src_code_parse() : context name=%s\n", ((context_t *)ctx)->name);
					state = 6;
					break;
				
// ************************** Parsing signal definitions ***********************

				case	6: //STATE_SIGNALS
					if (cur_type == WORD_TYPE_EOL) {break;}
					if (cur_type == WORD_TYPE_SIGNALS) { state = 7; break;}
					errnr = 8;
					stop = -1;
					break;
				case	7: //STATE_SIGNALS_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					state = 8;
					break;
				case	8: //STATE_SIGNAL_DEF
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_STREAM:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(m->streams)) == NULL)
								{stop = -1; errnr = 9;	break;}
							if (data_parse(dat, m->inp, scan, hash) != 0)
								{stop = -1; errnr = 10; break;}
							state = 8;
							break;
						case	WORD_TYPE_VAR:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(m->variables)) == NULL)
								{stop = -1; errnr = 9;	break;}
							if (data_parse(dat, m->inp, scan, hash) != 0)
								{stop = -1; errnr = 10; break;}
							state = 8;
							break;
						case	WORD_TYPE_CONST:
							scanner_push_back_word(scan, w);
							if ((dat = data_new(m->constants)) == NULL)
								{stop = -1; errnr = 9;	break;}
							if (data_parse(dat, m->inp, scan, hash) != 0)
								{stop = -1; errnr = 10; break;}
							state = 8;
							break;
						case	WORD_TYPE_END:
							state = 9;
							break;
						default:
							stop = -1;
							errnr = 11;
					}
					break;
				case	9: //STATE_SIGNAL_END_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					state = 10;
					break;
					
// ******************************* Parsing actors definitions ******************

				case	10:
					if (cur_type == WORD_TYPE_EOL) break;
					if (cur_type == WORD_TYPE_ACTORS) { state = 11; break;}
					errnr = 12;
					stop = -1;
					break;
				case	11:
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					state = 12;
					break;
				case	12:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_PRIMITIVE:
							state = 13;
							break;
						case	WORD_TYPE_COMPOSITE:
							state = 15;
							break;
						case	WORD_TYPE_END:
							state = 17;
							break;
						default:
							errnr = 13;
							stop = -1;
							break;
					}
					break;
				case	13:
					if (cur_type != WORD_TYPE_PRIM_NAME) {errnr = 14; stop = -1; break;}
					ctx = w->value.p; // Storing the context for the next state
					state = 14;
					break;
				case	14:
					if ((cur_type != WORD_TYPE_LABEL)) {errnr = 3;	stop = -1; break;}
					if (w->valid != 0) { errnr = 4; stop = -1; break;}
					ctx_inst = context_make_instance(ctx, m->actors, w->name, hash);
					if (ctx_inst == NULL) {errnr = 15; stop = -1; break;}
					w->type = WORD_TYPE_PRIM_ACTOR_NAME;
					w->value_type = VALUE_TYPE_PTR;
					w->value.p = ctx_inst;
					state = 11;
					break;
				case	15:
					if (cur_type != WORD_TYPE_COMPOSITE_NAME) {errnr = 16; stop = -1; break;}
					ctx = w->value.p; // Storing the context for the next state
					state = 16;
					break;
				case	16:
					if ((cur_type != WORD_TYPE_LABEL)) {errnr = 3;	stop = -1; break;}
					if (w->valid != 0) { errnr = 4; stop = -1; break;}
					ctx_inst = context_make_instance(ctx, m->actors, w->name, hash);
					if (ctx_inst == NULL) {errnr = 17; stop = -1; break;}
					w->type = WORD_TYPE_COMP_ACTOR_NAME;
					w->value_type = VALUE_TYPE_PTR;
					w->value.p = ctx_inst;
					state = 11;
					break;
				case	17:
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					state = 18;
					break;

// ************************ Parsing topology description **********************

				case	18://STATE_TOPOLOGY
					if (cur_type == WORD_TYPE_EOL) break;
					if (cur_type != WORD_TYPE_TOPOLOGY)
						{ stop = -1; errnr = 18; break;}
						
					scanner_push_back_word(scan, w);
					if (data_parse_topology(m->inp, scan, hash) != 0)
						{ stop = -1; errnr = 72; break;}
						
					if (contextset_check_ports_connection(m->actors) != 0)
						{stop = -1; errnr = 57; break;}
					if (contextset_check_ports_parent(m->actors) != 0)
						{stop = -1; errnr = 57; break;}
					if (context_check_ports_connection(m->context) != 0)
						{stop = -1; errnr = 57; break;}
					if (context_check_ports_parent(m->context) != 0)
						{stop = -1; errnr = 57; break;}
					if (dataset_check_readers(m->streams) != 0)
						{stop = -1; errnr = 57; break;}
					if (dataset_check_readers(m->variables) != 0)
						{stop = -1; errnr = 57; break;}
					if (dataset_check_readers(m->constants) != 0)
						{stop = -1; errnr = 57; break;}
					if (context_check_ports_readers(m->context) != 0)
						{stop = -1; errnr = 57; break;}
					if (dataset_check_writer(m->streams) != 0)
						{stop = -1; errnr = 57; break;}
					if (dataset_check_writer(m->variables) != 0)
						{stop = -1; errnr = 57; break;}
					if (context_check_ports_writer(m->context) != 0)
						{stop = -1; errnr = 57; break;}
					
					contextset_clear_subgraph(m->actors);
					contextset_clear_fire_count(m->actors);
					contextset_clear_max_fire_count(m->actors);
					cur_subgraph = 0;
					if ((m->isolated_count = contextset_mark_isolated_actors(m->actors)) == -1)
						{stop = -1; errnr = 59; break;}
					if ((m->subgraph_count = contextset_mark_subgraphs(m->actors)) == -1)
						{stop = -1; errnr = 60; break;}
					state = 31;
					break;

// ***************************** Parsing the schedule **************************

				case	31: //STATE_SCHEDULE
					if (cur_type == WORD_TYPE_EOL) {break;}
					if (cur_type != WORD_TYPE_SCHEDULE) {stop = -1; errnr = 44; break;}
					state = 32;
					break;
				case	32://STATE_SHEDULE_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					state = 33;
					break;
				case	33://STATE_SCHED_TYPE
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_MANUAL:
							state = 44;
							break;
						case	WORD_TYPE_AUTO:
							state = 45;
							break;
						case	WORD_TYPE_END:
							state = 41;
							break;
						default:
							errnr = 53;
							stop = -1;
							break;
					}
					break;

// *************************** Manual schedule *********************************

				case	44://STATE_MANUAL_SCHED_SEED
					if ((cur_type != WORD_TYPE_PRIM_ACTOR_NAME) && (cur_type != WORD_TYPE_COMP_ACTOR_NAME))
						{stop = -1; errnr = 61; break;}
					cur_subgraph = context_get_subgraph(w->value.p);
					if (cur_subgraph == 0) {stop = -1; errnr = 62; break;}
					state = 34;
					break;
				case	34://STATE_SCHED_TYPE_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					schedule_item_count = 0;
					m->schedule_depth = 0;
					cur_cycle_count = 1;
					cur_depth = 0;
					last_do = NULL;
					state = 35;
					break;
				case	35://STATE_MANUAL_SCHED_ITEM
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							break;
						case	WORD_TYPE_DO:
							w = words_dup_word(m->schedule, w);
							w->value_type = VALUE_TYPE_PTR;
							w->value.p = last_do;
							w->size = 0; // cycle count
							w->comp_type = lineno;
							w->valid = 0; // cycle count of the outer loop
							last_do = w;
							cur_depth++;
							if (cur_depth > m->schedule_depth)
							{
								m->schedule_depth = cur_depth;
							}
							schedule_item_count++;
							state = 36;
							break;
						case	WORD_TYPE_PRIM_ACTOR_NAME:
						case	WORD_TYPE_COMP_ACTOR_NAME:
							w = words_dup_word(m->schedule, w);
							w->comp_type = lineno;
							ctx = w->value.p;
							context_increment_max_fire_count(ctx, cur_cycle_count);
							schedule_item_count++;
							state = 37;
							break;
						case	WORD_TYPE_LOOP:
							if (cur_depth == 0) {stop = -1; errnr = 45; break;}
							w = words_dup_word(m->schedule, w);
							cur_depth--;
							cur_cycle_count /= last_do->size;
							w->comp_type = lineno;
							w->value.p = last_do;
							w->value_type = VALUE_TYPE_PTR;
							last_do = (word_t *)last_do->value.p;
							schedule_item_count++;
							state = 37;
							break;
						case	WORD_TYPE_END:
							if (schedule_item_count == 0)
								{stop = -1; errnr = 46; break;}
							state = 38;
							break;
						default:
							stop = -1;
							errnr = 47;
							break;
					}
					break;
				case	36://STATE_MANUAL_SCHED_LOOP_COUNT
					if (cur_type != WORD_TYPE_LITERAL)
						{stop = -1; errnr = 48; break;}
					if (w->value_type != VALUE_TYPE_LONG)
						{stop = -1; errnr = 49; break;}
					last_do->size = (int) w->value.l;
					if (last_do->size <= 0)
						{stop = -1; errnr = 50; break;}
					cur_cycle_count *= last_do->size;
					state = 37;
					break;
				case	37://STATE_MANUAL_SCHED_ITEM_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					state = 35;
					break;

// **************************** Checking the manual schedule *******************

				case	38://STATE_MANUAL_SCHED_END_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					if (contextset_check_max_fire_count(m->actors, cur_subgraph) != 0)
						{stop = -1; errnr = 65; break;}
					if (contextset_set_vector_count(m->actors,cur_subgraph) != 0)
						{errnr = 56; stop = -1; break;}
					if (contextset_check_vector_count(m->actors,cur_subgraph) != 0)
						{errnr = 63; stop = -1; break;}
					contextset_clear_fire_count(m->actors);
					m->schedule_depth = 0;
					m->initialized_for_fireing = -1;
					if (contextset_init_ports_cur_count(m->actors, cur_subgraph) != 0)
						{stop = -1; errnr = 64; break;}
					if (_src_code_check_schedule_execution(m, cur_subgraph) != 0)
						{stop = -1; errnr = 58; break;}
					state = 33;
					break;

// *************************** Automatic schedule ******************************

				case	45://STATE_AUTO_SCHED_SEED
					if ((cur_type != WORD_TYPE_PRIM_ACTOR_NAME) && (cur_type != WORD_TYPE_COMP_ACTOR_NAME))
						{stop = -1; errnr = 61; break;}
					cur_subgraph = context_get_subgraph(w->value.p);
					if (cur_subgraph == 0) {stop = -1; errnr = 62; break;}
					state = 39;
					break;
				case	39://STATE_AUTO_SCHED_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					if (cur_subgraph > 0)
					{
						if (contextset_compute_repetition(m->actors, cur_subgraph) != 0)
						{ stop = -1; errnr = 66; break;}
						if ((ptrset = contextset_compute_fireing(m->actors, cur_subgraph)) == NULL)
						{ stop = -1; errnr = 67; break; }
						if (_src_code_generate_loop_schedule(m, ptrset) != 0)
						{ stop = -1; errnr = 68; break; }
						pointer_delete_set(ptrset);
					}
					else
					{
						if (contextset_set_repetition(m->actors, cur_subgraph, 1) != 0)
						{ stop = -1; errnr = 69; break;}
						if ((ptrset = contextset_compute_isolated_fireing(m->actors, cur_subgraph)) == NULL)
						{ stop = -1; errnr = 70; break;}
						if (_src_code_generate_raw_schedule(m, ptrset) != 0)
						{ stop = -1; errnr = 71; break; }
						pointer_delete_set(ptrset);
					}
					if (contextset_check_max_fire_count(m->actors, cur_subgraph) != 0)
						{stop = -1; errnr = 65; break;}
					if (contextset_set_vector_count(m->actors,cur_subgraph) != 0)
						{errnr = 56; stop = -1; break;}
					if (contextset_check_vector_count(m->actors,cur_subgraph) != 0)
						{errnr = 63; stop = -1; break;}
					state = 33;
					break;
				case	40://STATE_SCHEDULE_END -- not used anymore
					if (cur_type == WORD_TYPE_EOL) break;
					if (cur_type != WORD_TYPE_END) {errnr = 54; stop = -1; break;}
					state = 41;
					break;
				case	41://STATE_SCHEDULE_END_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					if (contextset_check_max_fire_count(m->actors, 0) != 0)
						{stop = -1; errnr = 65; break;}
					if (contextset_check_vector_count(m->actors,0) != 0)
						{errnr = 63; stop = -1; break;}
					state = 42;
					break;

// *********************** Module definition end *******************************

				case	42://STATE_COMPOSITE_END
					if (cur_type == WORD_TYPE_EOL) break;
					if (cur_type != WORD_TYPE_END) {errnr = 54; stop = -1; break;}
					state = 43;
					break;
				case	43://STATE_COMPOSITE_END_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 5; stop = -1; break;}
					stop = -1;
					break;
				default:
					stop = -1;
					errnr = 51;
					break;
			}
		}
	}
	scanner_delete(scan);
	hash_table_delete(hash);
	if (errnr == 0)
	{
//		io_close(m->inp);
		m->state = SRC_CODE_STATE_PARSED;
		return 0;
	}
	else
	{
		if (io_get_name(m->inp,inp_name, IO_NAME_LENGTH + 1) != 0)
			inp_name[0] = 0;
		if (scanner_get_line(scan, inp_line, SCANNER_LINE_LENGTH + 1) != 0)
			inp_line[0] = 0;
		fprintf(stderr, "error: src_code_parse(): Parsing error\n");
		fprintf(stderr, "[input:%s] [line: %d] [%s]\n", inp_name, lineno, inp_line);
		fprintf(stderr, "[description: %s]\n", src_code_error_description[errnr]);
//		io_close(m->inp);
		m->state = SRC_CODE_STATE_ERROR;
		return -1;
	}
}

// ******************************* Generating context *************************

int	src_code_generate_context(void *src_code, void *out)
{
	src_code_t	*src = (src_code_t *) src_code;

	if(src_code_initialized == 0)
	{
		fprintf(stderr, "error: src_code_generate_context(): scr_code class not initialized\n");
		return -1;
	}
	if (objects_verify_object(src_code_set, src) != 0)
	{
		fprintf(stderr, "error: src_code_generate_context(): Invalid src_code object\n");
		return -1;
	}
	if (out == NULL)
	{
		fprintf(stderr, "error: src_code_generate_context(): Invalid output channel\n");
		return -1;
	}
	if (io_state(out) != IO_CHAN_OPEN_WR)
	{
		fprintf(stderr, "error: src_code_generate_context(): Invalid channel state\n");
		return -1;
	}
	if (src->state != SRC_CODE_STATE_PARSED)
	{
		fprintf(stderr, "error: src_code_generate_context(): Code has not parsed yet\n");
		return -1;
	}
	
	context_print(src->context, "", out);
	return 0;
}

// ******************************* Generating asm code *************************

int	src_code_generate_asm(void *src_code, void *out)
{
	src_code_t	*src = (src_code_t *) src_code;
	word_t			*w;
	int				l = 0;
	char			line[SCANNER_LINE_LENGTH + 1];

	if(src_code_initialized == 0)
	{
		fprintf(stderr, "error: src_code_generate_asm(): scr_code class not initialized\n");
		return -1;
	}
	if (objects_verify_object(src_code_set, src) != 0)
	{
		fprintf(stderr, "error: src_code_generate_asm(): Invalid src_code object\n");
		return -1;
	}
	if (out == NULL)
	{
		fprintf(stderr, "error: src_code_generate_asm(): Invalid output channel\n");
		return -1;
	}
	if (io_state(out) != IO_CHAN_OPEN_WR)
	{
		fprintf(stderr, "error: src_code_generate_asm(): Invalid channel state\n");
		return -1;
	}
	if (src->state != SRC_CODE_STATE_PARSED)
	{
		fprintf(stderr, "error: src_code_generate_asm(): Code has not parsed yet\n");
		return -1;
	}
//
// ******************* Generating meta segment ********************************
//	
	if (io_put(out,src_lang_get_asm_line(L_META_SEG)) != 0) return -1;
	sprintf(line, src_lang_get_asm_line(L_COMP_NAME),src->name);
	if (io_put(out,line) != 0) return -1;
	sprintf(line, src_lang_get_asm_line(L_COMP_VER),1);
	if (io_put(out,line) != 0) return -1;

// ================== Generating name tables ==================================
	
	if (context_set_generate_asm_actor_name_table(src->components, out) != 0) return -1;
	if (context_set_generate_asm_actor_name_table(src->actors, out) != 0) return -1;
	if (data_set_generate_asm_name_table(src->streams, out) != 0) return -1;
	if (data_set_generate_asm_name_table(src->variables, out) != 0) return -1;
	if (data_set_generate_asm_name_table(src->constants, out) != 0) return -1;
	if (context_generate_asm_port_name_table(src->context, out) != 0) return -1;

// ================== Generating load script ==================================

	if (io_put(out,src_lang_get_asm_line(L_LOAD_LAB)) != 0) return -1;
//	if (context_set_generate_asm_load_script(src->actors, out) != 0) return -1;
	if (context_set_generate_asm_load_script(src->components, out) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_META_EXIT)) != 0) return -1;

// ================== Generating make script ==================================

	if (io_put(out,src_lang_get_asm_line(L_MAKE_LAB)) != 0) return -1;
	if (context_set_generate_asm_make_script(src->actors, out) != 0) return -1;
	if (data_set_generate_asm_make_script(src->streams, out) != 0) return -1;
	if (data_set_generate_asm_make_script(src->variables, out) != 0) return -1;
	if (data_set_generate_asm_make_script(src->constants, out) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_META_EXIT)) != 0) return -1;

// ================== Generating delete script ==================================

	if (io_put(out,src_lang_get_asm_line(L_DEL_LAB)) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_META_EXIT)) != 0) return -1;

// ================ Generating segment end ====================================
	
	if (io_put(out,src_lang_get_asm_line(L_ENDSEG)) != 0) return -1;
//
// ******************* Generating context segment *****************************
//	
	if (io_put(out,src_lang_get_asm_line(L_CONTEXT_SEG)) != 0) return -1;
	if (context_generate_asm_port_pointers(src->context, out) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_ENDSEG)) != 0) return -1;	
//
// ******************* Generating code segment ********************************
//	
	if (io_put(out,src_lang_get_asm_line(L_CODE_SEG)) != 0) return -1;

	if (io_put(out,src_lang_get_asm_line(L_EXIT)) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_INIT_LAB)) != 0) return -1;
	if (context_set_generate_asm_actor_port_init(src->actors, out) != 0) return -1;
	if (context_set_generate_asm_actor_init_call(src->actors, out) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_RET)) != 0) return -1;
	
	if (io_put(out,src_lang_get_asm_line(L_END_CYCLE)) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_FIRE_LAB)) != 0) return -1;
	if (context_set_generate_asm_actor_port_init(src->actors, out) != 0) return -1;
	w = (word_t *) objects_get_first(src->schedule);
	while (w != NULL)
	{
		switch (w->type)
		{
			case	WORD_TYPE_DO:
				l++;
				sprintf(line, src_lang_get_asm_line(L_DO),w->size);
				if (io_put(out,line) != 0) return -1;
				sprintf(line, src_lang_get_asm_line(L_LOOP_LAB),l);
				if (io_put(out,line) != 0) return -1;
				w->valid = l;
				break;
			case	WORD_TYPE_PRIM_ACTOR_NAME:
			case	WORD_TYPE_COMP_ACTOR_NAME:
				if (context_generate_asm_actor_fire_call(w->value.p, out) != 0) return -1;
				if (context_generate_asm_actor_port_update(w->value.p, out) != 0) return -1;
				break;
			case	WORD_TYPE_LOOP:
				sprintf(line, src_lang_get_asm_line(L_LOOP),((word_t *)(w->value.p))->valid);
				if (io_put(out,line) != 0) return -1;
				break;
			default:
				return -1;
		}
		w = (word_t *) objects_get_next(src->schedule, w);
	}
	if (io_put(out,src_lang_get_asm_line(L_RET)) != 0) return -1;

	if (io_put(out,src_lang_get_asm_line(L_EXIT)) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_CLEAN_LAB)) != 0) return -1;
	if (context_set_generate_asm_actor_port_init(src->actors, out) != 0) return -1;
	if (context_set_generate_asm_actor_clean_call(src->actors, out) != 0) return -1;
	if (io_put(out,src_lang_get_asm_line(L_RET)) != 0) return -1;

// ================ Generating segment end ====================================

	if (io_put(out,src_lang_get_asm_line(L_ENDSEG)) != 0) return -1;
//
// **************** Generating the data segment	*******************************
//
	if (io_put(out,src_lang_get_asm_line(L_DATA_SEG)) != 0) return -1;

// ================ Generating the pointer table ==============================

	if (data_set_generate_asm_signal_pointer(src->streams, out) != 0) return -1;
	if (data_set_generate_asm_signal_pointer(src->variables, out) != 0) return -1;
	if (data_set_generate_asm_signal_pointer(src->constants, out) != 0) return -1;

// ================ Generating the signals ====================================

	if (data_set_generate_asm_signals(src->streams, out) != 0) return -1;
	if (data_set_generate_asm_signals(src->variables, out) != 0) return -1;
	if (data_set_generate_asm_signals(src->constants, out) != 0) return -1;

// ================ Generating the actor instances ============================

	if (context_set_generate_asm_actor_instances(src->actors, out) != 0) return -1;

// ================ Generating segment end ====================================

	if (io_put(out,src_lang_get_asm_line(L_ENDSEG)) != 0) return -1;
//
// **************** The end of the asm code generation ************************
//
	return 0;
}

// ****************************** Listing the source code **********************

int	src_code_print(void *src, void *out)
{
	src_code_t	*s = (src_code_t *) src;
	char		line[SCANNER_LINE_LENGTH + 1];
	
	if(src_code_initialized == 0)
	{
		fprintf(stderr, "error: src_code_print(): scr_code class not initialized\n");
		return -1;
	}
	if (objects_verify_object(src_code_set, src) != 0)
	{
		fprintf(stderr, "error: src_code_print(): Invalid src_code object\n");
		return -1;
	}
	if (out == NULL)
	{
		fprintf(stderr, "error: src_code_print(): Invalid output channel\n");
		return -1;
	}
	if (io_state(out) != IO_CHAN_OPEN_WR)
	{
		fprintf(stderr, "error: src_code_print(): Invalid channel state\n");
		return -1;
	}
	if (s->state != SRC_CODE_STATE_PARSED)
	{
		fprintf(stderr, "error: src_code_print(): Code has not parsed yet\n");
		return -1;
	}
	
	context_print_set((void *)s->components, "", out);
	
	sprintf(line, "composite\t%s",s->name);
	if (io_put(out,line) != 0) return -1;
	
	context_print_details(s->context, "\t", out);
	
	sprintf(line, "\tsignals");
	if (io_put(out,line) != 0) return -1;
	data_print_set(s->streams, "\t\t", out);
	data_print_set(s->variables, "\t\t", out);
	data_print_set(s->constants, "\t\t", out);
	sprintf(line, "\tend"); // signals
	if (io_put(out,line) != 0) return -1;
	
	sprintf(line, "\tactors");
	if (io_put(out,line) != 0) return -1;
	context_print_set(s->actors, "\t\t", out);
	sprintf(line, "\tend"); // actors
	if (io_put(out,line) != 0) return -1;

	sprintf(line, "\ttopology");
	if (io_put(out,line) != 0) return -1;
	context_print_set_connections(s->actors, "\t\t", out);	
	sprintf(line, "\tend"); // topology
	if (io_put(out,line) != 0) return -1;
	
	sprintf(line, "\tschedule");
	if (io_put(out,line) != 0) return -1;
	_src_code_print_schedule(s->schedule, "\t\t", out);
	sprintf(line, "\tend");
	if (io_put(out,line) != 0) return -1;
	
	sprintf(line, "end"); // implementation
	if (io_put(out,line) != 0) return -1;
	return 0;
}

/* *****************************************************************************
 * 							Private functions
 * ****************************************************************************/

// ************************** Generating schedule ******************************

static int _src_code_generate_raw_schedule(src_code_t *src, void *ptrset)
{
	void		**a;
	int			N, i;
	word_t		*w;
	
	if (src == NULL) return -1;
	if (ptrset == NULL) return -1;
	N = objects_get_set_size(ptrset);
	if (N == 0) return 0;
	a = pointer_get_raw_pointers(ptrset);
	if (a == NULL) return -1;
	for (i = 0; i < N; i++)
	{
		w = (word_t *) context_new_actor_token(src->schedule, a[i]);
		if (w == NULL)
		{
			free(a);
			return -1;
		}
	}
	free(a);
	return 0;
}

static int _src_code_generate_loop_schedule(src_code_t *src, void *ptrset)
{
	void	 	**a;
	int			N;
	
	if (src == NULL) return -1;
	src->depth = 0;
	if (ptrset == NULL) return -1;
	N = objects_get_set_size(ptrset);
	if (N <= 0) return -1;
	a = (void **) pointer_get_raw_pointers(ptrset);
	if (a == NULL) return -1;
	if (_src_code_find_loops(src, &a[0], N) != 0)
	{
		free(a);
		return -1;
	}
	free(a);
	return 0;
}

static int _src_code_find_loops(src_code_t *src, void **a, int N)
{
	int 		M, i, j, k, r, M0, k0, r0;
	word_t		*w/*, *last_do*/;
	
	if (N == 0) return -1;
//	last_do = NULL;
	M = N/2;
	M0 = k0 = 0;
	r0 = 1;
	while (M > 0)
	{
		k = 0;
		while ((k + (2 * M)) <= N)
		{
			j = k;
			i = k;
			r = 1;
			while ((j + (2 * M)) <= N)
			{
				for (i = j; i < j + M; i++)
				{
					if (a[i] != a[i + M]) break;
				}
				if (i < (j + M)) break;
				r++;
				j += M;
			}
			if ((r > 1) && (((r0 * M0) < (r * M)) || (((r0 * M0) == (r * M)) && (M < M0))))
			{
				r0 = r;
				k0 = k;
				M0 = M;
			}
			k = i + 1;
		}
		M--;
	}
	if (r0 == 1)
	{
		for (i = 0; i < N; i++)
		{
			w = (word_t *) context_new_actor_token(src->schedule, a[i]);
			if (w == NULL) return -1;
		}
	}
	else
	{
		if (k0 > 0)
		{
			if (_src_code_find_loops(src, &a[0], k0) != 0) return -1;
		}
		w = src_lang_new_do_token(src->schedule, r0);
		if (w == NULL) return -1;
		src->depth++;
		if (src->depth > src->schedule_depth)
		{
			src->schedule_depth = src->depth;
		}
		if (_src_code_find_loops(src,&a[k0], M0) != 0) return -1;
		src->depth--;
		w = src_lang_new_loop_token(src->schedule, w);
		if (w == NULL) return -1;
		if ((k0 + (r0 * M0)) < N)
		{
			if (_src_code_find_loops(src, &a[k0 + (r0 * M0)], N - (k0 + (r0 * M0))) != 0) return -1;
		}
	}
	return 0;
}

// ********************** Checking the source code **********************************

static int	_src_code_check_schedule_execution(src_code_t *src, int sub)
{
	word_t		*w;
	void		*act;
	int			cycle_count, depth;

	if (src == NULL)
	{
		fprintf(stderr,"error: _src_code_check_schedule_execution() : This pointer is NULL\n");
		return -1;
	}
	if (src->initialized_for_fireing == 0)
	{
		fprintf(stderr,"error: _src_code_check_schedule_execution() : Not initialized for fireing\n");
		return -1;
	}
	src->initialized_for_fireing = 0;
	if (src->actors == NULL)
	{
		fprintf(stderr,"error: _src_code_check_schedule_execution() : Actor set is NULL\n");
		return -1;
	}
	if (src->streams == NULL)
	{
		fprintf(stderr,"error: _src_code_check_schedule_execution() : Stream set is NULL\n");
		return -1;
	}
	if (src->schedule == NULL)
	{
		fprintf(stderr,"error: _src_code_check_schedule_execution() : Schedule item set is NULL\n");
		return -1;
	}
	
	cycle_count = 0;
	depth = 0;
	w = (word_t *) objects_get_first(src->schedule);
	while (w != NULL)
	{
		switch (w->type)
		{
			case	WORD_TYPE_DO:
				w->valid = cycle_count;
				cycle_count = w->size;
				if (cycle_count <= 0)
				{
					fprintf(stderr,"error: _src_code_check_schedule_execution(): Invalid loop count\n");
					fprintf(stderr, "\t[line: %d] [loop count %d]\n", w->comp_type, cycle_count);
					return -1;
				}
				depth++;
				break;
			case	WORD_TYPE_PRIM_ACTOR_NAME:
			case	WORD_TYPE_COMP_ACTOR_NAME:
				act = w->value.p;
				if (act == NULL)
				{
					fprintf(stderr,"error: _src_code_check_schedule_execution() : Invalid actor\n");
					fprintf(stderr, "\t[line: %d] [cycle count: %d]\n",w->comp_type, cycle_count);
					return -1;
				}
				if (context_fire(act) != 0)
				{
					fprintf(stderr,"error: _src_code_check_schedule_execution(): Actor is not fireable\n");
					fprintf(stderr, "\t[line: %d] [cycle count: %d]\n",w->comp_type, cycle_count);
				}
				break;
			case	WORD_TYPE_LOOP:
				if (depth <= 0)
				{
					fprintf(stderr,"error: _src_code_check_schedule_execution(): Loop without corresponding do\n");
					fprintf(stderr, "\t[line: %d]", w->comp_type);
					return -1;
				}
				if (w->value.p == NULL)
				{
					fprintf(stderr,"error: _src_code_check_schedule_execution(): Loop has invalid reference to do\n");
					fprintf(stderr, "\t[line: %d]", w->comp_type);
					return -1;
				}
				if (cycle_count <= 1)
				{
					cycle_count = ((word_t *) w->value.p)->valid;
					depth--;
				}
				else
				{
					w = (word_t *) w->value.p;
					cycle_count--;
				}
				break;
			default:
				fprintf(stderr, "error: _src_code_check_schedule_execution(): Invalid token in the schedule");
				fprintf(stderr, "\t[line: %d]", w->comp_type);
				return -1;
				break;
		}
		w = (word_t *) objects_get_next(src->schedule, w);
	}
	if (contextset_check_schedule_execution(src->actors, sub) != 0)
	{
		fprintf(stderr, "error: _src_code_check_schedule_execution(): Invalid schedule execution\n");
		return -1;
	}
	return 0;
}

// *********************** Listing functions ***********************************

void	_src_code_print_schedule(void *sched, char *prefix, void *out)
{
	word_t	*w;
	char	line[SCANNER_LINE_LENGTH + 1];
	
	
	if (sched == NULL) return;
	w = objects_get_first(sched);
	while (w != NULL)
	{
		if (w->type == WORD_TYPE_DO)
		{
			sprintf(line, "%s%s\t%d", prefix, w->name, w->size);
			if (io_put(out,line) != 0) return;
		}
		else
		{
			sprintf(line, "%s%s", prefix, w->name);
			if (io_put(out,line) != 0) return;
		}
		w = objects_get_next(sched, w);
	}
}

