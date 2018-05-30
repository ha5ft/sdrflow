/*******************************************************************************
 * 							Common asmcode
 * *****************************************************************************
 * 	Filename:		asm_code.c
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

#include	<string.h>
#include	<stdio.h>
#include	"asm_code.h"
#include	"sentence.h"
#include	"scanner.h"
#include	"hash_table.h"
#include	"value.h"
#include	"allocators.h"
#include	"instructions.h"
#include	"labels.h"
#include	"segments.h"
#include	"obj_format.h"
#include	"asm_lang.h"
#include	"objects.h"
#include	"words.h"
#include	"sdferr.h"
#include	"io.h"
#include	"common.h"

//#define	SET_SIZE						2

#define STATE_SEG_START					0
#define STATE_META_SEG_EOL				1

#define STATE_META_ALLOC_ARRAY_SIZE		3

#define STATE_META_INSTR_EOL			5
#define STATE_META_LABEL_OR_INSTR		6
#define STATE_META_INSTR				7
#define STATE_META_INSTR_OP				8
#define STATE_META_ALLOC_OP				9

#define STATE_CODE_ALLOC_ARRAY_SIZE		10
#define STATE_CODE_ALLOC_OP				11

#define	STATE_DATA_SEG_EOL				28
#define STATE_DATA_SEG_LABEL_OR_ALLOC	29
#define STATE_DATA_SEG_ALLOC			30
#define STATE_DATA_SEG_SIZE_OR_OP		31
#define STATE_DATA_SEG_OP				32

#define	STATE_CODE_SEG_EOL				33
#define STATE_CODE_SEG_LABEL_OR_INSTR	34
#define STATE_CODE_SEG_INSTR			35
#define STATE_CODE_SEG_OP				36

#define STATE_SEG_END_EOL				37

#define STATE_ERROR						38
#define STATE_END						39

#define	ASM_CODE_STATUS_INIT				0
#define	ASM_CODE_STATUS_ANALYSE			1
#define	ASM_CODE_STATUS_GENERATE			2

typedef struct _asm_code
{
	void			*scan;
	void			*hash;
	void			*sentence_set;
	void			*clones;
	void			*cur_sentence;
	int				loc[SEGDEF_COUNT];
	int				cat_count;
	sdferr_t		err;
} asm_code_t;

static void	*asm_code_set = NULL;
static int 	asm_code_initialized = 0;
static unsigned int	asm_code_type_index = (unsigned int) OBJECTS_TYPE_ASM_CODE;

static int	seg_eol_state[SEGDEF_COUNT] =
{
	[META_SEG] = STATE_META_SEG_EOL,
	[CODE_SEG] = STATE_CODE_SEG_EOL,
	[DATA_SEG] = STATE_DATA_SEG_EOL,
	[CONTEXT_SEG] = STATE_DATA_SEG_EOL,
	[END_SEG] = STATE_ERROR
};

static char	*error_description[43] =
{
	[0] = "No error",
	[1] = "Can not create new sentence",
	[2] = "Segment end is not allowed here",
	[3] = "Segment start is expected",
	[4] = "EOL is expected",
	[5] = "Address or string allocator is expected",
//	[6] = "Allocator is expected",
//	[7] = "Array size is expected",
	[6] = "Array size is allowed only after an allocator",
	[7] = "Literal, label, allocator or array size is expected",
	[8] = "Label is expected",
	[9] = "Label already has been defined",
	[10] = "Label meta instruction or allocator is expected",
	[11] = "Meta instruction or allocator is expected",
	[12] = "Incompatible operand",
	[13] = "Label, literal or type is expected",
	[14] = "String literal is expected",
	[15] = "Literal is expected",
	[16] = "Label, allocator or segend is expected",
	[17] = "Unexpected EOL",
	[18] = "Literal or label is expected",
	[19] = "Label or instruction is expected",
	[20] = "Instruction or allocator or EOL is expected",
	[21] = "Unknown token",
	[22] = "Label has not been defined",
	[23] = "Sentence length missmatch",
	[24] = "segment length missmatch",
	[25] = "Unknown word in the sentence",
	[26] = "Can not create asm_code object",
	[27] = "can not start asm_code assembling",
	[28] = "Can not allocate compilation buffer",
	[29] = "Can not duplicate a word",
	[30] = "Address allocator is not allowed here",
	[31] = "Can not set composite's memory",
	[32] = "can not compile the sentence",
	[33] = "Can not alolocate catalog buffer",
	[34] = "Can not generate prologs",
	[35] = "Label .code.offset not exists",
	[36] = "Label .data.offset not exists",
	[37] = "Label .size not exists",
	[38] = "Scanner error",
	[39] = "Can not clone a word",
	[40] = "Array size is expected here",
	[41] = "Label .ctx.size not exists",
	[42] = "Too long string literal"
};

static char	*prolog[] =
{
	"\t.meta", //1
	"\tchar\t's'", //2
	"\tchar\t'd'", //3
	"\tchar\t'f'", //4
	"\tchar\t0", //5
	"\taddress\t.name", //6
	"\taddress\t.version", //7
	"\taddress\t.size", //8
	"\taddress\t.code.offset", //9
	"\taddress\t.data.offset", //10
	"\taddress\t.ctx.size", //11
	"\tint\t0", //12
	"\taddress\t.load", //13
	"\taddress\t.make", //14
	"\taddress\t.delete", //15
	"\t.endseg", //16
	"\t.code", //17
	"\taddress\t.fire", //18
	"\taddress\t.init", //19
	"\taddress\t.clean", //20
	"\t.endseg", // 21
	"\t.data", //22
	"\tptr\t0", //23
	"\tptr\t0", //24
	"\tptr\t0", //25
	"\tptr\t0", //26
	"\tint\t0", //27
	"\tint\t0", //28
	"\tptr\t0", //29
	"\t.endseg", //30
	NULL //31
};

#define PROLOG_SIZE		30


/*******************************************************************************
 * 							Public functions
 ******************************************************************************/
 
int		asm_code_init()
{
	if (asm_code_initialized != 0) return -1;
//	if ((asm_code_type_index = objects_register_type(sizeof(asm_code_t), SET_SIZE)) == -1) return -1;
	asm_code_type_index = (unsigned int) OBJECTS_TYPE_ASM_CODE;
	if ((asm_code_set = objects_new_set(asm_code_type_index)) == NULL) return -1;
	asm_code_initialized = -1;
	return 0;
}

