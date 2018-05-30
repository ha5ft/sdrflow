/*******************************************************************************
 * 							Common src_lang
 * *****************************************************************************
 * 	Filename:		src_lang.c
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
#include	"config.h"
#include	"objects.h"
#include	"src_lang.h"
#include	"allocators.h"
#include	"instructions.h"
#include	"labels.h"
#include	"obj_format.h"
#include	"segments.h"
#include	"tokens.h"
#include	"value.h"
#include	"words.h"
#include	"hash_table.h"

#define		RESERVED_NAME_COUNT		(INSTR_COUNT - 3 + SEGDEF_COUNT + LABEL_COUNT)

typedef struct	_alloc_table_element
{
	int		code;
	char	*name;
	int		data_type;
} alloc_table_element_t;

typedef struct	_token_table_element
{
	int		code;
	char	*name;
	int		word_type;
} token_table_element_t;

static char	*reserved_name_table[RESERVED_NAME_COUNT] =
{
	EXIT_STR,
	PAUSE_STR,
	ENDCYCLE_STR,
	BREAK_STR,
	GOTO_STR,
	FIRE_STR,
	INIT_STR,
	CLEANUP_STR,
	RET_STR,
	PRIM_FIRE_STR,
	PRIM_INIT_STR,
	PRIM_CLEANUP_STR,
	CPPTR_STR,
	INCPTR_STR,
	CPCTXPTR_STR,
	META_SEG_STR,
	CODE_SEG_STR,
	DATA_SEG_STR,
	CONTEXT_SEG_STR,
	END_SEG_STR,
	LABEL_CODE_OFFS_STR,
	LABEL_DATA_OFFS_STR,
	LABEL_COMP_SIZE_STR,
	LABEL_COMP_NAME_STR,
	LABEL_COMP_VERS_STR,
	LABEL_CODE_INIT_STR,
	LABEL_CODE_FIRE_STR,
	LABEL_CODE_CLEANUP_STR,
	LABEL_META_LOAD_STR,
	LABEL_META_MAKE_STR,
	LABEL_META_DELETE_STR,
	LABEL_CONTEXT_SIZE_STR,
	META_EXIT_STR,
	META_LDCOMP_STR,
	META_LDPRIM_STR,
	META_MKBUF_STR,
	META_MKCOMPINST_STR,
	META_MKPRIMINST_STR,
	META_DELBUF_STR
};

static alloc_table_element_t alloc_table[ALLOC_COUNT] =
{
	[NULL_ALLOC] = {NULL_ALLOC, NULL_ALLOC_STR, VALUE_TYPE_NULL},
	[CHAR_ALLOC] = {CHAR_ALLOC, CHAR_ALLOC_STR, VALUE_TYPE_CHAR},
	[UCHAR_ALLOC] = {UCHAR_ALLOC, UCHAR_ALLOC_STR, VALUE_TYPE_UCHAR},
	[SHORT_ALLOC] = {SHORT_ALLOC, SHORT_ALLOC_STR, VALUE_TYPE_SHORT},
	[USHORT_ALLOC] = {USHORT_ALLOC, USHORT_ALLOC_STR, VALUE_TYPE_USHORT},
	[INT_ALLOC] = {INT_ALLOC, INT_ALLOC_STR, VALUE_TYPE_INT},
	[UINT_ALLOC] = {UINT_ALLOC, UINT_ALLOC_STR, VALUE_TYPE_UINT},
	[LONG_ALLOC] = {LONG_ALLOC, LONG_ALLOC_STR, VALUE_TYPE_LONG},
	[ULONG_ALLOC] = {ULONG_ALLOC, ULONG_ALLOC_STR, VALUE_TYPE_ULONG},
	[FLOAT_ALLOC] = {FLOAT_ALLOC, FLOAT_ALLOC_STR, VALUE_TYPE_FLOAT},
	[DOUBLE_ALLOC] = {DOUBLE_ALLOC, DOUBLE_ALLOC_STR, VALUE_TYPE_DOUBLE},
	[PTR_ALLOC] = {PTR_ALLOC, PTR_ALLOC_STR, VALUE_TYPE_PTR},
	[STRING_ALLOC] = {STRING_ALLOC, STRING_ALLOC_STR, VALUE_TYPE_STRING},
	[ADDR_ALLOC] = {ADDR_ALLOC, ADDR_ALLOC_STR, VALUE_TYPE_ADDR},
	[OFFSET_ALLOC] = {OFFSET_ALLOC, OFFSET_ALLOC_STR, VALUE_TYPE_OFFSET},
	[CHARARR_ALLOC] = {CHARARR_ALLOC, CHARARR_ALLOC_STR, VALUE_TYPE_CHARARR}
};

static token_table_element_t	token_table[TOKEN_COUNT] =
{
	[TOKEN_USE] = {TOKEN_USE, TOKEN_USE_STR, WORD_TYPE_USE},
	[TOKEN_PRIMITIVE] = {TOKEN_PRIMITIVE, TOKEN_PRIMITIVE_STR, WORD_TYPE_PRIMITIVE},
	[TOKEN_COMPOSITE] = {TOKEN_COMPOSITE, TOKEN_COMPOSITE_STR, WORD_TYPE_COMPOSITE},
	[TOKEN_IMPL] = {TOKEN_IMPL, TOKEN_IMPL_STR, WORD_TYPE_IMPL},
	[TOKEN_STREAM] = {TOKEN_STREAM, TOKEN_STREAM_STR, WORD_TYPE_STREAM},
	[TOKEN_VAR] = {TOKEN_VAR, TOKEN_VAR_STR, WORD_TYPE_VAR},
	[TOKEN_CONST] = {TOKEN_CONST, TOKEN_CONST_STR, WORD_TYPE_CONST},
	[TOKEN_INPUT] = {TOKEN_INPUT, TOKEN_INPUT_STR, WORD_TYPE_INPUT},
	[TOKEN_OUTPUT] = {TOKEN_OUTPUT, TOKEN_OUTPUT_STR, WORD_TYPE_OUTPUT},
	[TOKEN_PARAM] = {TOKEN_PARAM, TOKEN_PARAM_STR, WORD_TYPE_PARAM},
	[TOKEN_STATE] = {TOKEN_STATE, TOKEN_STATE_STR, WORD_TYPE_STATE},
	[TOKEN_TEMP] = {TOKEN_TEMP, TOKEN_TEMP_STR, WORD_TYPE_TEMP},
	[TOKEN_ACTORS] = {TOKEN_ACTORS, TOKEN_ACTORS_STR, WORD_TYPE_ACTORS},
	[TOKEN_SCHEDULE] = {TOKEN_SCHEDULE, TOKEN_SCHEDULE_STR, WORD_TYPE_SCHEDULE}, 
	[TOKEN_DO] = {TOKEN_DO, TOKEN_DO_STR, WORD_TYPE_DO},
	[TOKEN_LOOP] = {TOKEN_LOOP, TOKEN_LOOP_STR, WORD_TYPE_LOOP},
	[TOKEN_END] = {TOKEN_END, TOKEN_END_STR, WORD_TYPE_END},
	[TOKEN_SIGNALS] = {TOKEN_SIGNALS, TOKEN_SIGNALS_STR, WORD_TYPE_SIGNALS},
	[TOKEN_TOPOLOGY] = {TOKEN_TOPOLOGY, TOKEN_TOPOLOGY_STR, WORD_TYPE_TOPOLOGY},
	[TOKEN_CONTEXT] = {TOKEN_CONTEXT, TOKEN_CONTEXT_STR, WORD_TYPE_CONTEXT},
	[TOKEN_MANUAL] = {TOKEN_MANUAL, TOKEN_MANUAL_STR, WORD_TYPE_MANUAL},
	[TOKEN_AUTO] = {TOKEN_AUTO, TOKEN_AUTO_STR, WORD_TYPE_AUTO}
};

static int	alloc_size[SDF_ARCHITECTURE_COUNT][VALUE_TYPE_COUNT] =
{
	[SDF_ARCHITECTURE_64] =
	{
		[VALUE_TYPE_NULL] = VALUE_NULL_SIZE_64,
		[VALUE_TYPE_CHAR] = VALUE_CHAR_SIZE_64,
		[VALUE_TYPE_UCHAR] = VALUE_UCHAR_SIZE_64,
		[VALUE_TYPE_SHORT] = VALUE_SHORT_SIZE_64,
		[VALUE_TYPE_USHORT] = VALUE_USHORT_SIZE_64,
		[VALUE_TYPE_INT] = VALUE_INT_SIZE_64,
		[VALUE_TYPE_UINT] = VALUE_UINT_SIZE_64,
		[VALUE_TYPE_LONG] = VALUE_LONG_SIZE_64,
		[VALUE_TYPE_ULONG] = VALUE_ULONG_SIZE_64,
		[VALUE_TYPE_FLOAT] = VALUE_FLOAT_SIZE_64,
		[VALUE_TYPE_DOUBLE] = VALUE_DOUBLE_SIZE_64,
		[VALUE_TYPE_PTR] = VALUE_PTR_SIZE_64,
		[VALUE_TYPE_STRING] = VALUE_STRING_SIZE_64,
		[VALUE_TYPE_ADDR] = VALUE_ADDR_SIZE_64,
		[VALUE_TYPE_OFFSET] = VALUE_OFFSET_SIZE_64,
		[VALUE_TYPE_CHARARR] = VALUE_CHARARR_SIZE_64
	},
	[SDF_ARCHITECTURE_32] =
	{
		[VALUE_TYPE_NULL] = VALUE_NULL_SIZE_32,
		[VALUE_TYPE_CHAR] = VALUE_CHAR_SIZE_32,
		[VALUE_TYPE_UCHAR] = VALUE_UCHAR_SIZE_32,
		[VALUE_TYPE_SHORT] = VALUE_SHORT_SIZE_32,
		[VALUE_TYPE_USHORT] = VALUE_USHORT_SIZE_32,
		[VALUE_TYPE_INT] = VALUE_INT_SIZE_32,
		[VALUE_TYPE_UINT] = VALUE_UINT_SIZE_32,
		[VALUE_TYPE_LONG] = VALUE_LONG_SIZE_32,
		[VALUE_TYPE_ULONG] = VALUE_ULONG_SIZE_32,
		[VALUE_TYPE_FLOAT] = VALUE_FLOAT_SIZE_32,
		[VALUE_TYPE_DOUBLE] = VALUE_DOUBLE_SIZE_32,
		[VALUE_TYPE_PTR] = VALUE_PTR_SIZE_32,
		[VALUE_TYPE_STRING] = VALUE_STRING_SIZE_32,
		[VALUE_TYPE_ADDR] = VALUE_ADDR_SIZE_32,
		[VALUE_TYPE_OFFSET] = VALUE_OFFSET_SIZE_32,
		[VALUE_TYPE_CHARARR] = VALUE_CHARARR_SIZE_32
	},
	[SDF_ARCHITECTURE_16] =
	{
		[VALUE_TYPE_NULL] = VALUE_NULL_SIZE_16,
		[VALUE_TYPE_CHAR] = VALUE_CHAR_SIZE_16,
		[VALUE_TYPE_UCHAR] = VALUE_UCHAR_SIZE_16,
		[VALUE_TYPE_SHORT] = VALUE_SHORT_SIZE_16,
		[VALUE_TYPE_USHORT] = VALUE_USHORT_SIZE_16,
		[VALUE_TYPE_INT] = VALUE_INT_SIZE_16,
		[VALUE_TYPE_UINT] = VALUE_UINT_SIZE_16,
		[VALUE_TYPE_LONG] = VALUE_LONG_SIZE_16,
		[VALUE_TYPE_ULONG] = VALUE_ULONG_SIZE_16,
		[VALUE_TYPE_FLOAT] = VALUE_FLOAT_SIZE_16,
		[VALUE_TYPE_DOUBLE] = VALUE_DOUBLE_SIZE_16,
		[VALUE_TYPE_PTR] = VALUE_PTR_SIZE_16,
		[VALUE_TYPE_STRING] = VALUE_STRING_SIZE_16,
		[VALUE_TYPE_ADDR] = VALUE_ADDR_SIZE_16,
		[VALUE_TYPE_OFFSET] = VALUE_OFFSET_SIZE_16,
		[VALUE_TYPE_CHARARR] = VALUE_CHARARR_SIZE_16
	},
};

static char *asm_line[ASM_CODE_COUNT] =
{
	[L_META_SEG] = "\t" META_SEG_STR,
	[L_COMP_NAME] = LABEL_COMP_NAME_STR "\t" STRING_ALLOC_STR "\t\"%s\"",
	[L_COMP_VER] = ".version\tuint\t%08X",
	[L_DEF_NAME] = "%s.n\tstring\t\"%s\"",
	[L_LOAD_LAB] = ".load",
	[L_LOAD_PRIM] = "\tld.prim\t%s.n",
//	[L_LOAD_PRIM] = "\tld.prim\t%s.n %s.n",
	[L_META_EXIT] = "\tmeta.exit",
	[L_MAKE_LAB] = ".make",
	[L_MAKE_BUF] = "\tmk.buffer\t%s[%d] %s.n %s.p",
	[L_MAKE_PRIM] = "\tmk.prim.inst\t%s.n %s.n %s",
	[L_MAKE_COMP] = "\tmk.comp.inst\t%s.n %s.n %s",
	[L_DEL_LAB] = ".delete",
	[L_DEL_BUF] = "\tdel.buf\t%s",
	[L_ENDSEG] = "\t.endseg",
	[L_CODE_SEG] = "\t.code",
	[L_EXIT] = "\texit",
	[L_INIT_LAB] = ".init",
	[L_CP_PTR] = "\tcp.ptr\t%s %s.p",
	[L_INIT_PRIM] = "\tinit.prim\t%s",
	[L_INIT_COMP] = "\tinit.comp\t%s",
	[L_RET] = "\tret",
	[L_END_CYCLE] = "\tend.cycle",
	[L_FIRE_LAB] = ".fire",
	[L_DO] = "\tdo\t%d",
	[L_LOOP_LAB] = ".l%d",
	[L_FIRE_PRIM] = "\tfire.prim\t%s",
	[L_FIRE_COMP] = "\tfire.comp\t%s",
	[L_PADD_LINT] = "\tinc.ptr\t%s %d",
	[L_LOOP] = "\tloop\t.l%d",
	[L_CLEAN_PRIM] = "\tcleanup.prim\t%s",
	[L_CLEAN_COMP] = "\tcleanup.comp\t%s",
	[L_CP_CTX_PTR] = "\tcp.ctx.ptr\t%s %s",
	[L_CLEAN_LAB] = ".clean",
	[L_DATA_SEG] = "\t.data",
	[L_DEF_PTR] = "%s.p\tptr\t%s",
	[L_DEF_SIG] = "%s\t%s[%d]",
	[L_DEF_ACT] = "%s\tptr",
	[L_LOAD_COMP] = "\tld.comp\t%s.n",
//	[L_LOAD_COMP] = "\tld.comp\t%s.n %s.n",
	[L_CONTEXT_SEG] = "\t.context",
	[L_DEF_PORT_PTR] = "%s\tptr",
	[40] = ""
};

static int	architecture = SDF_ARCHITECTURE_64;

/* ************************** Externally accessible functions *****************/

