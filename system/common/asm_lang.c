/*******************************************************************************
 * 							Common asm_lang
 * *****************************************************************************
 * 	Filename:		asm_lang.c
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
#include	<stdio.h>
#include	<string.h>
#include	"config.h"
#include	"value.h"
#include	"allocators.h"
#include	"instructions.h"
#include	"labels.h"
#include	"segments.h"
#include	"obj_format.h"
#include	"asm_lang.h"
#include	"words.h"
#include	"hash_table.h"
#include	"sentence.h"

/*********************** Local types *****************************/

typedef struct _instr_table_element
{
	int		type;
	int		code;
	char	*name;
	int		op_count;
	int		op_type[4];
} instr_table_element_t;

typedef struct _segdef_table_element
{
	int		code;
	char	*name;
} segdef_table_element_t;

typedef struct _alloc_table_element
{
	int		code;
	char	*name;
	int		op_type;
} alloc_table_element_t;

typedef struct _data_table_element
{
	int		size;
	long	min;
	long	max;
} data_table_element_t;

typedef int compatibility_t[VALUE_TYPE_COUNT][VALUE_TYPE_COUNT];

typedef struct	_label_table_element
{
	int		code;
	char	*name;
	int		value_type;
} label_table_element_t;

typedef struct _architecture
{
	instr_table_element_t	*instr_tab;
	segdef_table_element_t	*segdef_tab;
	alloc_table_element_t	*alloc_tab;
	data_table_element_t	*data_tab;
	label_table_element_t	*label_tab;
	compatibility_t			*comp_tab;
	int						*instr_size;
}architecture_t;

/*********************** Local variables *****************************/

// architecture should be a formal parameter to the correcponding functions
static int	architecture = SDF_ARCHITECTURE_64; //target architecture