void	*asm_code_new(void)
{
	asm_code_t	*ac;
	int			cur_seg;
	
//	fprintf(stderr,"debug: asm_code_new()\n");
	if (asm_code_initialized == 0) return NULL;
	if ((ac = (asm_code_t *) objects_new(asm_code_set, sizeof(asm_code_t))) == NULL) return NULL;
	if ((ac->sentence_set = sentence_new_set()) == NULL)
	{
		fprintf(stderr,"error: asm_code_new(): Can not create sentence set\n");
		asm_code_delete(ac);
		return NULL;
	}
//	fprintf(stderr,"debug: asm_code_new() : Sentence set created\n");
	if ((ac->scan = scanner_new()) == NULL)
	{
		fprintf(stderr,"error: asm_code_new(): Can not create new scanner\n");
		asm_code_delete(ac);
		return NULL;		
	}
//	fprintf(stderr,"debug: asm_code_new() : Scenner created\n");
	if (( ac->hash = hash_table_new()) == NULL)
	{
		fprintf(stderr,"error: asm_code_new(): Can not create hash table\n");
		asm_code_delete(ac);
		return NULL;				
	}
//	fprintf(stderr,"debug: asm_code_new() : Hash table created\n");
	if (asm_lang_init_hash_table(ac->hash) != 0)
	{
		fprintf(stderr,"error: asm_code_new(): Can not initialize hash table\n");
		asm_code_delete(ac);
		return NULL;				
	}
//	fprintf(stderr,"debug: asm_code_new() : Hash table initialized\n");
	if ((ac->clones = words_new_set()) == NULL)
	{
		fprintf(stderr,"error: asm_code_new(): Can not create word set for clones\n");
		asm_code_delete(ac);
		return NULL;		
	}
//	fprintf(stderr,"debug: asm_code_new() : Clone word set created\n");
	ac->cur_sentence = NULL;
	ac->err.errnr = ac->err.lineno = 0;
	ac->err.errstr = error_description;
	for (cur_seg = 0; cur_seg < SEGDEF_COUNT; cur_seg++)
		ac->loc[cur_seg] = 0;
	return (void *) ac;
}

//void *asm_code_parse(void *asm_code, void *inp)
//{
//	asm_code_t	*ac = (asm_code_t *) asm_code;
//	
//	if (asm_code == NULL) return NULL;
//	if (asm_code_analyse(asm_code, inp) != 0) return NULL;
//	if (asm_code_set_sizes(asm_code) != 0) return NULL;
//	return asm_code_generate(ac);
//}

sdferr_t	*asm_code_get_error(void *asm_code)
{
	return &((asm_code_t *) asm_code)->err;
}

int	asm_code_delete(void *asm_code)
{
	if (asm_code == NULL) return -1;
	if (((asm_code_t *) asm_code)->scan != NULL)
		scanner_delete(((asm_code_t *) asm_code)->scan);
	if (((asm_code_t *) asm_code)->hash != NULL)
		hash_table_delete(((asm_code_t *) asm_code)->hash);
	if (((asm_code_t *) asm_code)->sentence_set != NULL)
		sentence_delete_set(((asm_code_t *) asm_code)->sentence_set);
	if (((asm_code_t *) asm_code)->clones != NULL)
		words_delete_set(((asm_code_t *) asm_code)->clones);
	return objects_delete(asm_code_set, asm_code);
}

int	asm_code_cleanup(void)
{
	void	*ac, *ac0;
	int		ret;
	
	if (asm_code_set != NULL)
	{
		ac = objects_get_first(asm_code_set);
		while (ac != NULL)
		{
			ac0 = ac;
			ac = objects_get_next(asm_code_set, ac0);
			asm_code_delete(ac0);
		}
	}
	ret = objects_delete_type(asm_code_type_index);
	asm_code_initialized = 0;
	return ret;
}

/*******************************************************************************
 * 							Private functions
 ******************************************************************************/

static int	asm_code_add_word(asm_code_t *ac, void *w)
{
	return sentence_add_word(ac->cur_sentence, w);
}

static int	asm_code_new_sentence(asm_code_t *ac, void *w, int lineno)
{
	if ((ac->cur_sentence = sentence_new(ac->sentence_set,scanner_lineno(ac->scan))) != NULL)
	{
		return sentence_add_word(ac->cur_sentence,w);
	}
	return -1;
}

int asm_code_build_prolog(void *asm_code)
{
//	asm_code_t	*ac = (asm_code_t *)asm_code;
	void		*chan;
	
	if ((chan = io_new_memory_channel(prolog,"prolog",IO_ITEM_TYPE_ASM_CODE)) == NULL)
	{
		fprintf(stderr, "Can not create new io memory channel\n");
		return -1;
	}
	if (io_open_for_read(chan) == -1)
	{
		io_delete_channel(chan);
		fprintf(stderr, "Can not open io memory channel\n");
		return -1;
	}
	if (asm_code_analyse(asm_code, chan) != 0)
	{
		fprintf(stderr, "Parsing error in analysing prolog\n");
		return -1;
	}
	io_close(chan);
	io_delete_channel(chan);
	return 0;
}


int asm_code_set_sizes(void *asm_code)
{
	asm_code_t	*ac = (asm_code_t *) asm_code;
	word_t		*w, *rw;
	int			ret;
	
	ret = 0;
	if ((w = hash_table_check(ac->hash, asm_lang_label_name(LABEL_CODE_OFFSET))) != NULL)
	{
		if (w->root != NULL)
			rw = w->root;
		else
			rw = w;
		rw->value.a.offs = ac->loc[META_SEG];
		rw->valid = -1;
		rw->comp_type = VALUE_TYPE_ADDR;
		if ((w = hash_table_check(ac->hash, asm_lang_label_name(LABEL_DATA_OFFSET))) != NULL)
		{
			if (w->root != NULL)
				rw = w->root;
			else
				rw = w;
			rw->value.a.offs = ac->loc[META_SEG] + ac->loc[CODE_SEG];
			rw->valid = -1;
			rw->comp_type = VALUE_TYPE_ADDR;
			if ((w = hash_table_check(ac->hash, asm_lang_label_name(LABEL_COMP_SIZE))) != NULL)
			{
				if (w->root != NULL)
					rw = w->root;
				else
					rw = w;
				rw->value.a.offs = ac->loc[META_SEG] + ac->loc[CODE_SEG] + ac->loc[DATA_SEG];
				rw->valid = -1;
				rw->comp_type = VALUE_TYPE_ADDR;
				if ((w = hash_table_check(ac->hash, asm_lang_label_name(LABEL_CONTEXT_SIZE))) != NULL)
				{
					if (w->root != NULL)
						rw = w->root;
					else
						rw = w;
					rw->value.a.offs = ac->loc[CONTEXT_SEG];
					rw->valid = -1;
					rw->comp_type = VALUE_TYPE_ADDR;
				}
				else
				{
					ac->err.errnr = 41;
					ret = -1;
				}
			}
			else
			{
				ac->err.errnr = 37;
				ret = -1;
			}
		}
		else
		{
			ac->err.errnr = 36;
			ret = -1;
		}
	}
	else
	{
		ac->err.errnr = 35;
		ret = -1;
	}
	
	if (ret == -1)
	{
		ac->err.lineno = scanner_lineno(ac->scan);
		fprintf(stderr,"[line: %d] [error: %d] %s\n", ac->err.lineno - PROLOG_SIZE, ac->err.errnr, error_description[ac->err.errnr]);
	}
	
	return ret;
}