int	src_lang_init_hash_table(void *hash_table)
{
	int		i;
	word_t	*w;
	
	for (i = 0; i < TOKEN_COUNT; i++)
	{
		if ((w = hash_table_lookup(hash_table, token_table[i].name)) == NULL) return -1;
		w->type = token_table[i].word_type;
		w->root = NULL;
		w->comp_type = VALUE_TYPE_NULL;
		w->value_type = VALUE_TYPE_LONG;
		w->value.l = (long) token_table[i].code;
		w->valid = -1;
	}
	for (i = 1; i < ALLOC_COUNT; i++)
	{
		if ((w = hash_table_lookup(hash_table, alloc_table[i].name)) == NULL) return -1;
		w->type = WORD_TYPE_ALLOCATOR;
		w->root = NULL;
		w->comp_type = VALUE_TYPE_NULL;
		w->value_type = VALUE_TYPE_LONG;
		w->value.l = (long) alloc_table[i].code;
		w->valid = -1;
	}
	for (i = 0; i < RESERVED_NAME_COUNT; i++)
	{
		if ((w = hash_table_lookup(hash_table, reserved_name_table[i])) == NULL) return -1;
		w->type = WORD_TYPE_RESERVED_NAME;
		w->root = NULL;
		w->comp_type = VALUE_TYPE_NULL;
		w->value_type = VALUE_TYPE_NULL;
		w->value.l = (long) 0;
		w->valid = -1;
	}
	return 0;
}

word_t *src_lang_new_do_token(void *set, int cycle_count)
{
	int	i;
	word_t	*w;
	
	w = words_new(set);
	if (w == NULL) return NULL;
	i = TOKEN_DO;
	words_store_name(w,token_table[i].name);
	w->type = token_table[i].word_type;
	w->value_type = VALUE_TYPE_PTR;
	w->value.p = NULL;
	w->size = cycle_count;
	return w;
}

word_t *src_lang_new_loop_token(void *set, void *last_do)
{
	int	i;
	word_t	*w;
	
	w = words_new(set);
	if (w == NULL) return NULL;
	i = TOKEN_LOOP;
	words_store_name(w,token_table[i].name);
	w->type = token_table[i].word_type;
	w->value_type = VALUE_TYPE_PTR;
	w->value.p = last_do;
	return w;
}

char *src_lang_get_alloc_name(unsigned int code)
{
	if (code < ALLOC_COUNT) return alloc_table[code].name;
	else return "";
}

int	src_lang_get_alloc_size(unsigned int code)
{
	return alloc_size[architecture][code];
}

char	*src_lang_get_asm_line(unsigned int index)
{
	if (index < ASM_CODE_COUNT)
		return asm_line[index];
	else
		return NULL;
}