static instr_table_element_t	instr_table[INSTR_COUNT] =
{
	[EXIT_INSTR] = {WORD_TYPE_INSTRUCTION, EXIT_INSTR, EXIT_STR, EXIT_OP_COUNT, {VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[PAUSE_INSTR] = {WORD_TYPE_INSTRUCTION, PAUSE_INSTR, PAUSE_STR, PAUSE_OP_COUNT, {VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[END_INSTR] = {WORD_TYPE_INSTRUCTION, END_INSTR, END_STR, END_OP_COUNT, {VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[ENDCYCLE_INSTR] = {WORD_TYPE_INSTRUCTION, ENDCYCLE_INSTR, ENDCYCLE_STR, ENDCYCLE_OP_COUNT, {VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[BREAK_INSTR] = {WORD_TYPE_INSTRUCTION, BREAK_INSTR, BREAK_STR, BREAK_OP_COUNT, {VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[GOTO_INSTR] = {WORD_TYPE_INSTRUCTION, GOTO_INSTR, GOTO_STR, GOTO_OP_COUNT, {VALUE_TYPE_OFFSET, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[DO_INSTR] = {WORD_TYPE_INSTRUCTION, DO_INSTR, DO_STR, DO_OP_COUNT, {VALUE_TYPE_INT, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[LOOP_INSTR] = {WORD_TYPE_INSTRUCTION, LOOP_INSTR, LOOP_STR, LOOP_OP_COUNT, {VALUE_TYPE_OFFSET, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[FIRE_INSTR] = {WORD_TYPE_INSTRUCTION, FIRE_INSTR, FIRE_STR, FIRE_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[INIT_INSTR] = {WORD_TYPE_INSTRUCTION, INIT_INSTR, INIT_STR, INIT_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[CLEANUP_INSTR] = {WORD_TYPE_INSTRUCTION, CLEANUP_INSTR, CLEANUP_STR, CLEANUP_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[RET_INSTR] = {WORD_TYPE_INSTRUCTION, RET_INSTR, RET_STR, RET_OP_COUNT, {VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[PRIM_FIRE_INSTR] = {WORD_TYPE_INSTRUCTION, PRIM_FIRE_INSTR, PRIM_FIRE_STR, PRIM_FIRE_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[PRIM_INIT_INSTR] = {WORD_TYPE_INSTRUCTION, PRIM_INIT_INSTR, PRIM_INIT_STR, PRIM_INIT_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[PRIM_CLEANUP_INSTR] = {WORD_TYPE_INSTRUCTION, PRIM_CLEANUP_INSTR, PRIM_CLEANUP_STR, PRIM_CLEANUP_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[CPPTR_INSTR] = {WORD_TYPE_INSTRUCTION, CPPTR_INSTR, CPPTR_STR, CPPTR_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[INCPTR_INSTR] = {WORD_TYPE_INSTRUCTION, INCPTR_INSTR, INCPTR_STR, INCPTR_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_INT, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[CPCTXPTR_INSTR] = {WORD_TYPE_INSTRUCTION, CPCTXPTR_INSTR, CPCTXPTR_STR, CPCTXPTR_OP_COUNT, {VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[META_EXIT_INSTR] = {WORD_TYPE_META, META_EXIT_INSTR, META_EXIT_STR, META_EXIT_OP_COUNT,{VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[META_LDCOMP_INSTR] = {WORD_TYPE_META, META_LDCOMP_INSTR, META_LDCOMP_STR, META_LDCOMP_OP_COUNT,{VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[META_LDPRIM_INSTR] = {WORD_TYPE_META, META_LDPRIM_INSTR, META_LDPRIM_STR, META_LDPRIM_OP_COUNT,{VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL}},
	[META_MKBUF_INSTR] = {WORD_TYPE_META, META_MKBUF_INSTR, META_MKBUF_STR, META_MKBUF_OP_COUNT,{VALUE_TYPE_INT, VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_NULL}},
	[META_MKCOMPINST_INSTR] = {WORD_TYPE_META, META_MKCOMPINST_INSTR, META_MKCOMPINST_STR, META_MKCOMPINST_OP_COUNT,{VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_NULL}},
	[META_MKPRIMINST_INSTR] = {WORD_TYPE_META, META_MKPRIMINST_INSTR, META_MKPRIMINST_STR, META_MKPRIMINST_OP_COUNT,{VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_ADDR, VALUE_TYPE_NULL}},
	[META_DELBUF_INSTR] = {WORD_TYPE_META, META_DELBUF_INSTR, META_DELBUF_STR, META_DELBUF_OP_COUNT,{VALUE_TYPE_ADDR, VALUE_TYPE_NULL, VALUE_TYPE_NULL, VALUE_TYPE_NULL}}
};

static segdef_table_element_t	segdef_table[SEGDEF_COUNT] =
{
	[META_SEG] = {META_SEG, META_SEG_STR},
	[CODE_SEG] = {CODE_SEG, CODE_SEG_STR},
	[DATA_SEG] = {DATA_SEG, DATA_SEG_STR},
	[CONTEXT_SEG] = {CONTEXT_SEG, CONTEXT_SEG_STR},
	[END_SEG] = {END_SEG, END_SEG_STR}
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

static label_table_element_t	label_table[LABEL_COUNT] =
{
	[LABEL_CODE_OFFSET] = {.code = LABEL_CODE_OFFSET, .name = LABEL_CODE_OFFS_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_DATA_OFFSET] = {.code = LABEL_DATA_OFFSET, .name = LABEL_DATA_OFFS_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_COMP_SIZE] = {.code = LABEL_COMP_SIZE, .name = LABEL_COMP_SIZE_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_COMP_NAME] = {.code = LABEL_COMP_NAME, .name = LABEL_COMP_NAME_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_COMP_VERS] = {.code = LABEL_COMP_VERS, .name = LABEL_COMP_VERS_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_CODE_INIT] = {.code = LABEL_CODE_INIT, .name = LABEL_CODE_INIT_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_CODE_FIRE] = {.code = LABEL_CODE_FIRE, .name = LABEL_CODE_FIRE_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_CODE_CLEANUP] = {.code = LABEL_CODE_CLEANUP, .name = LABEL_CODE_CLEANUP_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_META_LOAD] = {.code = LABEL_META_LOAD, .name = LABEL_META_LOAD_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_META_MAKE] = {.code = LABEL_META_MAKE, .name = LABEL_META_MAKE_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_META_DELETE] = {.code = LABEL_META_DELETE, .name = LABEL_META_DELETE_STR, .value_type = VALUE_TYPE_ADDR},
	[LABEL_CONTEXT_SIZE] = {.code = LABEL_CONTEXT_SIZE, .name = LABEL_CONTEXT_SIZE_STR, .value_type = VALUE_TYPE_ADDR}
};

static data_table_element_t	data_table_64[VALUE_TYPE_COUNT] =
{
	[VALUE_TYPE_NULL] = {.size = 0, .min = 0, .max = 0},
	[VALUE_TYPE_CHAR] = {.size = 1, .min = VALUE_INT8_MIN, .max = VALUE_INT8_MAX},
	[VALUE_TYPE_UCHAR] = {.size = 1, .min = VALUE_UINT8_MIN, .max = VALUE_UINT8_MAX},
	[VALUE_TYPE_SHORT] = {.size = 2, .min = VALUE_INT16_MIN, .max = VALUE_INT16_MAX},
	[VALUE_TYPE_USHORT] = {.size = 2, .min = VALUE_UINT16_MIN, .max = VALUE_UINT16_MAX},
	[VALUE_TYPE_INT] = {.size = 4, .min = VALUE_INT32_MIN, .max = VALUE_INT32_MAX},
	[VALUE_TYPE_UINT] = {.size = 4, .min = VALUE_UINT32_MIN, .max = VALUE_UINT32_MAX},
	[VALUE_TYPE_LONG] = {.size = 8, .min = VALUE_INT64_MIN, .max = VALUE_INT64_MAX},
	[VALUE_TYPE_ULONG] = {.size = 8, .min = VALUE_UINT64_MIN, .max = VALUE_UINT64_MAX},
	[VALUE_TYPE_FLOAT] = {.size = 4, .min = 0, .max = 0},
	[VALUE_TYPE_DOUBLE] = {.size = 8, .min = 0, .max = 0},
	[VALUE_TYPE_PTR] = {.size = 8, .min = VALUE_UINT32_MIN, .max = VALUE_UINT32_MAX},
	[VALUE_TYPE_STRING] = {.size = 1, .min = 0, .max = 0},
	[VALUE_TYPE_ADDR] = {.size = 4, .min = VALUE_UINT32_MIN, .max = VALUE_UINT32_MAX},
	[VALUE_TYPE_OFFSET] = {.size = 4, .min = VALUE_INT32_MIN, .max = VALUE_INT32_MAX},
	[VALUE_TYPE_CHARARR] = {.size = 1, .min = 0, .max = 0}
};

static data_table_element_t	data_table_32[VALUE_TYPE_COUNT] =
{
	[VALUE_TYPE_NULL] = {.size = 0, .min = 0, .max = 0},
	[VALUE_TYPE_CHAR] = {.size = 1, .min = VALUE_INT8_MIN, .max = VALUE_INT8_MAX},
	[VALUE_TYPE_UCHAR] = {.size = 1, .min = VALUE_UINT8_MIN, .max = VALUE_UINT8_MAX},
	[VALUE_TYPE_SHORT] = {.size = 2, .min = VALUE_INT16_MIN, .max = VALUE_INT16_MAX},
	[VALUE_TYPE_USHORT] = {.size = 2, .min = VALUE_UINT16_MIN, .max = VALUE_UINT16_MAX},
	[VALUE_TYPE_INT] = {.size = 4, .min = VALUE_INT32_MIN, .max = VALUE_INT32_MAX},
	[VALUE_TYPE_UINT] = {.size = 4, .min = VALUE_UINT32_MIN, .max = VALUE_UINT32_MAX},
	[VALUE_TYPE_LONG] = {.size = 4, .min = VALUE_INT32_MIN, .max = VALUE_INT32_MAX},
	[VALUE_TYPE_ULONG] = {.size = 4, .min = VALUE_UINT32_MIN, .max = VALUE_UINT32_MAX},
	[VALUE_TYPE_FLOAT] = {.size = 4, .min = 0, .max = 0},
	[VALUE_TYPE_DOUBLE] = {.size = 8, .min = 0, .max = 0},
	[VALUE_TYPE_PTR] = {.size = 4, .min = VALUE_UINT32_MIN, .max = VALUE_UINT32_MAX},
	[VALUE_TYPE_STRING] = {.size = 1, .min = 0, .max = 0},
	[VALUE_TYPE_ADDR] = {.size = 4, .min = VALUE_UINT32_MIN, .max = VALUE_UINT32_MAX},
	[VALUE_TYPE_OFFSET] = {.size = 4, .min = VALUE_INT32_MIN, .max = VALUE_INT32_MAX},
	[VALUE_TYPE_CHARARR] = {.size = 1, .min = 0, .max = 0}
};

static data_table_element_t	data_table_16[VALUE_TYPE_COUNT] =
{
	[VALUE_TYPE_NULL] = {.size = 0, .min = 0, .max = 0},
	[VALUE_TYPE_CHAR] = {.size = 1, .min = VALUE_INT8_MIN, .max = VALUE_INT8_MAX},
	[VALUE_TYPE_UCHAR] = {.size = 1, .min = VALUE_UINT8_MIN, .max = VALUE_UINT8_MAX},
	[VALUE_TYPE_SHORT] = {.size = 2, .min = VALUE_INT16_MIN, .max = VALUE_INT16_MAX},
	[VALUE_TYPE_USHORT] = {.size = 2, .min = VALUE_UINT16_MIN, .max = VALUE_UINT16_MAX},
	[VALUE_TYPE_INT] = {.size = 2, .min = VALUE_INT16_MIN, .max = VALUE_INT16_MAX},
	[VALUE_TYPE_UINT] = {.size = 2, .min = VALUE_UINT16_MIN, .max = VALUE_UINT16_MAX},
	[VALUE_TYPE_LONG] = {.size = 4, .min = VALUE_INT32_MIN, .max = VALUE_INT32_MAX},
	[VALUE_TYPE_ULONG] = {.size = 4, .min = VALUE_UINT32_MIN, .max = VALUE_UINT32_MAX},
	[VALUE_TYPE_FLOAT] = {.size = 4, .min = 0, .max = 0},
	[VALUE_TYPE_DOUBLE] = {.size = 8, .min = 0, .max = 0},
	[VALUE_TYPE_PTR] = {.size = 2, .min = VALUE_UINT16_MIN, .max = VALUE_UINT16_MAX},
	[VALUE_TYPE_STRING] = {.size = 1, .min = 0, .max = 0},
	[VALUE_TYPE_ADDR] = {.size = 2, .min = VALUE_UINT16_MIN, .max = VALUE_UINT16_MAX},
	[VALUE_TYPE_OFFSET] = {.size = 2, .min = VALUE_INT16_MIN, .max = VALUE_INT16_MAX},
	[VALUE_TYPE_CHARARR] = {.size = 1, .min = 0, .max = 0}
};

static int	instr_size_64[INSTR_COUNT] =
{
	[EXIT_INSTR] = (EXIT_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[PAUSE_INSTR] = (PAUSE_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[END_INSTR] = (END_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[ENDCYCLE_INSTR] = (ENDCYCLE_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[BREAK_INSTR] = (BREAK_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[GOTO_INSTR] = (GOTO_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[DO_INSTR] = (DO_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[LOOP_INSTR] = (LOOP_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[FIRE_INSTR] = (FIRE_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[INIT_INSTR] = (INIT_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[CLEANUP_INSTR] = (CLEANUP_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[RET_INSTR] = (RET_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[PRIM_FIRE_INSTR] = (PRIM_FIRE_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[PRIM_INIT_INSTR] = (PRIM_INIT_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[PRIM_CLEANUP_INSTR] = (PRIM_CLEANUP_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[CPPTR_INSTR] = (CPPTR_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[INCPTR_INSTR] = (INCPTR_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[META_EXIT_INSTR] = (META_EXIT_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[META_LDCOMP_INSTR] = (META_LDCOMP_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[META_LDPRIM_INSTR] = (META_LDPRIM_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[META_MKBUF_INSTR] = (META_MKBUF_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[META_MKCOMPINST_INSTR] = (META_MKCOMPINST_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[META_MKPRIMINST_INSTR] = (META_MKPRIMINST_OP_COUNT + 1) * VALUE_INT_SIZE_64,
	[META_DELBUF_INSTR] = (META_DELBUF_OP_COUNT + 1) * VALUE_INT_SIZE_64
};

static int	instr_size_32[INSTR_COUNT] =
{
	[EXIT_INSTR] = (EXIT_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[PAUSE_INSTR] = (PAUSE_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[END_INSTR] = (END_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[ENDCYCLE_INSTR] = (ENDCYCLE_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[BREAK_INSTR] = (BREAK_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[GOTO_INSTR] = (GOTO_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[DO_INSTR] = (DO_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[LOOP_INSTR] = (LOOP_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[FIRE_INSTR] = (FIRE_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[INIT_INSTR] = (INIT_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[CLEANUP_INSTR] = (CLEANUP_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[RET_INSTR] = (RET_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[PRIM_FIRE_INSTR] = (PRIM_FIRE_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[PRIM_INIT_INSTR] = (PRIM_INIT_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[PRIM_CLEANUP_INSTR] = (PRIM_CLEANUP_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[CPPTR_INSTR] = (CPPTR_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[INCPTR_INSTR] = (INCPTR_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[META_EXIT_INSTR] = (META_EXIT_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[META_LDCOMP_INSTR] = (META_LDCOMP_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[META_LDPRIM_INSTR] = (META_LDPRIM_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[META_MKBUF_INSTR] = (META_MKBUF_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[META_MKCOMPINST_INSTR] = (META_MKCOMPINST_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[META_MKPRIMINST_INSTR] = (META_MKPRIMINST_OP_COUNT + 1) * VALUE_INT_SIZE_32,
	[META_DELBUF_INSTR] = (META_DELBUF_OP_COUNT + 1) * VALUE_INT_SIZE_32
};

static int	instr_size_16[INSTR_COUNT] =
{
	[EXIT_INSTR] = (EXIT_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[PAUSE_INSTR] = (PAUSE_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[END_INSTR] = (END_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[ENDCYCLE_INSTR] = (ENDCYCLE_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[BREAK_INSTR] = (BREAK_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[GOTO_INSTR] = (GOTO_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[DO_INSTR] = (DO_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[LOOP_INSTR] = (LOOP_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[FIRE_INSTR] = (FIRE_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[INIT_INSTR] = (INIT_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[CLEANUP_INSTR] = (CLEANUP_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[RET_INSTR] = (RET_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[PRIM_FIRE_INSTR] = (PRIM_FIRE_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[PRIM_INIT_INSTR] = (PRIM_INIT_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[PRIM_CLEANUP_INSTR] = (PRIM_CLEANUP_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[CPPTR_INSTR] = (CPPTR_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[INCPTR_INSTR] = (INCPTR_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[META_EXIT_INSTR] = (META_EXIT_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[META_LDCOMP_INSTR] = (META_LDCOMP_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[META_LDPRIM_INSTR] = (META_LDPRIM_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[META_MKBUF_INSTR] = (META_MKBUF_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[META_MKCOMPINST_INSTR] = (META_MKCOMPINST_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[META_MKPRIMINST_INSTR] = (META_MKPRIMINST_OP_COUNT + 1) * VALUE_INT_SIZE_16,
	[META_DELBUF_INSTR] = (META_DELBUF_OP_COUNT + 1) * VALUE_INT_SIZE_16
};

static compatibility_t	type_comp =
{
	[VALUE_TYPE_NULL]  = //value_type == VALUE_TYPE_NULL
		{
			[VALUE_TYPE_NULL] = 		-1,
			[VALUE_TYPE_CHAR] = 		0,
			[VALUE_TYPE_UCHAR] = 	0,
			[VALUE_TYPE_SHORT] = 	0,
			[VALUE_TYPE_USHORT] = 	0,
			[VALUE_TYPE_INT] = 		0,
			[VALUE_TYPE_UINT] = 		0,
			[VALUE_TYPE_LONG] = 		0,
			[VALUE_TYPE_ULONG] = 	0,
			[VALUE_TYPE_FLOAT] = 	0,
			[VALUE_TYPE_DOUBLE] = 	0,
			[VALUE_TYPE_PTR] = 		0,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		0,
			[VALUE_TYPE_OFFSET] = 0,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_CHAR]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	-1,
			[VALUE_TYPE_SHORT] = 	-1,
			[VALUE_TYPE_USHORT] = 	-1,
			[VALUE_TYPE_INT] = 		-1,
			[VALUE_TYPE_UINT] = 		-1,
			[VALUE_TYPE_LONG] = 		-1,
			[VALUE_TYPE_ULONG] = 	-1,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_UCHAR]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	-1,
			[VALUE_TYPE_SHORT] = 	-1,
			[VALUE_TYPE_USHORT] = 	-1,
			[VALUE_TYPE_INT] = 		-1,
			[VALUE_TYPE_UINT] = 		-1,
			[VALUE_TYPE_LONG] = 		-1,
			[VALUE_TYPE_ULONG] = 	-1,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_SHORT]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	-1,
			[VALUE_TYPE_SHORT] = 	-1,
			[VALUE_TYPE_USHORT] = 	-1,
			[VALUE_TYPE_INT] = 		-1,
			[VALUE_TYPE_UINT] = 		-1,
			[VALUE_TYPE_LONG] = 		-1,
			[VALUE_TYPE_ULONG] = 	-1,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_USHORT]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	-1,
			[VALUE_TYPE_SHORT] = 	-1,
			[VALUE_TYPE_USHORT] = 	-1,
			[VALUE_TYPE_INT] = 		-1,
			[VALUE_TYPE_UINT] = 		-1,
			[VALUE_TYPE_LONG] = 		-1,
			[VALUE_TYPE_ULONG] = 	-1,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_INT]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	-1,
			[VALUE_TYPE_SHORT] = 	-1,
			[VALUE_TYPE_USHORT] = 	-1,
			[VALUE_TYPE_INT] = 		-1,
			[VALUE_TYPE_UINT] = 		-1,
			[VALUE_TYPE_LONG] = 		-1,
			[VALUE_TYPE_ULONG] = 	-1,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_UINT]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	-1,
			[VALUE_TYPE_SHORT] = 	-1,
			[VALUE_TYPE_USHORT] = 	-1,
			[VALUE_TYPE_INT] = 		-1,
			[VALUE_TYPE_UINT] = 		-1,
			[VALUE_TYPE_LONG] = 		-1,
			[VALUE_TYPE_ULONG] = 	-1,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_LONG]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	-1,
			[VALUE_TYPE_SHORT] = 	-1,
			[VALUE_TYPE_USHORT] = 	-1,
			[VALUE_TYPE_INT] = 		-1,
			[VALUE_TYPE_UINT] = 		-1,
			[VALUE_TYPE_LONG] = 		-1,
			[VALUE_TYPE_ULONG] = 	-1,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_ULONG]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	-1,
			[VALUE_TYPE_SHORT] = 	-1,
			[VALUE_TYPE_USHORT] = 	-1,
			[VALUE_TYPE_INT] = 		-1,
			[VALUE_TYPE_UINT] = 		-1,
			[VALUE_TYPE_LONG] = 		-1,
			[VALUE_TYPE_ULONG] = 	-1,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_FLOAT] = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		0,
			[VALUE_TYPE_UCHAR] = 	0,
			[VALUE_TYPE_SHORT] = 	0,
			[VALUE_TYPE_USHORT] = 	0,
			[VALUE_TYPE_INT] = 		0,
			[VALUE_TYPE_UINT] = 		0,
			[VALUE_TYPE_LONG] = 		0,
			[VALUE_TYPE_ULONG] = 	0,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		0,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		0,
			[VALUE_TYPE_OFFSET] = 0,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_DOUBLE] = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		0,
			[VALUE_TYPE_UCHAR] = 	0,
			[VALUE_TYPE_SHORT] = 	0,
			[VALUE_TYPE_USHORT] = 	0,
			[VALUE_TYPE_INT] = 		0,
			[VALUE_TYPE_UINT] = 		0,
			[VALUE_TYPE_LONG] = 		0,
			[VALUE_TYPE_ULONG] = 	0,
			[VALUE_TYPE_FLOAT] = 	-1,
			[VALUE_TYPE_DOUBLE] = 	-1,
			[VALUE_TYPE_PTR] = 		0,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		0,
			[VALUE_TYPE_OFFSET] = 0,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_PTR]    = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		0,
			[VALUE_TYPE_UCHAR] = 	0,
			[VALUE_TYPE_SHORT] = 	0,
			[VALUE_TYPE_USHORT] = 	0,
			[VALUE_TYPE_INT] = 		0,
			[VALUE_TYPE_UINT] = 		0,
			[VALUE_TYPE_LONG] = 		0,
			[VALUE_TYPE_ULONG] = 	0,
			[VALUE_TYPE_FLOAT] = 	0,
			[VALUE_TYPE_DOUBLE] = 	0,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		0,
			[VALUE_TYPE_OFFSET] = 0,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_STRING] = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		-1,
			[VALUE_TYPE_UCHAR] = 	0,
			[VALUE_TYPE_SHORT] = 	0,
			[VALUE_TYPE_USHORT] = 	0,
			[VALUE_TYPE_INT] = 		0,
			[VALUE_TYPE_UINT] = 		0,
			[VALUE_TYPE_LONG] = 		0,
			[VALUE_TYPE_ULONG] = 	0,
			[VALUE_TYPE_FLOAT] = 	0,
			[VALUE_TYPE_DOUBLE] = 	0,
			[VALUE_TYPE_PTR] = 		0,
			[VALUE_TYPE_STRING] = 	-1,
			[VALUE_TYPE_ADDR] = 		0,
			[VALUE_TYPE_OFFSET] = 0,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_ADDR]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		0,
			[VALUE_TYPE_UCHAR] = 	0,
			[VALUE_TYPE_SHORT] = 	0,
			[VALUE_TYPE_USHORT] = 	0,
			[VALUE_TYPE_INT] = 		0,
			[VALUE_TYPE_UINT] = 		0,
			[VALUE_TYPE_LONG] = 		0,
			[VALUE_TYPE_ULONG] = 	0,
			[VALUE_TYPE_FLOAT] = 	0,
			[VALUE_TYPE_DOUBLE] = 	0,
			[VALUE_TYPE_PTR] = 		-1,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_OFFSET]   = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		0,
			[VALUE_TYPE_UCHAR] = 	0,
			[VALUE_TYPE_SHORT] = 	0,
			[VALUE_TYPE_USHORT] = 	0,
			[VALUE_TYPE_INT] = 		0,
			[VALUE_TYPE_UINT] = 		0,
			[VALUE_TYPE_LONG] = 		0,
			[VALUE_TYPE_ULONG] = 	0,
			[VALUE_TYPE_FLOAT] = 	0,
			[VALUE_TYPE_DOUBLE] = 	0,
			[VALUE_TYPE_PTR] = 		0,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		-1,
			[VALUE_TYPE_OFFSET] = -1,
			[VALUE_TYPE_CHARARR] =	0
		},
	[VALUE_TYPE_CHARARR] = 
		{
			[VALUE_TYPE_NULL] = 		0,
			[VALUE_TYPE_CHAR] = 		0,
			[VALUE_TYPE_UCHAR] = 	0,
			[VALUE_TYPE_SHORT] = 	0,
			[VALUE_TYPE_USHORT] = 	0,
			[VALUE_TYPE_INT] = 		0,
			[VALUE_TYPE_UINT] = 		0,
			[VALUE_TYPE_LONG] = 		0,
			[VALUE_TYPE_ULONG] = 	0,
			[VALUE_TYPE_FLOAT] = 	0,
			[VALUE_TYPE_DOUBLE] = 	0,
			[VALUE_TYPE_PTR] = 		0,
			[VALUE_TYPE_STRING] = 	0,
			[VALUE_TYPE_ADDR] = 		0,
			[VALUE_TYPE_OFFSET] = 0,
			[VALUE_TYPE_CHARARR] =	-1
		}
};

static architecture_t	architectures[SDF_ARCHITECTURE_COUNT] =
{
	[SDF_ARCHITECTURE_64] = 
	{
		.instr_tab = instr_table,
		.segdef_tab = segdef_table,
		.alloc_tab = alloc_table,
		.label_tab = label_table,
		.data_tab = data_table_64,
		.comp_tab = &type_comp,
		.instr_size = instr_size_64
	},
	[SDF_ARCHITECTURE_32] = 
	{
		.instr_tab = instr_table,
		.segdef_tab = segdef_table,
		.alloc_tab = alloc_table,
		.label_tab = label_table,
		.data_tab = data_table_32,
		.comp_tab = &type_comp,
		.instr_size = instr_size_32
	},
	[SDF_ARCHITECTURE_16] = 
	{
		.instr_tab = instr_table,
		.segdef_tab = segdef_table,
		.alloc_tab = alloc_table,
		.label_tab = label_table,
		.data_tab = data_table_16,
		.comp_tab = &type_comp,
		.instr_size = instr_size_16
	}
};

/*************************** Public functions *******************************/

int asm_lang_init_hash_table(void *hash)
{
	int i;
	word_t	*w;
	
	if (architecture >= SDF_ARCHITECTURE_COUNT)
		return -1;
	for (i = 0; i < INSTR_COUNT; i++)
	{
		if ((w = hash_table_lookup(hash, architectures[architecture].instr_tab[i].name)) == NULL)
		{
			return -1;
		}
		if (w->type != WORD_TYPE_UNDEF)
		{
			return -1;
		}
		w->type = architectures[architecture].instr_tab[i].type;
		w->comp_type = VALUE_TYPE_INT;
		w->value_type = VALUE_TYPE_LONG;
		w->value.l = (long) architectures[architecture].instr_tab[i].code;
		w->valid = -1;
	}
	for (i = 0; i < SEGDEF_COUNT; i++)
	{
		if ((w = hash_table_lookup(hash, architectures[architecture].segdef_tab[i].name)) == NULL)
		{
			return -1;
		}
		if (w->type != WORD_TYPE_UNDEF)
		{
			return -1;
		}
		if(architectures[architecture].segdef_tab[i].code == END_SEG)
			w->type = WORD_TYPE_SEG_END;
		else
			w->type = WORD_TYPE_SEGDEF;
		w->comp_type = VALUE_TYPE_NULL; // todo: make it configurable
		w->value_type = VALUE_TYPE_LONG;
		w->value.l = (long) architectures[architecture].segdef_tab[i].code;
		w->valid = -1;
	}
	for (i = 0; i < ALLOC_COUNT; i++)
	{
		if ((w = hash_table_lookup(hash, architectures[architecture].alloc_tab[i].name)) == NULL)
		{
			return -1;
		}
		if (w->type != WORD_TYPE_UNDEF)
		{
			return -1;
		}
		w->type = WORD_TYPE_ALLOCATOR;
		w->comp_type = VALUE_TYPE_NULL; // todo: make it configurable
		w->value_type = VALUE_TYPE_LONG;
		w->value.l = (long) architectures[architecture].alloc_tab[i].code;
		w->valid = -1;
	}
	for (i = 0; i < LABEL_COUNT; i++)
	{
		if ((w = hash_table_lookup(hash, architectures[architecture].label_tab[i].name)) == NULL)
		{
			return -1;
		}
		if (w->type != WORD_TYPE_UNDEF)
		{
			return -1;
		}
		w->type = WORD_TYPE_LABEL;
		w->comp_type = VALUE_TYPE_NULL; // todo: make it configurable
		w->value_type = architectures[architecture].label_tab[i].value_type;
		w->value.a.seg = 0;
		w->value.a.offs = 0;
		w->valid = 0;
	}
	return 0;
}

int asm_lang_instr_optype(unsigned int instr, unsigned int opnr)
{
	return architectures[architecture].instr_tab[instr].op_type[opnr];
}

int asm_lang_instr_opcount(unsigned int instr)
{
	return architectures[architecture].instr_tab[instr].op_count;
}

int asm_lang_instr_size(unsigned int instr)
{
	return architectures[architecture].instr_size[instr];
}

int asm_lang_alloc_optype(unsigned int alloc)
{
	return architectures[architecture].alloc_tab[alloc].op_type;
}

char *asm_lang_segdef_name(unsigned int segdef)
{
	return architectures[architecture].segdef_tab[segdef].name;
}

char *asm_lang_alloc_name(unsigned int alloc)
{
	return architectures[architecture].alloc_tab[alloc].name;
}

char *asm_lang_label_name(unsigned int label)
{
	return architectures[architecture].label_tab[label].name;
}

int asm_lang_datasize(unsigned int datatype)
{
	return architectures[architecture].data_tab[datatype].size;
}

int asm_lang_datamin(unsigned int datatype)
{
	return architectures[architecture].data_tab[datatype].min;
}

int asm_lang_datamax(unsigned int datatype)
{
	return architectures[architecture].data_tab[datatype].max;
}

int asm_lang_is_compatible(unsigned int value_type, unsigned int type)
{
	return (*architectures[architecture].comp_tab)[value_type][type];
}

int	asm_lang_compile_sentence(void *buf, int len, void *sent)
{
	void			*ptr = buf;
	void			*iter;
	word_t			*cw;
	word_t			*rw;
	int				comp_len, comp_size;
	long			comp_min, comp_max;
	double			d;
	float			f;
	long			l;
	unsigned long	ul;
	
	comp_len = 0;
	cw = (word_t *) sentence_get_first_word(sent,&iter);
	while(cw != NULL)
	{
		comp_size = 0;
		if (cw->root != NULL)
			rw = cw->root;
		else
			rw = cw;
		if (cw->comp_type != VALUE_TYPE_NULL)
		{
			if (architectures[architecture].comp_tab[rw->value_type][cw->comp_type])
			{
				comp_size = architectures[architecture].data_tab[cw->comp_type].size;
				switch(rw->value_type)
				{
					case	VALUE_TYPE_ADDR:
					case	VALUE_TYPE_LONG:
						if (rw->value_type == VALUE_TYPE_ADDR)
						{
							if (cw->comp_type == VALUE_TYPE_OFFSET)
								l = (long)rw->value.a.offs - (long)cw->value.a.offs;
							else
								l = rw->value.a.offs;
						}
						else
							l = rw->value.l;
						switch (cw->comp_type)
						{
							case	VALUE_TYPE_FLOAT:
								f = (float) l;
								if ((comp_len + comp_size) > len) 
								{
									fprintf(stderr,"error: asm_lang_compile_sentence(): len=%d comp_len=%d comp_size=%d\n",len,comp_len,comp_size);
									return -1;
								}
								memcpy(ptr, (void *) &f, comp_size);
								break;
							case	VALUE_TYPE_DOUBLE:
								d = (double) l;
								if ((comp_len + comp_size) > len)
								{
									fprintf(stderr,"error: asm_lang_compile_sentence(): len=%d comp_len=%d comp_size=%d\n",len,comp_len,comp_size);
									return -1;
								}
								memcpy(ptr, (void *) &d, comp_size);
								break;
							case	VALUE_TYPE_STRING:
								fprintf(stderr,"error: asm_lang_compile_sentence(): String compile type not allowed\n");
								return -1;
								break;
							default: // all the integral types
								comp_min = architectures[architecture].data_tab[cw->comp_type].min;
								comp_max = architectures[architecture].data_tab[cw->comp_type].max;
								if (comp_min == 0)
								{
									if (l < 0) return -1;
									ul = (unsigned long) l;
									if (ul > comp_max) return -1;
									if ((comp_len + comp_size) > len)
									{
										fprintf(stderr,"error: asm_lang_compile_sentence(): len=%d comp_len=%d comp_size=%d\n",len,comp_len,comp_size);
										return -1;
									}
									memcpy(ptr, (void *) &ul, comp_size);
								}
								else
								{
									if (l < comp_min) return -1;
									if (l > comp_max) return -1;
									if ((comp_len + comp_size) > len) return -1;
									memcpy(ptr, (void *) &l, comp_size);
								}
								break;
						} // end switch(w->comp_type)
						break;
					case	VALUE_TYPE_DOUBLE:
						switch (cw->comp_type)
						{
							case	VALUE_TYPE_DOUBLE:
								d = rw->value.d;
								if ((comp_len + comp_size) > len) return -1;
								memcpy(ptr, (void *) &d, comp_size);
								break;
							case	VALUE_TYPE_FLOAT:
								f = (float) rw->value.d;
								if ((comp_len + comp_size) > len) return -1;
								memcpy(ptr, (void *) &f, comp_size);
								break;
							default:
								fprintf(stderr,"error: asm_lang_compile_sentence(): Only float or double compile type allowed\n");
								return -1;
						} // end switch(w->comp_type)
						break;
					case	VALUE_TYPE_STRING:
						switch (cw->comp_type)
						{
							case	VALUE_TYPE_STRING:
								comp_size = strlen(rw->value.s) + 1;
								if ((comp_len + comp_size) > len) return -1;
								strcpy((char *)ptr, rw->value.s);
								break;
							case	VALUE_TYPE_CHAR:
								comp_size = strlen(rw->value.s);
								if ((comp_len + comp_size) > len) return -1;
								memcpy(ptr, (void *) rw->value.s, comp_size);
								break;
							default:
								fprintf(stderr,"error: asm_lang_compile_sentence(): Only string compile type allowed\n");
								return -1;
						} // end switch(w->comp_type)
						break;
					default:
						fprintf(stderr,"error: asm_lang_compile_sentence(): Invalid value type\n");
						return -1;
				} // end switch(w->value_type)
			}
			else
			{
				fprintf(stderr,"error: asm_lang_compile_sentence(): Incompatible compile type\n");
				return -1;
			} // end if (pf->compatibility[w->value_type][w->comp_type])
		}// end if(w->comp_type != VALUE_TYPE_NULL)
		if (comp_size < 0)
			fprintf(stderr,"error: asm_lang_compile_sentence(): Negative comp_size %d\n", comp_size);
		comp_len += comp_size;
		if (comp_len < 0)
			fprintf(stderr,"error: asm_lang_compile_sentence(): Negative comp_len %d\n", comp_len);
		ptr += comp_size;
		//cw = cw->next;
		cw = (word_t *) sentence_get_next_word(sent,&iter);
	} // end while(cw!=NULL) 
	return comp_len;
}

int	asm_lang_compile_word(void *buf, int len, void *word)
{
	void			*ptr = buf;
	word_t			*cw = (word_t *) word;
	word_t			*rw;
	int				comp_size;
	long			comp_min, comp_max;
	double			d;
	float			f;
	long			l;
	unsigned long	ul;
	
	comp_size = 0;

	if (cw->root != NULL)
		rw = cw->root;
	else
		rw = cw;
	if (cw->comp_type != VALUE_TYPE_NULL)
	{
		if (architectures[architecture].comp_tab[rw->value_type][cw->comp_type])
		{
			comp_size = architectures[architecture].data_tab[cw->comp_type].size;
			switch(rw->value_type)
			{
				case	VALUE_TYPE_ADDR:
				case	VALUE_TYPE_LONG:
						if (rw->value_type == VALUE_TYPE_ADDR)
						{
							if (cw->comp_type == VALUE_TYPE_OFFSET)
							{
								l = (long)rw->value.a.offs - (long)cw->value.a.offs;
							}
							else
								l = rw->value.a.offs;
						}
						else
							l = rw->value.l;
					switch (cw->comp_type)
					{
						case	VALUE_TYPE_FLOAT:
							f = (float) l;
							if (comp_size > len) return -1;
							memcpy(ptr, (void *) &f, comp_size);
							break;
						case	VALUE_TYPE_DOUBLE:
							d = (double) l;
							if (comp_size > len) return -1;
							memcpy(ptr, (void *) &d, comp_size);
							break;
						case	VALUE_TYPE_STRING:
							return -1;
							break;
						default: // all the integral types
							comp_min = architectures[architecture].data_tab[cw->comp_type].min;
							comp_max = architectures[architecture].data_tab[cw->comp_type].max;
							if (comp_min == 0)
							{
								if (l < 0) return -1;
								ul = (unsigned long) l;
								if (ul > comp_max) return -1;
								if (comp_size > len) return -1;
								memcpy(ptr, (void *) &ul, comp_size);
							}
							else
							{
								if (l < comp_min) return -1;
								if (l > comp_max) return -1;
								if (comp_size > len) return -1;
								memcpy(ptr, (void *) &l, comp_size);
							}
							break;
					} // end switch(w->comp_type)
					break;
				case	VALUE_TYPE_DOUBLE:
					switch (cw->comp_type)
					{
						case	VALUE_TYPE_DOUBLE:
							d = rw->value.d;
							if (comp_size > len) return -1;
							memcpy(ptr, (void *) &d, comp_size);
							break;
						case	VALUE_TYPE_FLOAT:
							f = (float) rw->value.d;
							if (comp_size > len) return -1;
							memcpy(ptr, (void *) &f, comp_size);
							break;
						default:
							return -1;
					} // end switch(w->comp_type)
					break;
				case	VALUE_TYPE_STRING:
					switch (cw->comp_type)
					{
						case	VALUE_TYPE_STRING:
							comp_size = strlen(rw->value.s)+1;
							if (comp_size > len) return -1;
							memcpy(ptr, (void *)rw->value.s, comp_size);
							break;
						case	VALUE_TYPE_CHAR:
							comp_size = strlen(rw->value.s);
							if ((comp_size) > len) return -1;
							memcpy(ptr, (void *) rw->value.s, comp_size);
							break;
						default:
							return -1;
					} // end switch(w->comp_type)
					break;
				default:
					return -1;
			} // end switch(w->value_type)
		}
		else
		{
			return -1;
		} // end if (pf->compatibility[w->value_type][w->comp_type])
	} // end if(w->comp_type != VALUE_TYPE_NULL)
	return comp_size;
}
