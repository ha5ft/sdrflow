/*******************************************************************************
 * 							Common data
 * *****************************************************************************
 * 	Filename:		data.c
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

#define	_DATA_

#include	"objects.h"
#include	"words.h"
#include	"hash_table.h"
#include	"scanner.h"
#include	"io.h"
#include	"data.h"
#include	"src_lang.h"
#include	"value.h"
#include	"allocators.h"
#include	"pointer.h"
#include	"context.h"

//#define		DATA_ALLOC_SIZE		128

static unsigned int	data_type_index = (unsigned int) OBJECTS_TYPE_DATA;
static int	data_initialized = 0;

static char	*data_error_description[] =
{
	[0] = "No error",	
	[1] = "Unexpected EOF",	
	[2] = "Scanner error",	
	[3] = "Only signal or port type is allowed here",	
	[4] = "ptr, string or addr allocator is not allowed here",	
	[5] = "Only allocator is allowed here",	
	[6] = "Array size is 0",	
	[7] = "Identifier already defined",	
	[8] = "Can not get identifier's name",	
	[9] = "Only array size or identifier is allowed here",	
	[10] = "Only identifier is allowed here",	
	[11] = "Only const could have default values",	
	[12] = "Can not create default value set",	
	[13] = "Can not duplicate default value token",	
	[14] = "Only array size, default value or EOL is allowed here",
	[15] = "Only default value or EOL is allowed here",	
	[16] = "Only EOL is allowed here",
	[17] = "Undefined vector count is not allowed here",
	[18] = "Only actor port name, end or EOL is allowed here",
	[19] = "Port already connected",
	[20] = "Only in or deleyed in operator is allowed here",
	[21] = "Only stream, var, const, input port or param port name allowed here",
	[22] = "Only out operator is allowed here",
	[23] = "Only stream, var or output port name allowed here",
	[24] = "Only in operator is allowed here",
	[25] = "Only var, const, or param port name allowed here",
	[26] = "Only inout operator is allowed here",
	[27] = "Only var name allowed here",
	[28] = "Can not create readers list",
	[29] = "Can not put a new reader into the readers list",
	[30] = "Signal already has a driver",
	[31] = "Signal and port has different data type",
	[32] = "Signal and port hash different vector size",
	[33] = "signal and port has different data count",
	[34] = "State and temp port can not share signal",
	[35] = "Delay must be positive",
	[36] = "Delay allowed only with streams",
	[37] = "Delayed input is allowed only with steams",
	[38] = "Only topology or EOL is allowed here",
	[39] = "Failed checking the topology"
};


static	int	word_types[9] =
{
	[DATA_TYPE_UNDEF] = WORD_TYPE_UNDEF_NAME,
	[DATA_TYPE_STREAM] = WORD_TYPE_STREAM_NAME,
	[DATA_TYPE_VAR] = WORD_TYPE_VAR_NAME,
	[DATA_TYPE_CONST] = WORD_TYPE_CONST_NAME,
	[DATA_TYPE_INPUT] = WORD_TYPE_ACT_INP_NAME,
	[DATA_TYPE_OUTPUT] = WORD_TYPE_ACT_OUT_NAME,
	[DATA_TYPE_PARAM] = WORD_TYPE_ACT_PAR_NAME,
	[DATA_TYPE_STATE] = WORD_TYPE_ACT_STA_NAME,
	[DATA_TYPE_TEMP] = WORD_TYPE_ACT_TMP_NAME
};

static char	*data_c_typenames[ALLOC_COUNT] =
{
	[NULL_ALLOC] = "",
	[CHAR_ALLOC] = "char",
	[UCHAR_ALLOC] = "unsigned char",
	[SHORT_ALLOC] = "short",
	[USHORT_ALLOC] = "unsigned short",
	[INT_ALLOC] = "int",
	[UINT_ALLOC] = "unsigned int",
	[LONG_ALLOC] = "long",
	[ULONG_ALLOC] = "unsigned long",
	[FLOAT_ALLOC] = "float",
	[DOUBLE_ALLOC] = "double",
	[PTR_ALLOC] = "(void *)",
	[STRING_ALLOC] = "(char *)",
	[ADDR_ALLOC] = "int",
	[OFFSET_ALLOC] = "int",
	[CHARARR_ALLOC] = ""
};

static data_t	*_data_clone(data_t *d, void *clone_set, void *parent, char *prefix, void *hash);

// ***************************** Public functions ******************************

int		data_initialize(void)
{
	if(data_initialized != 0) return -1;
	if (objects_is_initialized() == 0)
	{
		if (objects_init() != 0) return -1;
	}
//	if ((data_type_index = objects_register_type(sizeof(data_t), DATA_ALLOC_SIZE)) < 0) return -1;
	data_type_index = (unsigned int) OBJECTS_TYPE_DATA;
	data_initialized = -1;
	return 0;	
}

void	*data_new_set(void)
{
	if(data_initialized == 0) return NULL;
	return objects_new_set(data_type_index);			
}

void	*data_clone_set(void *set, void *parent, char *prefix, void *hash)
{
	void	*clone_set = NULL;
	void	*dat;
	
	if(data_initialized == 0) return NULL;
	if ((clone_set = objects_new_set(data_type_index)) == NULL) return NULL;
	dat = objects_get_first(set);
	while (dat)
	{
		if (_data_clone(dat, clone_set, parent, prefix, hash) == NULL)
		{
			data_delete_set(clone_set);
			return NULL;
		}
		dat = objects_get_next(set, dat);
	}
	return clone_set;
}

void	*data_new(void *set)
{
	data_t	*d;
	
	if(data_initialized == 0) return NULL;
	d = (data_t *) objects_new(set, sizeof(data_t));
	if (d != NULL)
	{
		d->name[0] = 0;
		d->type = DATA_TYPE_UNDEF;
		d->vector_size = 1;
		d->vector_count = 0;
		d->delay = 0;
		d->value_type = VALUE_TYPE_NULL;
		d->value_count = 0;
		d->values = NULL;
		d->connection = NULL;
		d->readers = NULL;
		d->not_for_scheduling = 0;
		d->connected_to_signal = 0;
		d->not_for_update = 0;
//		d->visited = 0;
		d->cur_count = 0;
		d->parent = NULL;
	}
	return d;	
}

char	*data_get_name(void *dat)
{
	if (dat == NULL) return "";
	return ((data_t *) dat)->name;
}

void	data_set_parent(void *dat, void *ctx)
{
	if (dat == NULL) return;
	((data_t *) dat)->parent = ctx;
	return;
}

int	data_get_vector_count(void *dat)
{
	if (dat == 0)
	{
		fprintf(stderr, "error: data_get_vector_count(): Signal or port is NULL\n");
		return 0;
	}
	return ((data_t *) dat)->vector_count;
}

void	*data_get_connection(void *dat)
{
	if (dat == NULL)
	{
		fprintf(stderr, "error: data_get_connection(): Signal or port is NULL\n");
		return NULL;
	}
	return ((data_t *) dat)->connection;
}

int	data_get_type(void *dat)
{
	if (dat == 0)
	{
		fprintf(stderr, "error: data_get_type(): Signal or port is NULL\n");
		return 0;
	}
	return ((data_t *) dat)->type;
}

// ********************* Load signal and port names into the hash table ********

int		data_set_load_hash(void *set, void *hash)
{
	data_t	*d;
	word_t	*w;
	
	if(data_initialized == 0) return -1;
	if (hash == NULL) return -1;
	if (set == NULL) return -1;
	d = (data_t *) objects_get_first(set);
	while (d != NULL)
	{
		if ((w = hash_table_lookup(hash, d->name)) == NULL) return -1;
		if (w->type != WORD_TYPE_LABEL) return -1;
		if (w->valid != 0) return -1;
		w->type = word_types[d->type];
		w->value_type = VALUE_TYPE_PTR;
		w->value.p = d;
		w->valid = -1;
		d = objects_get_next(set, (void *) d);
	}
	return 0;
}

// ************************* Parsing signal and port definition *****************

int		data_parse(void *dat, void *inp, void *scan, void *hash)
{
	data_t		*d = (data_t *) dat;
	int			state = 0;
	int			stop = 0;
	word_t		*w = NULL;
	sdferr_t	*scan_err = NULL;
	int			cur_type;
	int			errnr= 0;
	int			lineno = 0;
	int			label_word_type = 0;
	int			val_count = 0;
	int			val_max_count = 1;
	
	if ((dat == NULL) || (inp == NULL) || (scan == NULL) || (hash == NULL))
		return -1;
	if (d->type != DATA_TYPE_UNDEF) return -1;
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
					stop = -1;
					errnr = 1;
					break;
				default:
					stop = -1;
					errnr = 2;
					break;
			}
		}
		else
		{
			cur_type = w->type;
		}
		lineno = scanner_lineno(scan);
		if (errnr == 0)
		{
			switch(state)
			{
				case	0:
					state = 1;
					switch (cur_type)
					{
						case	WORD_TYPE_INPUT:
							d->type = DATA_TYPE_INPUT;
							label_word_type = WORD_TYPE_INPUT_NAME;
							break;
						case	WORD_TYPE_OUTPUT:
							d->type = DATA_TYPE_OUTPUT;
							label_word_type = WORD_TYPE_OUTPUT_NAME;
							break;
						case	WORD_TYPE_PARAM:
							d->type = DATA_TYPE_PARAM;
							label_word_type = WORD_TYPE_PARAM_NAME;
							break;
						case	WORD_TYPE_STATE:
							d->type = DATA_TYPE_STATE;
							label_word_type = WORD_TYPE_STATE_NAME;
							break;
						case	WORD_TYPE_TEMP:
							d->type = DATA_TYPE_TEMP;
							label_word_type = WORD_TYPE_TEMP_NAME;
							break;
						case	WORD_TYPE_STREAM:
							d->type = DATA_TYPE_STREAM;
							label_word_type = WORD_TYPE_STREAM_NAME;
							break;
						case	WORD_TYPE_VAR:
							d->type = DATA_TYPE_VAR;
							label_word_type = WORD_TYPE_VAR_NAME;
							break;
						case	WORD_TYPE_CONST:
							d->type = DATA_TYPE_CONST;
							label_word_type = WORD_TYPE_CONST_NAME;
							break;
						default:
							stop = -1;
							errnr = 3;
							break;
					}
					break;
				case	1:
					switch (cur_type)
					{
						case	WORD_TYPE_ALLOCATOR:
							if ((w->value.l == PTR_ALLOC) ||\
								(w->value.l == STRING_ALLOC)||\
								(w->value.l == ADDR_ALLOC))
								{ stop = -1; errnr = 4; break;}
							d->value_type = w->value.l;
							//if ((d->type == DATA_TYPE_INPUT) || (d->type == DATA_TYPE_OUTPUT))
								state = 2;
							//else
								//state = 3;
							break;
						default:
							stop = -1;
							errnr = 5;
							break;
					}
					break;
				case	2:
					switch (cur_type)
					{
						case	WORD_TYPE_ARRAY_SIZE:
							if (w->value.l == 0) {stop = -1; errnr = 6; break;}
							d->vector_size = w->value.l;
							val_max_count = d->vector_size;
							state = 3;
							break;
						case	WORD_TYPE_LABEL:
							if (w->valid != 0) { errnr = 7; stop = -1; break;}
							if (words_get_name(w, &d->name[0], WORD_NAME_LENGTH + 1) != 0)
									{errnr = 8;	stop = -1; break;}
							w->type = label_word_type;
							w->value_type = VALUE_TYPE_PTR;
							w->value.p = dat;
							w->valid = -1;
							d->vector_size = 1;
							val_max_count = d->vector_size;
							state = 4;
							break;
						default:
							stop = -1;
							errnr = 9;
							break;
					}
					break;
				case	3:
					switch (cur_type)
					{
						case	WORD_TYPE_LABEL:
							if (w->valid != 0) { errnr = 7; stop = -1; break;}
							if (words_get_name(w, &d->name[0], WORD_NAME_LENGTH + 1) != 0)
									{errnr = 8;	stop = -1; break;}
							w->type = label_word_type;
							w->value_type = VALUE_TYPE_PTR;
							w->value.p = dat;
							w->valid = -1;
							state = 4;
							break;
						default:
							stop = -1;
							errnr = 10;
							break;
					}
					break;
				case	4:
					switch (cur_type)
					{
						case	WORD_TYPE_ARRAY_SIZE:
							//if (w->value.l == 0) {stop = -1; errnr = 6; break;}
							d->vector_count = w->value.l;
							val_max_count *= d->vector_count;
							if ((d->vector_count == 0) &&\
								(d->type != DATA_TYPE_INPUT) &&\
								(d->type != DATA_TYPE_OUTPUT) &&\
								(d->type != DATA_TYPE_STREAM))
							{
								stop = -1;
								errnr = 17;
								break;
							}
							if (d->type == DATA_TYPE_CONST)
								state = 5;
							else
								state = 6;
							break;
						case	WORD_TYPE_LITERAL:
							d->vector_count = 1;
							if (d->type != DATA_TYPE_CONST)
								{ stop = -1; errnr = 11; break;}
							if ((d->values = words_new_set()) == NULL)
								{ stop = -1; errnr = 12; break;}
							if (words_dup_word(d->values, w) == NULL)
								{ stop = -1; errnr = 13; break;}
							val_count++;
							d->value_count = val_count;
							if (val_count == val_max_count)
								state = 6;
							else
								state = 5;
							break;
						case	WORD_TYPE_EOL:
							d->vector_count = 1;
							stop = -1;
							break;
						default:
							stop = -1;
							errnr = 14;
							break;
					}
					break;
				case	5:
					switch (cur_type)
					{
						case	WORD_TYPE_LITERAL:
							if (d->values == NULL)
							{
								if ((d->values = words_new_set()) == NULL)
									{ stop = -1; errnr = 12; break;}
							}
							if (words_dup_word(d->values, w) == NULL)
								{ stop = -1; errnr = 13; break;}
							val_count++;
							d->value_count = val_count;
							if (val_count == val_max_count)
								state = 6;
							else
								state = 5;
							break;
						case	WORD_TYPE_EOL:
							stop = -1;
							break;
						default:
							stop = -1;
							errnr = 15;
							break;
					}
					break;
				case	6:
					switch (cur_type)
					{
						case	WORD_TYPE_EOL:
							stop = -1;
							break;
						default:
							stop = -1;
							errnr = 16;
							break;
					}
					break;				
			}
		}
	}
	if (errnr == 0)
		return 0;
	else
	{
		fprintf(stderr,"[line: %d] [error: %d] %s\n", lineno, errnr, data_error_description[errnr]);
		return -1;
	}
}


int		data_parse_topology(void *inp, void *scan, void *hash)
{
	data_t		*port = NULL, *sig = NULL;
	void		*ptr = NULL;
	int			state = 18;
	int			stop = 0;
	word_t		*w = NULL;
	sdferr_t		*scan_err = NULL;
	int			cur_type = 0;
	int			errnr= 0;
	int			lineno = 0;
	char		inp_name[IO_NAME_LENGTH + 1];
	char		inp_line[SCANNER_LINE_LENGTH + 1];
	
	if ((inp == NULL) || (scan == NULL) || (hash == NULL))
		return -1;
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
					stop = -1;
					errnr = 1;
					break;
				default:
					stop = -1;
					errnr = 2;
					break;
			}
		}
		else
		{
			cur_type = w->type;
		}
		lineno = scanner_lineno(scan);
		if (errnr == 0)
		{
			switch(state)
			{
				case	18://STATE_TOPOLOGY
					if (cur_type == WORD_TYPE_EOL) break;
					if (cur_type == WORD_TYPE_TOPOLOGY) { state = 19; break;}
					errnr = 38;
					stop = -1;
					break;
				case	19://STATE_TOPOLOGY_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 16; stop = -1; break;}
					state = 20;
					break;
				case	20://STATE_PORT_NAME
					switch (cur_type)
					{
						case	WORD_TYPE_ACT_INP_NAME:
							port = (data_t *) w->value.p;
							port->not_for_scheduling = 0;
							port->not_for_update = 0;
							state = 21;
							break;
						case	WORD_TYPE_ACT_OUT_NAME:
							port = (data_t *) w->value.p;
							port->not_for_scheduling = 0;
							port->not_for_update = 0;
							state = 23;
							break;
						case	WORD_TYPE_ACT_PAR_NAME:
							port = (data_t *) w->value.p;
							port->not_for_scheduling = 0;
							port->not_for_update = 0;
							state = 25;
							break;
						case	WORD_TYPE_ACT_STA_NAME:
							port = (data_t *) w->value.p;
							port->not_for_scheduling = 0;
							port->not_for_update = 0;
							state = 27;
							break;
						case	WORD_TYPE_ACT_TMP_NAME:
							port = (data_t *) w->value.p;
							port->not_for_scheduling = 0;
							port->not_for_update = 0;
							state = 27;
							break;
						case	WORD_TYPE_END:
							state = 30;
							break;
						case	WORD_TYPE_EOL:
							break;
						default:
							stop = -1;
							errnr = 18;
							break;
					}
					break;
				case	21://STATE_TOPOLOGY_INP_OP
					switch (cur_type)
					{
						case	WORD_TYPE_IN_OP:
							state = 22;
							break;
						case	WORD_TYPE_DELEYED_IN_OP:
							port->delay = (int) w->value.l;
							if (port->delay < 0)
								{stop = -1; errnr = 35; break;}
							state = 22;
							break;
						default:
							stop = -1;
							errnr = 20;
							break;
					}
					break;
				case	22://STATE_TOPOLOGY_INP_SIG
					switch (cur_type)
					{
						case	WORD_TYPE_VAR_NAME:
						case	WORD_TYPE_CONST_NAME:
						case	WORD_TYPE_PARAM_NAME:
							port->not_for_scheduling = -1;
							port->connected_to_signal = -1;
							port->not_for_update = -1;
							if (port->delay != 0)
								{stop = -1; errnr = 37; break;}
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						case	WORD_TYPE_STREAM_NAME:
							port->connected_to_signal = -1;
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						case	WORD_TYPE_INPUT_NAME:
							port->connected_to_signal = 0;
							port->not_for_scheduling = -1;
							port->not_for_update = 0;
							if (port->delay != 0)
								{stop = -1; errnr = 37; break;}
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						default:
							stop = -1;
							errnr = 21;
							break;
					}
					break;
				case	23://STATE_TOPOLOGY_OUT_OP
					switch (cur_type)
					{
						case	WORD_TYPE_OUT_OP:
							state = 24;
							break;
						default:
							stop = -1;
							errnr = 22;
							break;
					}
					break;
				case	24://STATE_TOPOLOGY_OUT_SIG
					switch (cur_type)
					{	
						case	WORD_TYPE_VAR_NAME:
							port->not_for_scheduling = -1;
							port->connected_to_signal = -1;
							port->not_for_update = -1;
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						case	WORD_TYPE_STREAM_NAME:
							port->connected_to_signal = -1;
							port->not_for_update = 0;
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						case	WORD_TYPE_OUTPUT_NAME:
							port->not_for_scheduling = -1;
							port->connected_to_signal = 0;
							port->not_for_update = 0;
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						default:
							stop = -1;
							errnr = 23;
							break;
					}
					break;
				case	25://STATE_TOPOLOGY_PAR_OP
					switch (cur_type)
					{
						case	WORD_TYPE_IN_OP:
							state = 26;
							break;
						default:
							stop = -1;
							errnr = 24;
							break;
					}
					break;
				case	26://STATE_TOPOLOGY_PAR_SIG
					switch (cur_type)
					{
						case	WORD_TYPE_VAR_NAME:
						case	WORD_TYPE_CONST_NAME:
							port->connected_to_signal = -1;
							port->not_for_scheduling = -1;
							port->not_for_update = -1;
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						case	WORD_TYPE_PARAM_NAME:
							port->connected_to_signal = 0;
							port->not_for_scheduling = -1;
							port->not_for_update = 1;
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						default:
							stop = -1;
							errnr = 25;
							break;
					}
					break;
				case	27://STATE_TOPOLOGY_STA_TMP_OP
					switch (cur_type)
					{
						case	WORD_TYPE_EQU_OP:
							state = 28;
							break;
						default:
							stop = -1;
							errnr = 26;
							break;							
					}
					break;
				case	28://STATE_TOPOLOGY_STA_TMP_SIG
					switch (cur_type)
					{
						case	WORD_TYPE_VAR_NAME:
							port->not_for_scheduling = -1;
							port->connected_to_signal = -1;
							port->not_for_update = -1;
							sig = (data_t *) w->value.p;
							state = 29;
							break;
						default:
							stop = -1;
							errnr = 27;
							break;
					}
					break;
// **************************** Checking the topology **************************
				case	29://STATE_TOPOLOGY_ITEM_EOL
					if (cur_type != WORD_TYPE_EOL) {stop = -1; errnr = 16; break;}
					
					if (port->connection != NULL) {stop = -1; errnr = 19; break;}
					port->connection = sig;
					
					if (port->type != DATA_TYPE_OUTPUT)
					{
						//fprintf(stderr,"%s readers = %ld\n", sig->name, (long)sig->readers);
						if (sig->readers == NULL)
						{
							sig->readers = pointer_new_set();
							if (sig->readers == NULL) {stop = -1; errnr = 28; break;}
							//fprintf(stderr,"new readers = %ld\n", (long)sig->readers);
						}
						else
						{
							if ((port->type == DATA_TYPE_STATE) || (port->type == DATA_TYPE_TEMP))
							{
								if (sig->readers != NULL) {stop = -1; errnr = 34; break;}
							}
						}
						ptr = pointer_new(sig->readers, port);
						if (ptr == NULL) {stop = -1; errnr = 29; break;}
					}
					
					if ((port->type == DATA_TYPE_OUTPUT) ||\
						(port->type == DATA_TYPE_STATE) ||\
						(port->type == DATA_TYPE_TEMP))
					{
						if (sig->connection != NULL) {stop = -1; errnr = 30; break;}
						sig->connection = port;
					}
					
					if (port->type == DATA_TYPE_INPUT)
					{
						if (port->delay != 0)
						{
							if (sig->type != DATA_TYPE_STREAM)
								{stop = -1; errnr = 36; break;}
							if (port->delay > sig->delay)
							{
								sig->delay = port->delay;
							}
						}
					}
					
					if (port->value_type != sig->value_type)
						{stop = -1; errnr = 31; break;}
					if (port->vector_size != sig->vector_size)
						{stop = -1; errnr = 32; break;}
					if ((sig->type == DATA_TYPE_VAR) || (sig->type == DATA_TYPE_CONST) || (sig->type == DATA_TYPE_PARAM))
					{
						if (port->vector_count != sig->vector_count)
							{stop = -1; errnr = 33; break;}
					}
					state = 20;
					break;
				case	30://STATE_TOPOLOGY_END_EOL
					if (cur_type != WORD_TYPE_EOL) {errnr = 16; stop = -1; break;}
					stop = -1;
					break;
			}
		}
	}
	if (errnr == 0)
		return 0;
	else
	{
		if (io_get_name(inp,inp_name, IO_NAME_LENGTH + 1) != 0)
			inp_name[0] = 0;
		if (scanner_get_line(scan, inp_line, SCANNER_LINE_LENGTH + 1) != 0)
			inp_line[0] = 0;
		lineno = scanner_lineno(scan);
		fprintf(stderr, "error: data_parse_topology(): Parsing error errnr = %d\n", errnr);
		fprintf(stderr, "[input: %s] [line: %d] [%s]\n", inp_name, lineno, inp_line);
		fprintf(stderr, "[description: %s]\n", data_error_description[errnr]);
		return -1;
	}
}

// ************************** Creating schedule ********************************

int	dataset_set_output_repetition(void *set, int num, int denom)
{
	data_t		*src, *snk, *sig;
	void		*rdr, *act;
	int			rnum, rdenom, g;

	src = objects_get_first(set);
	while (src != NULL)
	{
		sig = (data_t *)src->connection;
		if (sig->type == DATA_TYPE_STREAM)
		{
			rdr = objects_get_first(sig->readers);
			while (rdr != NULL)
			{
				snk = (data_t *) pointer_get_raw(rdr);
				act = snk->parent;
				if (context_get_rep_num(act) == 0)
				{
					rnum = src->vector_count * num;
					rdenom = snk->vector_count * denom;
					if ((g = context_gcd(rnum, rdenom)) == 0) return -1;;
					rnum /= g;
					rdenom /= g;
					if (context_set_repetition(act, rnum, rdenom) != 0) return -1;
				}
				rdr = objects_get_next(sig->readers, rdr);
			}
		}
		src = (data_t *) objects_get_next(set, src);
	}
	return 0;
}

int	dataset_set_input_repetition(void *set, int num, int denom)
{
	data_t		*snk, *sig, *src;
	void		*act;
	int			rnum, rdenom, g;

	snk = (data_t *) objects_get_first(set);
	while (snk != NULL)
	{
		sig = (data_t *)snk->connection;
		src = (data_t *)sig->connection;
		if (sig->type == DATA_TYPE_STREAM)
		{
			act = src->parent;
			if (context_get_rep_num(act) == 0)
			{
				rnum = snk->vector_count * num;
				rdenom = src->vector_count * denom;
				if ((g = context_gcd(rnum, rdenom)) == 0) return -1;;
				rnum /= g;
				rdenom /= g;
				if (context_set_repetition(act, rnum, rdenom) != 0) return -1;
			}
		}
		snk = (data_t *) objects_get_next(set, snk);
	}
	return 0;
}

int dataset_init_cur_count(void *set)
{
	data_t	*port, *sig;
	
	if(data_initialized == 0)
	{
		fprintf(stderr,"error: dataset_init_cur_count() : Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr,"error: dataset_init_cur_count() : Null pointer\n");
		return -1;
	}
	port = (data_t *)objects_get_first(set);
	while (port != NULL)
	{
		switch (port->type)
		{
			case	DATA_TYPE_INPUT:
				if (port->not_for_scheduling == 0)
				{
					sig = (data_t *) port->connection;
					if (sig == NULL) return -1;
					port->cur_count = sig->delay - port->delay;
					sig->cur_count = sig->delay;
				}
				break;
			case	DATA_TYPE_OUTPUT:
				if (port->not_for_scheduling == 0)
				{
					sig = (data_t *) port->connection;
					if (sig == NULL) return -1;
					port->cur_count = 0;
					sig->cur_count = sig->delay;
				}
				break;
			default:
				break;
		}
		port = objects_get_next(set, port);
	}
	return 0;
}

int	dataset_is_input_fireable(void *set)
{
	data_t	*port, *sig;
	
	if(data_initialized == 0)
	{
		fprintf(stderr,"error: dataset_is_input_fireable() : Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr,"error: dataset_is_input_fireable() : Null pointer\n");
		return -1;
	}
	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		if ((port->type == DATA_TYPE_INPUT) && (port->not_for_scheduling == 0))
		{
			sig = (data_t *) port->connection;
			if (sig == NULL) return -1;
			if (sig->cur_count < port->cur_count + port->vector_count)
				return 0;
		}
		port = (data_t *) objects_get_next(set, port);
	}
	return 1;
}

int	dataset_check_input_fireability(void *set)
{
	data_t	*port, *sig;
	
	if(data_initialized == 0)
	{
		fprintf(stderr,"error: dataset_check_input_fireability() : Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr,"error: dataset_check_input_fireability() : Null pointer\n");
		return -1;
	}
	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		if ((port->type == DATA_TYPE_INPUT) && (port->not_for_scheduling == 0))
		{
			sig = (data_t *) port->connection;
			if (sig == NULL) return -1;
			if (sig->cur_count < port->cur_count + port->vector_count)
			{
				fprintf(stderr,"error: dataset_check_input_fireability() : Not enough data for fireing\n");
				fprintf(stderr, "\t[port: %s[%d]] [data needed: %d] [stream: %s[%d]]\n",port->name,port->cur_count,port->vector_count,sig->name,sig->cur_count);
				return -1;
			}
		}
		port = (data_t *) objects_get_next(set, port);
	}
	return 0;
}

int dataset_increment_cur_count(void *set)
{
	data_t	*port, *sig;

	if(data_initialized == 0)
	{
		fprintf(stderr,"error: dataset_increment_cur_count() : Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr,"error: dataset_increment_cur_count() : Null pointer\n");
		return -1;
	}
	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		switch (port->type)
		{
			case	DATA_TYPE_INPUT:
				if (port->not_for_scheduling == 0)
				{
					port->cur_count += port->vector_count;
				}
				break;
			case	DATA_TYPE_OUTPUT:
				if (port->not_for_scheduling == 0)
				{
					sig = (data_t *) port->connection;
					if (sig == NULL) return -1;
					port->cur_count += port->vector_count;
					sig->cur_count += port->vector_count;
				}
				break;
			default:
				break;
		}
		port = (data_t *) objects_get_next(set, port);
	}
	return 0;
}

int	dataset_set_vector_count(void *set)
{
	data_t	*d, *sig;
	int		count;
	
	if (set == NULL) return -1;
	d = (data_t *) objects_get_first(set);
	while (d != NULL)
	{
		switch (d->type)
		{
			case	DATA_TYPE_INPUT:
				sig = (data_t *) d->connection;
				if (sig->type == DATA_TYPE_INPUT)
				{
					count = context_get_max_fire_count(d->parent);
					count *= d->vector_count;
					if (sig->vector_count == 0)
					{
						sig->vector_count = count;
					}
					else
					{
						if (sig->vector_count != count)
						{
							fprintf(stderr, "error: dataset_set_vector_count(): Vector count missmatch\n");
							fprintf(stderr, "\t[actor port: %s] [count: %d] [signal: %s] [count: %d]\n", d->name, count, sig->name, sig->vector_count);
							return -1;
						}
					}
				}
			
				break;
			case	DATA_TYPE_OUTPUT:
				sig = (data_t *) d->connection;
				if ((sig->type == DATA_TYPE_STREAM) || (sig->type == DATA_TYPE_OUTPUT))
				{
					count = context_get_max_fire_count(d->parent);
					count *= d->vector_count;
					sig->vector_count = count;
				}
				break;
			default:
				return -1;
				break;
		}
		d = (data_t *) objects_get_next(set, d);
	}
	return 0;
}

int dataset_mark_connected(void *set, int sub)
{
	data_t	*port, *writer_port, *reader_port, *sig;
	void	*ptr;
	void	*act;
	
	if (set == NULL)
	{
		fprintf(stderr, "error: dataset_mark_connected(): Port set is NULL\n");
		return -1;
	}
	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		if (port->not_for_scheduling == 0)
		{
			switch (port->type)
			{
				case	DATA_TYPE_INPUT:
					sig = (data_t *) port->connection;
					if (sig == NULL)
					{
						fprintf(stderr, "error: dataset_mark_connected(): Port is not connected\n");
						fprintf(stderr, "\t[port: %s]\n", port->name);
						return -1;
					}
					writer_port = (data_t *) sig->connection;
					if (writer_port == NULL)
					{
						fprintf(stderr, "error: dataset_mark_connected(): Stream is not connected\n");
						fprintf(stderr, "\t[stream: %s]\n", sig->name);
						return -1;
					}
					act = writer_port->parent;
					if (act == NULL)
					{
						fprintf(stderr, "error: dataset_mark_connected(): Port is not connected to an actor\n");
						fprintf(stderr, "\t[port: %s]\n", port->name);
						return -1;
					}
					if (context_mark_connected(act, sub) != 0)
					{
						fprintf(stderr, "error: dataset_mark_connected(): Error in visiting an actor\n");
						return -1;
					}
					break;
				case	DATA_TYPE_OUTPUT:
					sig = (data_t *) port->connection;
					if (sig == NULL)
					{
						fprintf(stderr, "error: dataset_mark_connected(): Port is not connected\n");
						fprintf(stderr, "\t[port: %s]\n", port->name);
						return -1;
					}
					if (sig->readers == NULL)
					{
						fprintf(stderr, "error: dataset_mark_connected(): Stream has no readers\n");
						fprintf(stderr, "\t[stream: %s]\n", sig->name);
						return -1;
					}
					ptr = objects_get_first(sig->readers);
					while (ptr != NULL)
					{
						reader_port = (data_t *) pointer_get_raw(ptr);
						if (reader_port == NULL)
						{
							fprintf(stderr, "error: dataset_mark_connected(): Stream's reader is NULL port\n");
							fprintf(stderr, "\t[stream: %s]\n", sig->name);
							return -1;
						}
						act = reader_port->parent;
						if (act == NULL)
						{
							fprintf(stderr, "error: dataset_mark_connected(): Port is not connected to an actor\n");
							fprintf(stderr, "\t[port: %s]\n", port->name);
							return -1;
						}
						if (context_mark_connected(act, sub) != 0)
						{
							fprintf(stderr, "error: dataset_mark_connected(): Error in visiting an actor\n");
							return -1;
						}
						ptr = objects_get_next(sig->readers, ptr);
					}
					break;
			}
		}
		port = (data_t *) objects_get_next(set, port);
	}
	return 0;
}

int dataset_is_not_for_scheduling(void *set)
{
	data_t	*port;
	
	if (set == NULL)
	{
		fprintf(stderr, "error: dataset_is_not_for_scheduling(): Port set is NULL\n");
		return -1;
	}
	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		if (port->not_for_scheduling == 0)
		{
			return 0;
		}
		port = (data_t *) objects_get_next(set, port);
	}
	return 1;
}

int dataset_is_not_connected_to_signal(void *set)
{
	data_t	*port;
	int		not_connected = 1;
	
	if (set == NULL)
	{
		fprintf(stderr, "error: dataset_is_not_connected_to_signal(): Port set is NULL\n");
		return -1;
	}
	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		if (port->connected_to_signal != 0)
		{
			not_connected = 0;
		}
		port = (data_t *) objects_get_next(set, port);
	}
	return not_connected;
}

int	data_is_input_connected_to_var(void *dat)
{
	data_t	*port, *sig;
	
	if (dat == NULL)
	{
		fprintf(stderr, "error: data_is_input_connected_to_var(): Port is NULL\n");
		return -1;
	}
	port = (data_t *) dat;
	if (port->type != DATA_TYPE_INPUT)
	{
		fprintf(stderr, "error: data_is_input_connected_to_var(): Port is not input\n");
		return -1;
	}
	if (port->connected_to_signal == 0) return 0;
	sig = (data_t *) port->connection;
	if (sig == NULL)
	{
		fprintf(stderr, "error: data_is_input_connected_to_var(): Connected signal is NULL\n");
		return -1;
	}
	if (sig->type != DATA_TYPE_VAR) return 0;
	return 1;
}

int	dataset_is_output_not_connected_in_sub(void *set, int sub)
{
	data_t	*port, *sig, *port1;
	void	*ctx, *ptr;
	int		not_connected = 1;
	
	if (set == NULL)
	{
		fprintf(stderr, "error: dataset_is_output_not_connected_in_sub(): Port set is NULL\n");
		return -1;
	}
	if (sub == 0)
	{
		fprintf(stderr, "error: dataset_is_output_not_connected_in_sub(): Subgraph is 0\n");
		return -1;
	}
	port = objects_get_first(set);
	while (port != NULL)
	{
		if (port->type != DATA_TYPE_OUTPUT)
		{
			fprintf(stderr, "error: dataset_is_output_not_connected_in_sub(): Not an output port\n");
			fprintf(stderr, "\t[port: %s]\n", port->name);
			return -1;
		}
		if (sub == -1)
		{
			if (port->connected_to_signal != 0)
			{
				sig = port->connection;
				if (sig == NULL)
				{
					fprintf(stderr, "error: dataset_is_output_not_connected_in_sub(): Port connected to NULL\n");
					fprintf(stderr, "\t[port: %s]\n", port->name);
					return -1;
				}
				if (sig->type == DATA_TYPE_VAR)
				{
					if (sig->readers == NULL)
					{
						fprintf(stderr, "error: dataset_is_output_not_connected_in_sub(): Readers set is NULL\n");
						fprintf(stderr, "\t[port: %s] [signal: %s]\n", port->name, sig->name);
						return -1;
					}
					ptr = objects_get_first(sig->readers);
					while (ptr != NULL)
					{
						port1 = pointer_get_raw(ptr);
						if (port1 == NULL)
						{
							fprintf(stderr, "error: dataset_is_output_not_connected_in_sub(): Reader port is NULL\n");
							fprintf(stderr, "\t[port: %s] [signal: %s]\n", port->name, sig->name);
							return -1;
						}
						ctx = port1->parent;
						if (ctx == NULL)
						{
							fprintf(stderr, "error: dataset_is_output_not_connected_in_sub(): Reader's port parent is NULL\n");
							fprintf(stderr, "\t[port: %s] [signal: %s] [reader port: %s]\n", port->name, sig->name, port1->name);
							return -1;
						}
						if (context_get_subgraph(ctx) == -1)
						{
							not_connected = 0;
						}
						ptr = objects_get_next(sig->readers, ptr);
					}
				}
			}
		}
		else
		{
			if (port->connected_to_signal)
			{
				sig = port->connection;
				if (sig == NULL)
				{
					fprintf(stderr, "error: dataset_is_output_not_connected_in_sub(): Port connected to NULL\n");
					fprintf(stderr, "\t[port: %s]\n", port->name);
					return -1;
				}
				if (sig->type == DATA_TYPE_STREAM)
				{
					not_connected = 0;
				}
			}
		}
		port = objects_get_next(set, port);
	}
	return not_connected;
}

int data_get_actor_connected_to_input_through_var(void *dat, void **ctx)
{
	data_t	*sig, *port, *port1;
	
	if (dat == NULL)
	{
		fprintf(stderr, "error: data_get_actor_connected_to_input_through_var(): Port is NULL\n");
		return -1;
	}
	port = (data_t *) dat;
	if (port->type != DATA_TYPE_INPUT)
	{
		fprintf(stderr, "error: data_get_actor_connected_to_input_through_var(): Port is not an input\n");
		fprintf(stderr, "\t[port: %s]\n", port->name);
		return -1;
	}
	if (port->connected_to_signal == 0)
	{
		return 0;
	}
	sig = ((data_t *) dat)->connection;
	if (sig == NULL)
	{
		fprintf(stderr, "error: data_get_actor_connected_to_input_through_var(): Connected signal is NULL\n");
		fprintf(stderr, "\t[port: %s]\n", port->name);
		return -1;
	}
	if (sig->type == DATA_TYPE_VAR)
	{
		port1 = (data_t *) sig->connection;
		if (port1 == NULL)
		{
			fprintf(stderr, "error: data_get_actor_connected_to_input_through_var(): Connected signal is NULL\n");
			fprintf(stderr, "\t[port: %s] [signal: %s]\n", port->name, sig->name);
			return -1;
		}
		if (port1->parent == NULL)
		{
			fprintf(stderr, "error: data_get_actor_connected_to_input_through_var(): Connected port's actor is NULL\n");
			fprintf(stderr, "\t[port: %s] [signal: %s] [port: %s]\n", port->name, sig->name, port1->name);
			return -1;
		}
		*ctx = port1->parent;
	}
	else
	{
		fprintf(stderr, "error: data_get_actor_connected_to_input_through_var(): Connected signal is not a var\n");
		fprintf(stderr, "\t[port: %s] [signal: %s]\n", port->name, sig->name);
		return -1;
	}
	return 0;
}

// ************************** Checking functions *******************************

int	dataset_check_connection(void *set)
{
	data_t	*d = NULL;
	
	if (set == NULL)
	{
		fprintf(stderr,"error: dataset_check_connection(): Signal or port set is NULL\n");
		return -1;
	}
	d = objects_get_first(set);
	while (d != NULL)
	{
		if (d->connection == NULL)
		{
			fprintf(stderr, "error: dataset_check_connection(): Not connected");
			fprintf(stderr, "\t[signal or port: %s]\n", d->name);
			return -1;
		}
		d = objects_get_next(set, d);
	}
	return 0;
}

int	dataset_check_parent(void *set)
{
	data_t	*d = NULL;
	
	if (set == NULL)
	{
		fprintf(stderr,"error: dataset_check_parent(): Signal or port set is NULL\n");
		return -1;
	}
	d = objects_get_first(set);
	while (d != NULL)
	{
		if (d->parent == NULL)
		{
			fprintf(stderr, "error: dataset_check_parent(): Has no parent");
			fprintf(stderr, "\t[signal or port: %s]\n", d->name);
			return -1;
		}
		d = objects_get_next(set, d);
	}
	return 0;
}

int	dataset_check_readers(void *set)
{
	data_t	*d, *port;
	void	*ptr;
	int		got_one;
	
	if (set == NULL)
	{
		fprintf(stderr,"error: dataset_check_readers(): Siganal or port set is NULL\n");
		return -1;
	}
	d = (data_t *) objects_get_first(set);
	while (d != NULL)
	{
		if (d->readers == NULL)
		{
			fprintf(stderr, "error: dataset_check_readers(): reader set is NULL");
			fprintf(stderr, "\t[signal or port: %s]\n", d->name);
			return -1;
		}
		got_one = 0;
		ptr = objects_get_first(d->readers);
		while (ptr != NULL)
		{
			port = (data_t *) pointer_get_raw(ptr);
			if (port == NULL)
			{
				fprintf(stderr, "error: dataset_check_readers(): Reader point to a NULL port\n");
				fprintf(stderr, "\t[signal or port: %s]\n", d->name);
				return -1;
			}
			got_one = -1;
			ptr = objects_get_next(d->readers, ptr);
		}
		if (got_one == 0)
		{
			fprintf(stderr, "error: dataset_check_readers(): has no reader");
			fprintf(stderr, "\t[signal or port: %s]\n", d->name);
			return -1;
		}
		d = (data_t *) objects_get_next(set, d);
	}
	return 0;
}

int	dataset_check_writer(void *set)
{
	data_t	*d;
	
	if (set == NULL)
	{
		fprintf(stderr,"error: dataset_check_writer(): Signal or port set is NULL\n");
		return -1;
	}
	d = (data_t *) objects_get_first(set);
	while (d != NULL)
	{
		if (d->connection == NULL)
		{
			fprintf(stderr, "error: dataset_check_writer(): Has no writer");
			fprintf(stderr, "\t[signal or port: %s]\n", d->name);
			break;
		}
		d = objects_get_next(set, d);
	}
	return 0;
}

int	dataset_check_vector_count(void *set)
{
	data_t	*d, *sig;
	int		count;
	
	if (set == NULL) return -1;
	d = (data_t *) objects_get_first(set);
	while (d != NULL)
	{
		switch (d->type)
		{
			case	DATA_TYPE_INPUT:
				sig = (data_t *) d->connection;
				if ((sig->type == DATA_TYPE_INPUT) || (sig->type == DATA_TYPE_STREAM))
				{
					count = context_get_max_fire_count(d->parent);
					count *= d->vector_count;
					if (sig->vector_count != count)
					{
						fprintf(stderr, "error: dataset_check_vector_count(): Vector count missmatch\n");
						fprintf(stderr, "\t[port: %s] [count: %d] [signal: %s] [count: %d]\n", d->name, count, sig->name, sig->vector_count);
						return -1;
					}
				}
			
				break;
			case	DATA_TYPE_OUTPUT:
				sig = (data_t *) d->connection;
				if ((sig->type == DATA_TYPE_STREAM) || (sig->type == DATA_TYPE_OUTPUT))
				{
					count = context_get_max_fire_count(d->parent);
					count *= d->vector_count;
					if (sig->vector_count != count)
					{
						fprintf(stderr, "error: dataset_check_vector_count(): Vector count missmatch\n");
						fprintf(stderr, "\t[port: %s] [count: %d] [signal: %s] [count: %d]\n", d->name, count, sig->name, sig->vector_count);
						return -1;
					}
				}
				break;
			default:
				return -1;
				break;
		}
		d = (data_t *) objects_get_next(set, d);
	}
	return 0;
}


// ************************** Generating c header file *************************

void	data_generate_c_header(void *dat, void *out)
{
	data_t	*d = (data_t *) dat;
	char	buf[128];
	int		i = 0;
	
	switch (d->type)
	{
		case	DATA_TYPE_INPUT:
			sprintf(&buf[i], "\tconst");
			i = strlen(buf);
			break;
		case	DATA_TYPE_OUTPUT:
			sprintf(&buf[i], "\t");
			i = strlen(buf);
			break;
		case	DATA_TYPE_PARAM:
			sprintf(&buf[i], "\tconst");
			i = strlen(buf);
			break;
		case	DATA_TYPE_STATE:
			sprintf(&buf[i], "\t");
			i = strlen(buf);
			break;
		case	DATA_TYPE_TEMP:
			sprintf(&buf[i], "\t");
			i = strlen(buf);
			break;
	}
	if ((d->value_type > 0) && (d->value_type < ALLOC_COUNT))
		sprintf(&buf[i], "\t%s", data_c_typenames[d->value_type]);
	else
		sprintf(&buf[i], " undef");
	i = strlen(buf);
	sprintf(&buf[i], "\t(* const %s)", d->name);
	i = strlen(buf);
	if (d->vector_count > 1)
	{
		sprintf(&buf[i], "[%d]", d->vector_count);
		i = strlen(buf);
	}
	if (d->vector_size > 1)
	{
		sprintf(&buf[i], "[%d]", d->vector_size);
		i = strlen(buf);
	}
	sprintf(&buf[i], ";\n");
	i = strlen(buf);
	io_write(out, buf, i);
}

// ******************** Creating source code listing ***************************

void	data_print(void *dat, char *prefix, void *out)
{
	data_t	*d = (data_t *) dat;
	word_t	*w = NULL;
	int		i;
	char	line[SCANNER_LINE_LENGTH + 1];
	
	switch (d->type)
	{
		case	DATA_TYPE_INPUT:
			sprintf(line, "%sinput", prefix);
			break;
		case	DATA_TYPE_OUTPUT:
			sprintf(line, "%soutput", prefix);
			break;
		case	DATA_TYPE_PARAM:
			sprintf(line, "%sparameter", prefix);
			break;
		case	DATA_TYPE_STATE:
			sprintf(line, "%sstate", prefix);
			break;
		case	DATA_TYPE_TEMP:
			sprintf(line, "%stemp", prefix);
			break;
		case	DATA_TYPE_STREAM:
			sprintf(line, "%sstream", prefix);
			break;
		case	DATA_TYPE_VAR:
			sprintf(line, "%svar", prefix);
			break;
		case	DATA_TYPE_CONST:
			sprintf(line, "%sconst", prefix);
			break;
	}
	if ((d->value_type > 0) && (d->value_type < ALLOC_COUNT))
		sprintf(&line[strlen(line)], "\t%s", src_lang_get_alloc_name(d->value_type));
	else
		sprintf(&line[strlen(line)], " undef");
	if (d->vector_size >= 0)
		sprintf(&line[strlen(line)], "[%d]", d->vector_size);
	sprintf(&line[strlen(line)], "\t%s", d->name);
	if (d->vector_count >= 0)
		sprintf(&line[strlen(line)], "[%d]", d->vector_count);
	if (d->values != NULL)
	{
		w = (word_t *) objects_get_first(d->values);
		i = 0;
		while (w != NULL)
		{
			if (strlen(line) == 0)
				sprintf(line,"\t\t");
			if (i == 0)
			{
				if (w->value_type == VALUE_TYPE_STRING)
					sprintf(&line[strlen(line)], "\t\"%s\"", w->name);
				else
					sprintf(&line[strlen(line)], "\t%s", w->name);
			}
			else
			{
				if (w->value_type == VALUE_TYPE_STRING)
					sprintf(&line[strlen(line)], " \"%s\"", w->name);
				else
					sprintf(&line[strlen(line)], " %s", w->name);
			}
			i++;
			if (i == 4)
			{
				sprintf(&line[strlen(line)],"\\");
				if (io_put(out,line) != 0) return;
				line[0] = 0;
				i = 0;
			}
			w = (word_t *) objects_get_next(d->values, w);
		}
	}
	if (strlen(line) != 0)
		if (io_put(out,line) != 0) return;
}

void	data_print_connection(void *dat, char *prefix, void *out)
{
	data_t	*d = (data_t *) dat;
	char	line[SCANNER_LINE_LENGTH + 1];
	
	sprintf(line, "%s%s", prefix, d->name);
	switch (d->type)
	{
		case	DATA_TYPE_INPUT:
			if (d->delay == 0)
				sprintf(&line[strlen(line)], "\t<<");
			else
				sprintf(&line[strlen(line)], "\t<%d<", d->delay);
			break;
		case	DATA_TYPE_OUTPUT:
			sprintf(&line[strlen(line)], "\t>>");
			break;
		case	DATA_TYPE_PARAM:
			sprintf(&line[strlen(line)], "\t<<");
			break;
		case	DATA_TYPE_STATE:
			sprintf(&line[strlen(line)], "\t=");
			break;
		case	DATA_TYPE_TEMP:
			sprintf(&line[strlen(line)], "\t=");
			break;
		default:
			break;
	}
	if (d->connection != NULL)
		sprintf(&line[strlen(line)], "\t%s", ((data_t *)d->connection)->name);
	io_put(out,line);
}

void	data_print_set(void *set,char *prefix, void *out)
{
	void *dat;
	
	if (set == NULL) return;
	dat = objects_get_first(set);
	while (dat != NULL)
	{
		data_print(dat, prefix, out);
		dat = objects_get_next(set, dat);
	}
}

void	data_print_set_connections(void *set,char *prefix, void *out)
{
	void *dat;
	
	if (set == NULL) return;
	dat = objects_get_first(set);
	while (dat != NULL)
	{
		data_print_connection(dat, prefix, out);
		dat = objects_get_next(set, dat);
	}
}

// ************************ Generating asm code ********************************

int	data_set_generate_asm_name_table(void *set, void *out)
{
	data_t		*dat;
	char		line[SCANNER_LINE_LENGTH + 1];

	dat = (data_t *) objects_get_first(set);
	while (dat != NULL)
	{
		sprintf(line, src_lang_get_asm_line(L_DEF_NAME),dat->name, dat->name);
		if (io_put(out,line) != 0) return -1;
		dat = objects_get_next(set, dat);
	}
	return 0;
}

int	data_set_generate_asm_make_script(void *set, void *out)
{
	data_t		*dat;
	char		line[SCANNER_LINE_LENGTH + 1];
	char		*type_name;
	int			size;

	dat = (data_t *) objects_get_first(set);
	while (dat != NULL)
	{
		type_name = src_lang_get_alloc_name(dat->value_type);
		size = dat->vector_size * dat->vector_count;
		sprintf(line, src_lang_get_asm_line(L_MAKE_BUF),type_name, size, dat->name, dat->name);
		if (io_put(out,line) != 0) return -1;
		dat = objects_get_next(set, dat);
	}
	return 0;
}

int	data_set_generate_asm_actor_port_init(void *set, void *out)
{
	data_t		*port;
	data_t		*sig;
	char		line[SCANNER_LINE_LENGTH + 1];

	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		sig = (data_t *) port->connection;
		if ((sig->type == DATA_TYPE_INPUT) ||
			(sig->type == DATA_TYPE_OUTPUT) ||
			(sig->type == DATA_TYPE_PARAM) ||
			(sig->type == DATA_TYPE_STATE) ||
			(sig->type == DATA_TYPE_TEMP))
		{
			sprintf(line,src_lang_get_asm_line(L_CP_CTX_PTR),port->name,sig->name);
		}
		else
		{
			sprintf(line,src_lang_get_asm_line(L_CP_PTR),port->name,sig->name);
		}
		if (io_put(out,line) != 0) return -1;
		port = (data_t *) objects_get_next(set, port);
	}
	return 0;
}

int	data_set_generate_asm_actor_port_update(void *set, void *out)
{
	data_t		*port;
	int			size;
	char		line[SCANNER_LINE_LENGTH + 1];

	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		if (port->not_for_update == 0)
		{
			size = src_lang_get_alloc_size(port->value_type);
			size = size * port->vector_size * port->vector_count;
			sprintf(line,src_lang_get_asm_line(L_PADD_LINT),port->name,size);
			if (io_put(out,line) != 0) return -1;
		}
		port = (data_t *) objects_get_next(set, port);
	}
	return 0;
}


int	data_set_generate_asm_signal_pointer(void *set, void *out)
{
	data_t		*dat;
	char		line[SCANNER_LINE_LENGTH + 1];

	dat = (data_t *) objects_get_first(set);
	while (dat != NULL)
	{
		sprintf(line, src_lang_get_asm_line(L_DEF_PTR),dat->name, dat->name);
		if (io_put(out,line) != 0) return -1;
		dat = objects_get_next(set, dat);
	}
	return 0;
}

int	data_set_generate_asm_signals(void *set, void *out)
{
	data_t		*sig;
	word_t		*w;
	char		*type_name;
	int			size,i,l;
	char		line[SCANNER_LINE_LENGTH + 1];

	sig = (data_t *) objects_get_first(set);
	while (sig != NULL)
	{
		type_name = src_lang_get_alloc_name(sig->value_type);
		size = sig->vector_size * sig->vector_count;
		sprintf(line, src_lang_get_asm_line(L_DEF_SIG),sig->name,type_name,size);
		l = strlen(line);
		if (sig->type == DATA_TYPE_CONST)
		{
			if (sig->values != NULL)
			{
				w = objects_get_first(sig->values);
				i = 0;
				while(w != NULL)
				{
					if (l == 0)
					{
						sprintf(line, "\t\t");
						l = strlen(line);
					}
					if (i == 0)
					{
						if (w->value_type == VALUE_TYPE_STRING)
							sprintf(&line[l], "\t\"%s\"", w->name);
						else
							sprintf(&line[l], "\t%s", w->name);
					}
					else
					{
						if (w->value_type == VALUE_TYPE_STRING)
							sprintf(&line[l], " \"%s\"", w->name);
						else
							sprintf(&line[l], " %s", w->name);
					}
					l = strlen(line);
					i++;
					if (i == 4)
					{
						sprintf(&line[l],"\\");
						if (io_put(out,line) != 0) return -1;
						l = 0;
						i = 0;
					}
					w = objects_get_next(sig->values, w);
				}
			}
		}
		if (l != 0)
			if (io_put(out,line) != 0) return -1;
		sig = objects_get_next(set, sig);
	}
	return 0;
}

int	data_set_generate_asm_port_pointer(void *set, void *out)
{
	data_t		*port;
	char		line[SCANNER_LINE_LENGTH + 1];

	port = (data_t *) objects_get_first(set);
	while (port != NULL)
	{
		sprintf(line, src_lang_get_asm_line(L_DEF_PORT_PTR),port->name);
		if (io_put(out,line) != 0) return -1;
		port = objects_get_next(set, port);
	}
	return 0;
}

// ************************** Generating c language code ***********************


void	data_set_generate_c_header(void *set, void *out)
{
	void *dat;
	
	if (set == NULL) return;
	dat = objects_get_first(set);
	while (dat != NULL)
	{
		data_generate_c_header(dat, out);
		dat = objects_get_next(set, dat);
	}
}

int		data_delete(void *set, void *d)
{
	if(data_initialized == 0) return -1;
	if ((set == NULL) || (d == NULL)) return -1;
	if (objects_verify_object(set, d) != 0) return -1;
	if (((data_t *)d)->values != NULL)
		words_delete_set(((data_t *)d)->values);
	return objects_delete(set, d);
}

int		data_delete_set(void *set)
{
	data_t	*d;
	
	if(data_initialized == 0) return -1;
	if (set == NULL) return -1;
	d = (data_t *) objects_get_first(set);
	while (d != NULL)
	{
		words_delete_set(d->values);
		d = (data_t *) objects_get_next(set, (void *) d);
	}
	return objects_delete_set(set);
}

int		data_cleanup(void)
{
	void	*set;
	data_t	*d;
	
	if(data_initialized == 0) return -1;
	set = objects_get_first_set(data_type_index);
	while (set != NULL)
	{
		d = (data_t *)objects_get_first(set);
		while (d != NULL)
		{
			words_delete_set(d->values);
			d = (data_t *)objects_get_next(set, (void *)d);
		}
		set = objects_get_next_set(data_type_index,set);
	}
	objects_delete_type(data_type_index);
//	data_type_index = -1;
	data_initialized = 0;
	return 0;
}

// ************************* Local functions ***********************************

static data_t	*_data_clone(data_t *d, void *clone_set, void *parent, char *prefix, void *hash)
{
	data_t	*clone;
	char	name[(2 * WORD_NAME_LENGTH) + 1];
	int		l;
	word_t	*w;
	
	l = strlen(prefix);
	if (l > WORD_NAME_LENGTH - 1) return NULL;
	strcpy(name, prefix);
	name[l++] = '.';
	name[l] = 0;
	if ((clone = (data_t *) objects_new(clone_set, sizeof(data_t))) == NULL) return NULL;
	memcpy(clone, d, sizeof(data_t));
	strcpy(&name[l], d->name);
	if (strlen(name) > WORD_NAME_LENGTH)
	{
		objects_delete(clone_set, clone);
		return NULL;
	}
	strcpy(clone->name, name);
	if (d->values != NULL)
	{
		clone->values = words_dup_set(d->values);
		if (clone->values == NULL)
		{
			data_delete(clone_set, clone);
			return NULL;
		}
	}
	clone->parent = parent;
	if (hash != NULL)
	{
		w = hash_table_lookup(hash, name);
		if (w != NULL)
		{
			if (w->valid != 0)
			{
				data_delete(clone_set, clone);
				return NULL;
			}
			w->type = word_types[clone->type];
			w->value_type = VALUE_TYPE_PTR;
			w->value.p = clone;
			w->valid = -1;
		}
	}
	return clone;
}