int	asm_code_analyse(void *asm_code, void *inp)
{
	word_t		*w = NULL;
	word_t		*rw = NULL;
	word_t		*lw = NULL;
	int			cur_type, cur_loc, cur_len, n;
	int			cur_instr, cur_alloc, cur_seg;
	int			op_count, op_size, op_type, cur_count;
	int 		state, errnr;
	int			ret = 0;
	sdferr_t	*scan_err = NULL;
	asm_code_t	*ac = (asm_code_t *)asm_code;
	char		inp_line[SCANNER_LINE_LENGTH + 1];
	int			str_len;

	cur_type = cur_loc = cur_len = n = 0;
	cur_instr = cur_alloc = cur_seg = 0;
	op_count = op_size = op_type = cur_count = 0;
	state = STATE_SEG_START;
	errnr = 0;
	while (1)
	{
		if ((rw = scanner_scan(ac->scan, inp, ac->hash)) == NULL)
		{
			scan_err = scanner_error(ac->scan);
			switch (scan_err->errnr)
			{
				case	SCANNER_ERROR_EOL:
					cur_type = WORD_TYPE_EOL;
					//fprintf(stderr,"\n");
					break;
				case	SCANNER_ERROR_EOF:
					cur_type = WORD_TYPE_EOF;
					break;
				default:
					ac->err.errnr = errnr = 38;
					ac->err.lineno = scanner_lineno(ac->scan);
					if (scanner_get_line(ac->scan, inp_line, SCANNER_LINE_LENGTH + 1) != 0)
						inp_line[0] = 0;
					fprintf(stderr,"[line: %d] [error: %d] %s\n", ac->err.lineno - PROLOG_SIZE, errnr, error_description[errnr]);
					fprintf(stderr, "[source line:%d] %s\n", ac->err.lineno - PROLOG_SIZE, inp_line);
					return -1;
			}
		}
		else
		{
			cur_type = rw->type;
			if ((cur_type == WORD_TYPE_LABEL) ||
				(cur_type == WORD_TYPE_INSTRUCTION) ||
				(cur_type == WORD_TYPE_META) ||
				(cur_type == WORD_TYPE_SEGDEF) ||
				(cur_type == WORD_TYPE_SEG_END) ||
				(cur_type == WORD_TYPE_ALLOCATOR))
			{
				w = words_clone_word(ac->clones, rw);
				if (w == NULL)
				{
					ac->err.errnr = errnr = 38;
					ac->err.lineno = scanner_lineno(ac->scan);
					if (scanner_get_line(ac->scan, inp_line, SCANNER_LINE_LENGTH + 1) != 0)
						inp_line[0] = 0;
					fprintf(stderr,"[line: %d] [error: %d] %s\n", ac->err.lineno - PROLOG_SIZE, errnr, error_description[errnr]);
					fprintf(stderr, "[source line:%d] %s\n", ac->err.lineno - PROLOG_SIZE, inp_line);
					return -1;					
				}
			}
			else
				w = rw;
		}
		switch (state)
		{
//------------------------------------------------------------------------------
//					SEGMENT START
//------------------------------------------------------------------------------
			case	STATE_SEG_START:
				if (cur_type == WORD_TYPE_SEGDEF)
				{
					cur_seg = rw->value.l;
					if (cur_seg != END_SEG)
					{
						cur_loc = ac->loc[cur_seg];
						if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) == 0)
						{
							cur_len = 0;
							state = seg_eol_state[cur_seg];
						}
						else
						{
							errnr = 1; // Can not create new sentence
							state = STATE_ERROR;
						}
					}
					else
					{
						errnr = 2; // Segment end is not allowed
						state = STATE_ERROR;
					}
					break;
				}
				if (cur_type == WORD_TYPE_EOL)
					break;
				if (cur_type == WORD_TYPE_EOF)
				{
					state = STATE_END;
					break;
				}
				errnr = 3;
				state = STATE_ERROR;
				break;

//------------------------------------------------------------------------------
//					META SEGMENT
//------------------------------------------------------------------------------

			case	STATE_META_SEG_EOL:
				if (cur_type == WORD_TYPE_EOL)
				{
					sentence_set_size(ac->cur_sentence, cur_len);
					state = STATE_META_LABEL_OR_INSTR;
				}
				else
				{
					errnr = 4;
					state = STATE_ERROR;
				}
				break;
			case	STATE_META_INSTR_EOL:
				if (cur_type == WORD_TYPE_EOL)
				{
					sentence_set_size(ac->cur_sentence, cur_len);
					state = STATE_META_LABEL_OR_INSTR;
					break;
				}
				errnr = 4;
				state = STATE_ERROR;
				break;
			case	STATE_META_LABEL_OR_INSTR:
				if (cur_type == WORD_TYPE_EOL)
					break;
				if (cur_type == WORD_TYPE_SEG_END)
				{
					ac->loc[cur_seg] = cur_loc;
					cur_seg = 0;
					cur_len = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
					{
						state = STATE_ERROR;
						errnr = 1;
						break;
					}
					state = STATE_SEG_END_EOL;
					break;
				}
				if (cur_type == WORD_TYPE_LABEL)
				{
					if (rw->valid == 0)
					{
						cur_len = 0;
						rw->value.a.seg = cur_seg; //assign value to the root word
						rw->value.a.offs = cur_loc;
						rw->valid = -1; // validity should allways checked in the root word
						if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
						{
							errnr = 1;
							state = STATE_ERROR;
							break;
						}
						state = STATE_META_INSTR;
						break;
					}
					errnr = 9;
					state = STATE_ERROR;
					break;
				}
				if (cur_type == WORD_TYPE_META)
				{
					cur_instr = rw->value.l;
					cur_loc += asm_lang_datasize(w->comp_type);
					cur_len = asm_lang_datasize(w->comp_type);
					op_count = asm_lang_instr_opcount(cur_instr);
					cur_count = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
					{
						errnr = 1;
						state = STATE_ERROR;
						break;
					}
					if (op_count != 0)
						state = STATE_META_INSTR_OP;
					else
						state = STATE_META_INSTR_EOL;
					break;
				}
				if (cur_type == WORD_TYPE_ALLOCATOR)
				{
					cur_alloc = (int) rw->value.l;
					op_type = asm_lang_alloc_optype(cur_alloc);
					op_size = asm_lang_datasize(op_type);
					op_count = 1;
					cur_count = 0;
					cur_len = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
					{
						errnr = 1;
						state = STATE_ERROR;
						break;
					}
					if (cur_alloc == STRING_ALLOC)
						state = STATE_META_ALLOC_OP;
					else
						state = STATE_META_ALLOC_ARRAY_SIZE;
					break;
				}
				errnr = 10;
				state = STATE_ERROR;
				break;
			case	STATE_META_INSTR:
				if (cur_type == WORD_TYPE_EOL)
				{
					state = STATE_META_LABEL_OR_INSTR;
					break;
				}
				if (cur_type == WORD_TYPE_META)
				{
					cur_instr = rw->value.l;
					cur_loc += asm_lang_datasize(w->comp_type);
					cur_len += asm_lang_datasize(w->comp_type);
					op_count = asm_lang_instr_opcount(cur_instr);
					cur_count = 0;
					asm_code_add_word(ac,w);
					if (op_count != 0)
						state = STATE_META_INSTR_OP;
					else
						state = STATE_META_INSTR_EOL;
					break;
				}
				if (cur_type == WORD_TYPE_ALLOCATOR)
				{
					cur_alloc = (int) rw->value.l;
					op_type = asm_lang_alloc_optype(cur_alloc);
					op_size = asm_lang_datasize(op_type);
					op_count = 1;
					cur_count = 0;
					asm_code_add_word(ac,w);
					if (cur_alloc == STRING_ALLOC)
						state = STATE_META_ALLOC_OP;
					else
						state = STATE_META_ALLOC_ARRAY_SIZE;
					cur_len = 0;
					break;
				}
				errnr = 11;
				state = STATE_ERROR;
				break;
			case	STATE_META_INSTR_OP:
				op_type = asm_lang_instr_optype(cur_instr, cur_count);
				op_size = asm_lang_datasize(op_type);
				if((cur_type == WORD_TYPE_LABEL) || (cur_type == WORD_TYPE_LITERAL))
				{
					if (lw != NULL)
					{
						errnr = 40; // Array size is expected
						state = STATE_ERROR;
						lw = NULL;
						break;
					}
					if (asm_lang_is_compatible(rw->value_type, op_type))
					{
						if ((cur_type == WORD_TYPE_LABEL) && (op_type == VALUE_TYPE_OFFSET))
						{
							w->value.a.offs = cur_loc;
							w->value.a.seg = cur_seg;
						}
						cur_loc += op_size;
						cur_len += op_size;
						w->comp_type = op_type;
						asm_code_add_word(ac, w);
						if (++cur_count == op_count)
						{
							state = STATE_META_INSTR_EOL;
						}
						break;
					}
					errnr = 12;
					state = STATE_ERROR;
					break;
				}
				if (cur_type == WORD_TYPE_ALLOCATOR)
				{
					if (lw != NULL)
					{
						errnr = 40; // Array size is expected
						state = STATE_ERROR;
						lw = NULL;
						break;
					}
					lw = w;
					break;
				}
				if (cur_type == WORD_TYPE_ARRAY_SIZE)
				{
					if (lw == NULL)
					{
						errnr = 6; //array size is allowed only after an allocator
						state = STATE_ERROR;
						break;
					}
					if (asm_lang_is_compatible(rw->value_type, op_type))
					{
						cur_loc += op_size;
						cur_len += op_size;
						w->comp_type = op_type;
						w->value.l *= asm_lang_datasize((int)lw->value.l);
						words_prefix_name(w, lw->name);
						asm_code_add_word(ac, w);
						lw = NULL;
						if (++cur_count == op_count)
						{
							state = STATE_META_INSTR_EOL;
						}
						break;
					}
					lw = NULL;
					errnr = 12; // Incompatible operand
					state = STATE_ERROR;
					break;					
				}
				lw = NULL;
				errnr = 7; //Literal, label, allocator or array size is expected
				state = STATE_ERROR;
				break;
			case	STATE_META_ALLOC_ARRAY_SIZE:
				if (cur_type == WORD_TYPE_ARRAY_SIZE)
				{
					op_count = (int) rw->value.l;
					w->comp_type = VALUE_TYPE_NULL;
					asm_code_add_word(ac, w);
					state = STATE_META_ALLOC_OP;
					break;
				}
				// No break is allowed here
			case	STATE_META_ALLOC_OP:
				if (cur_type == WORD_TYPE_EOL)
				{
					if (cur_alloc == STRING_ALLOC)
					{
						errnr = 17; // Unexpected EOL
						state = STATE_ERROR;
						break;
					}
					cur_loc += (op_count - cur_count)  * op_size;
					cur_len += (op_count - cur_count)  * op_size;
					sentence_set_size(ac->cur_sentence,cur_len);
					state = STATE_META_LABEL_OR_INSTR;
					break;
				}
				if ((cur_type == WORD_TYPE_LITERAL) || (cur_type == WORD_TYPE_LABEL))
				{
					if (asm_lang_is_compatible(rw->value_type, op_type))
					{
						if (cur_alloc == STRING_ALLOC)
						{
							cur_loc += op_size * (strlen(w->value.s) + 1);
							cur_len += op_size * (strlen(w->value.s) + 1);
							w->comp_type = op_type;
							asm_code_add_word(ac, w);
							state = STATE_META_SEG_EOL;
							break;
						}
						w->comp_type = op_type;
						asm_code_add_word(ac, w);
						cur_loc += op_size;
						cur_len += op_size;
						if (++cur_count == op_count)
							state = STATE_META_SEG_EOL;
						break;
					}
					state = STATE_ERROR;
					errnr = 12; // incompatible operand;
					break;
				}
				state = STATE_ERROR;
				errnr = 18; // Literal or label is expected
				break;
			
//------------------------------------------------------------------------------
//					DATA SEGMENT
//------------------------------------------------------------------------------

			case	STATE_DATA_SEG_EOL:
				if (cur_type == WORD_TYPE_EOL)
				{
					sentence_set_size(ac->cur_sentence,cur_len);
					state = STATE_DATA_SEG_LABEL_OR_ALLOC;
					break;
				}
				state = STATE_ERROR;
				errnr = 4; // EOL is expected
				break;
			case	STATE_DATA_SEG_LABEL_OR_ALLOC:
				if (cur_type == WORD_TYPE_EOL)
					break;
				if (cur_type == WORD_TYPE_SEG_END)
				{
					ac->loc[cur_seg] = cur_loc;
					cur_seg = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) == 0)
					{
						state = STATE_SEG_END_EOL;
						cur_len = 0;
						break;
					}
					state = STATE_ERROR;
					errnr = 1; // Can not create new sentence
					break;
				}
				if (cur_type == WORD_TYPE_LABEL)
				{
					if (rw->valid == 0) // validity always should be checked with the roor word
					{
						rw->value.a.seg = cur_seg;
						rw->value.a.offs = cur_loc;
						rw->valid = -1;
						if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
						{
							state = STATE_ERROR;
							errnr = 1; // Can not create new sentence
							break;
						}
						state = STATE_DATA_SEG_ALLOC;
						cur_len = 0;
						break;
					}
					state = STATE_ERROR;
					errnr = 9; // Label already has defined
					break;
				}
				if (cur_type == WORD_TYPE_ALLOCATOR)
				{
					cur_alloc = (int) rw->value.l;
					op_type = asm_lang_alloc_optype(cur_alloc);
					op_size = asm_lang_datasize(op_type);
					op_count = 1;
					cur_count = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
					{
						state = STATE_ERROR;
						errnr = 1; // Can not create new sentence
						break;
					}
					cur_len = 0;
					if (cur_alloc == STRING_ALLOC)
						state = STATE_DATA_SEG_OP;
					else
					{
						lw = w;
						state = STATE_DATA_SEG_SIZE_OR_OP;
					}
					break;
				}
				errnr = 16; // Label, allocator or segend is expected
				state = STATE_ERROR;
				break;
			case	STATE_DATA_SEG_ALLOC:
				if (cur_type == WORD_TYPE_EOL)
				{
					state = STATE_DATA_SEG_LABEL_OR_ALLOC;
					break;
				}
				if (cur_type == WORD_TYPE_ALLOCATOR)
				{
					cur_alloc = (int) rw->value.l;
					op_type = asm_lang_alloc_optype(cur_alloc);
					op_size = asm_lang_datasize(op_type);
					op_count = 1;
					cur_count = 0;
					asm_code_add_word(ac, w);
					if (cur_alloc == STRING_ALLOC)
						state = STATE_DATA_SEG_OP;
					else
					{
						lw = w;
						state = STATE_DATA_SEG_SIZE_OR_OP;
					}
					break;
				}
				state = STATE_ERROR;
				errnr = 6;
				break;
			case	STATE_DATA_SEG_SIZE_OR_OP:
				if (cur_type == WORD_TYPE_ARRAY_SIZE)
				{
					op_count = (int) rw->value.l;
//					w->comp_type = VALUE_TYPE_NULL;
//					asm_code_add_word(p, w);
					words_postfix_name(lw, w->name);
					lw = NULL;
					state = STATE_DATA_SEG_OP;
					break;
				}
				// Important: There should be no break here.
			case	STATE_DATA_SEG_OP:
				if (lw != NULL)
					lw = NULL;
				if (cur_type == WORD_TYPE_EOL)
				{
					if (cur_alloc == STRING_ALLOC)
					{
						errnr = 17; // Unexpected EOL
						state = STATE_ERROR;
						break;
					}
					cur_loc += (op_count - cur_count)  * op_size;
					cur_len += (op_count - cur_count)  * op_size;
					sentence_set_size(ac->cur_sentence,cur_len);
					state = STATE_DATA_SEG_LABEL_OR_ALLOC;
					break;
				}
				if ((cur_type == WORD_TYPE_LITERAL) || (cur_type == WORD_TYPE_LABEL))
				{
					if (asm_lang_is_compatible(rw->value_type, op_type))
					{
						if (cur_alloc == STRING_ALLOC) // only string literal is allowed here
						{
							str_len = strlen(w->value.s);
							cur_loc += op_size * (str_len + 1);
							cur_len += op_size * (str_len + 1);
							w->comp_type = op_type;
							asm_code_add_word(ac, w);
							state = STATE_DATA_SEG_EOL;
							break;
						}
						if ((cur_alloc == CHAR_ALLOC) && (rw->value_type == VALUE_TYPE_STRING))
						{
							str_len = strlen(w->value.s);
							if ((cur_count + str_len) <= op_count)
							{
								str_len = strlen(w->value.s);
								cur_loc += op_size * str_len;
								cur_len += op_size * str_len;
								w->comp_type = op_type;
								asm_code_add_word(ac, w);
								cur_count += str_len;
								if (cur_count == op_count)
									state = STATE_DATA_SEG_EOL;
								break;
							}
							else
							{
								errnr = 42; // Too long string literal
								state = STATE_ERROR;
							}
							
						}
						w->comp_type = op_type;
						asm_code_add_word(ac, w);
						cur_loc += op_size;
						cur_len += op_size;
						if (++cur_count == op_count)
							state = STATE_DATA_SEG_EOL;
						break;
					}
					state = STATE_ERROR;
					errnr = 12; // incompatible operand;
					break;
				}
				state = STATE_ERROR;
				errnr = 18; // Literal oe label is expected
				break;
