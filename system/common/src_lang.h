/*******************************************************************************
 * 							Common src_lang
 * *****************************************************************************
 * 	Filename:		src_lang.h
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

#include	"words.h"

#ifndef	SRC_LANG_H
#define	SRC_LANG_H
/*
#define ALLOC_COUNT					14
#define	ALLOC_DATA_TYPE_COUNT		14

#define	UNDEF_ALLOC			0
#define	CHAR_ALLOC			1
#define	UCHAR_ALLOC			2
#define SHORT_ALLOC			3
#define USHORT_ALLOC		4
#define INT_ALLOC			5
#define UINT_ALLOC			6
#define LONG_ALLOC			7
#define ULONG_ALLOC			8
#define FLOAT_ALLOC			9
#define DOUBLE_ALLOC		10
#define PTR_ALLOC			11
#define	STRING_ALLOC		12
#define ADDR_ALLOC			13
*/
/*
#define	ALLOC_DATA_TYPE_NULL		0
#define	ALLOC_DATA_TYPE_CHAR		1
#define	ALLOC_DATA_TYPE_UCHAR		2
#define ALLOC_DATA_TYPE_SHORT		3
#define ALLOC_DATA_TYPE_USHORT		4
#define ALLOC_DATA_TYPE_INT			5
#define ALLOC_DATA_TYPE_UINT		6
#define ALLOC_DATA_TYPE_LONG		7
#define ALLOC_DATA_TYPE_ULONG		8
#define ALLOC_DATA_TYPE_FLOAT		9
#define ALLOC_DATA_TYPE_DOUBLE		10
#define ALLOC_DATA_TYPE_PTR			11
#define ALLOC_DATA_TYPE_STRING		12
#define	ALLOC_DATA_TYPE_ADDR		13
*/

#define	ASM_CODE_COUNT	41

#define	L_META_SEG		0
#define	L_COMP_NAME		1
#define	L_COMP_VER		2
#define	L_DEF_NAME		3
#define	L_LOAD_LAB		4
#define	L_LOAD_PRIM		5
#define	L_META_EXIT		6
#define	L_MAKE_LAB		7
#define	L_MAKE_BUF		8
#define	L_MAKE_PRIM		9
#define	L_MAKE_COMP		10
#define	L_DEL_LAB		11
#define	L_DEL_BUF		12
#define	L_ENDSEG		13
#define	L_CODE_SEG		14
#define	L_EXIT			15
#define	L_INIT_LAB		16
#define	L_CP_PTR		17
#define	L_INIT_PRIM		18
#define	L_INIT_COMP		19
#define	L_RET			20
#define	L_END_CYCLE		21
#define	L_FIRE_LAB		22
#define	L_DO			23
#define	L_LOOP_LAB		24
#define	L_FIRE_PRIM		25
#define	L_FIRE_COMP		26
#define	L_PADD_LINT		27
#define	L_LOOP			28
#define	L_CLEAN_PRIM	29
#define	L_CLEAN_COMP		30
#define	L_CP_CTX_PTR	31
#define	L_CLEAN_LAB		32
#define	L_DATA_SEG		33
#define	L_DEF_PTR		34
#define	L_DEF_SIG		35
#define	L_DEF_ACT		36
#define	L_LOAD_COMP		37
#define L_CONTEXT_SEG	38
#define L_DEF_PORT_PTR	39

int		src_lang_init_hash_table(void *hash_table);
char 	*src_lang_get_alloc_name(unsigned int code);
int		src_lang_get_alloc_size(unsigned int code);
word_t 	*src_lang_new_do_token(void *set, int cycle_count);
word_t 	*src_lang_new_loop_token(void *set, void *last_do);
char	*src_lang_get_asm_line(unsigned int index);

#endif