//------------------------------------------------------------------------------
//					CODE SEGMENT
//------------------------------------------------------------------------------

			case	STATE_CODE_SEG_EOL:
				if (cur_type == WORD_TYPE_EOL)
				{
					sentence_set_size(ac->cur_sentence,cur_len);
					state = STATE_CODE_SEG_LABEL_OR_INSTR;
					break;
				}
				state = STATE_ERROR;
				errnr = 4; // EOL is expected
				break;
			case	STATE_CODE_SEG_LABEL_OR_INSTR:
				if (cur_type == WORD_TYPE_EOL)
					break;
				if (cur_type == WORD_TYPE_SEG_END)
				{
					ac->loc[cur_seg] = cur_loc;
					cur_seg = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) == 0)
					{
						state = STATE_SEG_END_EOL;
						cur_len = 0;
					}
					else
					{
						state = STATE_ERROR;
						errnr = 1; // Can not create new sentence
					}
					break;
				}
				if (cur_type == WORD_TYPE_LABEL)
				{
					if (rw->valid == 0)
					{
						rw->value.a.seg = cur_seg;
						rw->value.a.offs = cur_loc;
						rw->valid = -1;
						if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
						{
							state = STATE_ERROR;
							errnr = 1; // Can not create new sentence
							break;
						}
						state = STATE_CODE_SEG_INSTR;
						cur_len = 0;
						break;
					}
					state = STATE_ERROR;
					errnr = 9; // Label alredy has been defined
					break;
				}
				if (cur_type == WORD_TYPE_INSTRUCTION)
				{
					cur_instr = rw->value.l;
					cur_loc += asm_lang_datasize(w->comp_type);
					cur_len = asm_lang_datasize(w->comp_type);
					op_count = asm_lang_instr_opcount(cur_instr);
					cur_count = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
					{
						state = STATE_ERROR;
						errnr = 1; // Can not create new sentence
					}
					cur_len = asm_lang_datasize(w->comp_type);
					if (op_count == 0)
						state = STATE_CODE_SEG_EOL;
					else
						state = STATE_CODE_SEG_OP;
					break;
				}
				if (cur_type == WORD_TYPE_ALLOCATOR)
				{
					cur_alloc = (int) rw->value.l;
					op_type = asm_lang_alloc_optype(cur_alloc);
					op_size = asm_lang_datasize(op_type);
					op_count = 1;
					cur_count = 0;
					cur_len = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
					{
						errnr = 1;
						state = STATE_ERROR;
						break;
					}
					if (cur_alloc == STRING_ALLOC)
						state = STATE_CODE_ALLOC_OP;
					else
						state = STATE_CODE_ALLOC_ARRAY_SIZE;
					break;
				}
				errnr = 10;
				state = STATE_ERROR;
				break;
			case	STATE_CODE_SEG_INSTR:
				if (cur_type == WORD_TYPE_EOL)
				{
					state = STATE_CODE_SEG_LABEL_OR_INSTR;
					break;
				}
				if (cur_type == WORD_TYPE_INSTRUCTION)
				{
					cur_instr = rw->value.l;
					cur_loc += asm_lang_datasize(w->comp_type);
					cur_len += asm_lang_datasize(w->comp_type);
					op_count = asm_lang_instr_opcount(cur_instr);
					cur_count = 0;
					asm_code_add_word(ac, w);
					if (op_count == 0)
						state = STATE_CODE_SEG_EOL;
					else
						state = STATE_CODE_SEG_OP;
				}
				if (cur_type == WORD_TYPE_ALLOCATOR)
				{
					cur_alloc = (int) rw->value.l;
					op_type = asm_lang_alloc_optype(cur_alloc);
					op_size = asm_lang_datasize(op_type);
					op_count = 1;
					cur_count = 0;
					cur_len = 0;
					if (asm_code_new_sentence(ac, w, scanner_lineno(ac->scan)) != 0)
					{
						errnr = 1;
						state = STATE_ERROR;
						break;
					}
					if (cur_alloc == STRING_ALLOC)
						state = STATE_CODE_ALLOC_OP;
					else
						state = STATE_CODE_ALLOC_ARRAY_SIZE;
					break;
				}
//				else
//				{
					state = STATE_ERROR;
					errnr = 20; // Instruction is expected
//				}
				break;
			case	STATE_CODE_SEG_OP:
				if (cur_type == WORD_TYPE_EOL)
				{
					state = STATE_ERROR;
					break;
				}
				if ((cur_type == WORD_TYPE_LITERAL) || (cur_type == WORD_TYPE_LABEL))
				{
					if (lw != NULL)
					{
						errnr = 40; // Array size is expected
						state = STATE_ERROR;
						lw = NULL;
						break;
					}
					op_type = asm_lang_instr_optype(cur_instr,cur_count);
					op_size = asm_lang_datasize(op_type);
					if (asm_lang_is_compatible(rw->value_type, op_type))
					{
						if ((cur_type == WORD_TYPE_LABEL) && (op_type == VALUE_TYPE_OFFSET))
						{
							w->value.a.offs = cur_loc;
							w->value.a.seg = cur_seg;
						}
						cur_loc += op_size;
						cur_len += op_size;
						w->comp_type = op_type;
						asm_code_add_word(ac, w);
						if (++cur_count == op_count)
						{
							state = STATE_CODE_SEG_EOL;
						}
						break;
					}
					state = STATE_ERROR;
					errnr = 12; // Incompatible operand
					break;
				}
				if (cur_type == WORD_TYPE_ALLOCATOR)
				{
					if (lw != NULL)
					{
						errnr = 40; // Array size is expected
						state = STATE_ERROR;
						lw = NULL;
						break;
					}
					lw = w;
					break;
				}
				if (cur_type == WORD_TYPE_ARRAY_SIZE)
				{
					if (lw == NULL)
					{
						errnr = 6; //array size is allowed only after an allocator
						state = STATE_ERROR;
						break;
					}
					if (asm_lang_is_compatible(rw->value_type, op_type))
					{
						cur_loc += op_size;
						cur_len += op_size;
						w->comp_type = op_type;
						w->value.l *= asm_lang_datasize((int)lw->value.l);
						words_prefix_name(w, lw->name);
						asm_code_add_word(ac, w);
						lw = NULL;
						if (++cur_count == op_count)
						{
							state = STATE_CODE_SEG_EOL;
						}
						break;
					}
					lw = NULL;
					errnr = 12; // Incompatible operand
					state = STATE_ERROR;
					break;					
				}
				lw = NULL;
				state = STATE_ERROR;
				errnr = 7; // Literal, label, allocator or array size is expected
				break;
			case	STATE_CODE_ALLOC_ARRAY_SIZE:
				if (cur_type == WORD_TYPE_ARRAY_SIZE)
				{
					op_count = (int) rw->value.l;
					w->comp_type = VALUE_TYPE_NULL;
					asm_code_add_word(ac, w);
					state = STATE_CODE_ALLOC_OP;
					break;
				}
				// No break is allowed here
			case	STATE_CODE_ALLOC_OP:
				if (cur_type == WORD_TYPE_EOL)
				{
					if (cur_alloc == STRING_ALLOC)
					{
						errnr = 17; // Unexpected EOL
						state = STATE_ERROR;
						break;
					}
					cur_loc += (op_count - cur_count)  * op_size;
					cur_len += (op_count - cur_count)  * op_size;
					sentence_set_size(ac->cur_sentence,cur_len);
					state = STATE_CODE_SEG_LABEL_OR_INSTR;
					break;
				}
				if ((cur_type == WORD_TYPE_LITERAL) || (cur_type == WORD_TYPE_LABEL))
				{
					if (asm_lang_is_compatible(rw->value_type, op_type))
					{
						if (cur_alloc == STRING_ALLOC)
						{
							cur_loc += op_size * (strlen(w->value.s) + 1);
							cur_len += op_size * (strlen(w->value.s) + 1);
							state = STATE_CODE_SEG_EOL;
							break;
						}
						w->comp_type = op_type;
						asm_code_add_word(ac, w);
						cur_loc += op_size;
						cur_len += op_size;
						if (++cur_count == op_count)
							state = STATE_CODE_SEG_EOL;
						break;
					}
					state = STATE_ERROR;
					errnr = 12; // incompatible operand;
					break;
				}
				state = STATE_ERROR;
				errnr = 18; // Literal oe label is expected
				break;
			

//------------------------------------------------------------------------------
//					SEG END EOL
//------------------------------------------------------------------------------
			case	STATE_SEG_END_EOL:
				if (cur_type == WORD_TYPE_EOL)
				{
					sentence_set_size(ac->cur_sentence,cur_len);
					state = STATE_SEG_START;
				}
				else
				{
					state = STATE_ERROR;
					errnr = 4; // EOL is expected
				}
				break;
//------------------------------------------------------------------------------
//					DEFAULT ACTION
//------------------------------------------------------------------------------
			default:
				errnr = 21; // Unknown token
				state = STATE_ERROR;
				break;
		} // End of switch

		if (state == STATE_ERROR)
		{
			ret = -1;
			break; // breaks the loop
		}
		if (state == STATE_END)
		{
			ret = 0;
			break; // breaks the loop
		}
	} // end while(1)
	if (ret == -1)
	{
		ac->err.errnr = errnr;
		ac->err.lineno = scanner_lineno(ac->scan);
		if (scanner_get_line(ac->scan, inp_line, SCANNER_LINE_LENGTH + 1) != 0)
			inp_line[0] = 0;
		fprintf(stderr,"[line: %d] [error: %d] %s\n", ac->err.lineno - PROLOG_SIZE, errnr, error_description[errnr]);
		fprintf(stderr, "[source line:%d] %s\n", ac->err.lineno - PROLOG_SIZE, inp_line);
	}
	return ret;
}
int		asm_code_get_composite_size(void *asm_code)
{
	asm_code_t	*ac = (asm_code_t *) asm_code;
	int			comp_size;
	
	comp_size = ac->loc[META_SEG] + ac->loc[CODE_SEG] + ac->loc[DATA_SEG];
	return comp_size;
}

void	*asm_code_generate(void *asm_code)
{
	asm_code_t	*ac = (asm_code_t *) asm_code;
	void		*obj_code;
	void		*s, *iter;
	word_t		*w,*rw;
	int			cur_seg, cur_pos, obj_code_size/*,i*/;
	int			pos[SEGDEF_COUNT];
	
// check if all label has been defined
	s = objects_get_first(ac->sentence_set);
	while (s != NULL)
	{
		w = (word_t *) sentence_get_first_word(s,&iter);
		while (w != NULL)
		{
			if (w->root != NULL)
				rw = w->root;
			else
				rw = w;
			if (rw->valid == 0) break;
			w = (word_t *) sentence_get_next_word(s,&iter);
		}
		if (w != NULL) break;
		s = objects_get_next(ac->sentence_set,s);
	}
	if (s != NULL)
	{
		ac->err.errnr = 22;
		ac->err.lineno = sentence_get_lineno(s);
		fprintf(stderr,"[line: %d] [error: %d] %s\n", ac->err.lineno - PROLOG_SIZE, ac->err.errnr, error_description[ac->err.errnr]);
		fprintf(stderr,"label = %s\n", rw->name);
		w = (word_t *) sentence_get_first_word(s,&iter);
		while (w != NULL)
		{
			if (w->root != NULL)
				rw = w->root;
			else
				rw = w;
			fprintf(stderr,"token=%s valid=%d\n",rw->name, rw->valid);
			w = (word_t *) sentence_get_next_word(s,&iter);
		}
		return NULL;
	}
	
	cur_seg = END_SEG;
	cur_pos = 0;
	ac->err.errnr = ac->err.lineno = 0;
	obj_code_size = ac->loc[META_SEG] + ac->loc[CODE_SEG] + ac->loc[DATA_SEG];
	pos[META_SEG] = 0;
	pos[CODE_SEG] = pos[META_SEG] + ac->loc[META_SEG];
	pos[DATA_SEG] = pos[CODE_SEG] + ac->loc[CODE_SEG];
//	fprintf(stderr,"loc[META_SEG]=%d\n",ac->loc[META_SEG]);
//	fprintf(stderr,"loc[CODE_SEG]=%d\n",ac->loc[CODE_SEG]);
//	fprintf(stderr,"loc[DATA_SEG]=%d\n",ac->loc[DATA_SEG]);
//	fprintf(stderr,"loc[CONTEXT_SEG]=%d\n",ac->loc[CONTEXT_SEG]);
	fprintf(stderr,"object code size:\t%d\n", obj_code_size);
	fprintf(stderr,"Meta offset:\t\t%d\n", pos[META_SEG]);
	fprintf(stderr,"Code offset:\t\t%d\n", pos[CODE_SEG]);
	fprintf(stderr,"Data offset:\t\t%d\n", pos[DATA_SEG]);
	if ((obj_code = malloc(obj_code_size)) != NULL)
	{
		memset(obj_code, 0, obj_code_size);
		s = objects_get_first(ac->sentence_set);
		while (s != NULL)
		{
			w = (word_t *) sentence_get_first_word(s,&iter);
			if (w->root != NULL)
				rw = w->root;
			else
				rw = w;
			ac->err.lineno = sentence_get_lineno(s);
			if ((w->type == WORD_TYPE_SEGDEF) || (w->type == WORD_TYPE_SEG_END))
			{
				if(rw->value.l == END_SEG)
				{
					pos[cur_seg] = cur_pos;
					cur_seg = END_SEG;
				}
				else
				{
					cur_seg = (int) rw->value.l;
					cur_pos = pos[cur_seg];
				}
			}
			else
			{
				if (cur_seg != CONTEXT_SEG)
				{
					if (asm_lang_compile_sentence(obj_code + cur_pos , obj_code_size - cur_pos, s) < 0)
					{
						fprintf(stderr,"debug: asm_lang_compile_sentence() returned negative value\n");
						ac->err.errnr = 32;
						break;
					}
//					fprintf(stderr,"%04X ",cur_pos);
//					i = 0;
//					while (i<sentence_get_size(s))
//					{
//						fprintf(stderr,"%02X ",(int)(*((char *)(obj_code+cur_pos+i++)))&255);
//					}
//					while (w != NULL)
//					{
//						fprintf(stderr,"%s ", w->name);
//						w = (word_t *) sentence_get_next_word(s,&iter);
//					}
//					fprintf(stderr,"\n");
				}
				cur_pos += sentence_get_size(s);
			}
			s = objects_get_next(ac->sentence_set,s);
		} // end of while (s != NULL)
	}
	else
	{
		ac->err.errnr = 28;
	} // end of if ((buf = malloc(obj_code_size)) != NULL)
	if (ac->err.errnr != 0)
	{
		fprintf(stderr,"[line: %d] [error: %d] %s\n", ac->err.lineno -PROLOG_SIZE, ac->err.errnr, error_description[ ac->err.errnr]);
		
		if (obj_code != NULL)
		{
			free(obj_code);
			obj_code = NULL;
		}
	}
	return obj_code;
}

void asm_code_print(void *asm_code, void *out)
{
	asm_code_t	*ac= (asm_code_t *) asm_code;
	void			*s, *iter;
	word_t			*w;
	char			line[COMMON_LINE_LENGTH + 1];
	int				pos = 0;
	int				op_count = 0;
	int				i,j;
	
	if (ac == NULL) return;
	s = objects_get_first( ac->sentence_set);
	while (s != NULL)
	{
		op_count = 0;
		sprintf(&line[pos], "[%03d]   ", sentence_get_lineno(s) - PROLOG_SIZE);
		pos = strlen(line);
		w = (word_t *) sentence_get_first_word(s,&iter);
		if (w->type != WORD_TYPE_LABEL)
		{
			for (i = pos; i < 40; i++)
				line[i] = ' ';
//			sprintf(&line[pos], "                ");
			line[40] = 0;
			pos = 40;
		}
		else
		{
			sprintf(&line[pos], "%s", w->name);
			j = strlen(line);
			for (i = j; i < 40; i++)
				line[i] = ' ';
			line[40] = 0;
			pos = 40;
			w = (word_t *) sentence_get_next_word(s,&iter);
		}
		if (w != NULL)
		{
			sprintf(&line[pos], "%s", w->name);
			j = strlen(line);
			for (i = j; i < 56; i++)
				line[i] = ' ';
			line[56] = 0;
			pos = 56;
			w = (word_t *) sentence_get_next_word(s,&iter);
			while (w != NULL) // operands
			{
				if (op_count == 1)
				{
					line[pos++] = '\\';
					line[pos++] = '\n';
					io_write(out, &line[0],pos);
					sprintf(&line[0], "[%03d]   ", sentence_get_lineno(s) - PROLOG_SIZE);
					pos = strlen(line);
					for (i = pos; i < 56; i++)
						line[i] = ' ';
					line[56] = 0;
					pos = 56;
					op_count = 0;
				}
				if (w->value_type == VALUE_TYPE_STRING)
					sprintf(&line[pos], "\"%s\"",w->name);
				else
					sprintf(&line[pos], "%s", w->name);
				pos = strlen(line);
//				j = strlen(line);
//				for (i = j; i < (56 + op_count * 16); i++)
//					line[i] = ' ';
//				line[56 + op_count * 16] = 0;
//				pos = 56 + op_count * 16;
				w = (word_t *) sentence_get_next_word(s,&iter);
				op_count = 1;
			}
		}
		if (pos > 0)
		{
			line[pos++] = '\n';
			io_write(out, &line[0],pos);
			pos = 0;
			op_count = 0;
		}
		s = objects_get_next( ac->sentence_set,s);
	}
}

void asm_code_list(void *asm_code, void *out)
{
	asm_code_t	*ac= (asm_code_t *) asm_code;
	void			*s, *iter;
	word_t			*w,*rw;
	int				cur_seg,cur_pos,cur_len, sentence_pos;
	int				i,j,k,first;
	int				pos[SEGDEF_COUNT];
	unsigned char	buf[64];
	char			line[COMMON_LINE_LENGTH + 1];
	
	if (ac == NULL) return;
	s = objects_get_first( ac->sentence_set);
	pos[META_SEG] = pos[CODE_SEG] = pos[DATA_SEG] = pos[CONTEXT_SEG] = pos[END_SEG] = 0;
	cur_pos = 0;
	sentence_pos = 0;
	cur_seg = END_SEG;
	while(s != NULL)
	{
		cur_pos = sentence_pos;
		w = (word_t *) sentence_get_first_word(s,&iter);
		if (w->root != NULL)
			rw = w->root;
		else
			rw = w;
		if ((w->type == WORD_TYPE_SEGDEF) || (w->type == WORD_TYPE_SEG_END))
		{
			if(rw->value.l == END_SEG)
			{
				pos[cur_seg] = cur_pos;
				cur_seg = END_SEG;
			}
			else
			{
				cur_seg = (int) rw->value.l;
				cur_pos = pos[cur_seg];
				sentence_pos = cur_pos;
			}
			sprintf(&line[0],"%01X %04X                          %s\n",cur_seg,cur_pos,w->name);
			io_write(out, &line[0], strlen(line));
		}
		else
		{
			while (w != NULL)
			{
				memset(buf,0,64);
				cur_len = asm_lang_compile_word(buf,64,w);
				if (cur_len == 0)
				{
					sprintf(&line[0],"%01X %04X                          %s\n",cur_seg, cur_pos, w->name);
					io_write(out, &line[0], strlen(line));
				}
				else
				{
					first = -1;
					i = 0;
					while (i < cur_len)
					{
						sprintf(&line[0],"%01X %04X  ", cur_seg, cur_pos + i);
						j = (cur_len - i) > 8 ? 8 : cur_len - i;
						for (k = 0; k < j; k++)
							sprintf(&line[strlen(line)],"%02X ",(unsigned int)(*(buf + i + k)));
						for (k = 0; k < 8 - j; k++)
							sprintf(&line[strlen(line)],"   ");
						if (first != 0)
						{
							if (w->value_type == VALUE_TYPE_STRING)
								sprintf(&line[strlen(line)],"\"%s\"\n",w->name);
							else
								sprintf(&line[strlen(line)],"%s\n",w->name);
						}
						else
							sprintf(&line[strlen(line)],"\n");
						io_write(out, &line[0], strlen(line));
						first = 0;
						i += j;
					}
				}
				cur_pos += cur_len;
				w = (word_t *) sentence_get_next_word(s,&iter);
				if (w != NULL)
				{
					if (w->root != NULL)
						rw = w->root;
					else
						rw = w;
				}
			}
		}
		sentence_pos += sentence_get_size(s);
		s = objects_get_next( ac->sentence_set,s);
	}
}
