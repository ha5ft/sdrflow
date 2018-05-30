/*******************************************************************************
 * 							Common words
 * *****************************************************************************
 * 	Filename:		words.h
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

#ifndef	WORDS_H
#define	WORDS_H

#include	"value.h"
#include	"common.h"

#define WORD_TYPE_COUNT			57

#define WORD_TYPE_UNDEF			0
#define	WORD_TYPE_SEGDEF		1
#define	WORD_TYPE_ALLOCATOR		2
#define	WORD_TYPE_INSTRUCTION	3
#define WORD_TYPE_LABEL			4
#define WORD_TYPE_ARRAY_SIZE	5
#define WORD_TYPE_LITERAL		6
#define WORD_TYPE_COMMENT		7
#define WORD_TYPE_EOL			8
#define WORD_TYPE_EOF			9
#define WORD_TYPE_ERROR			10
#define	WORD_TYPE_SEG_END		11
#define	WORD_TYPE_META			12

#define	WORD_TYPE_UNDEF_NAME	13
#define	WORD_TYPE_INPUT_NAME	14
#define	WORD_TYPE_OUTPUT_NAME	15
#define	WORD_TYPE_PARAM_NAME	16
#define	WORD_TYPE_STATE_NAME	17
#define	WORD_TYPE_TEMP_NAME		18
#define	WORD_TYPE_STREAM_NAME	19
#define	WORD_TYPE_VAR_NAME		20
#define	WORD_TYPE_CONST_NAME	21
#define	WORD_TYPE_PRIM_NAME		22
#define	WORD_TYPE_COMPOSITE_NAME	23
#define	WORD_TYPE_PRIM_ACTOR_NAME	24
#define	WORD_TYPE_COMP_ACTOR_NAME	25
#define	WORD_TYPE_ACT_INP_NAME	26
#define	WORD_TYPE_ACT_OUT_NAME	27
#define	WORD_TYPE_ACT_PAR_NAME	28
#define	WORD_TYPE_ACT_STA_NAME	29
#define	WORD_TYPE_ACT_TMP_NAME	30

#define	WORD_TYPE_IN_OP			31
#define	WORD_TYPE_DELEYED_IN_OP	32
#define	WORD_TYPE_OUT_OP		33
#define	WORD_TYPE_EQU_OP		34

#define	WORD_TYPE_USE			35
#define	WORD_TYPE_PRIMITIVE		36
#define	WORD_TYPE_COMPOSITE		37
#define	WORD_TYPE_IMPL			38
#define	WORD_TYPE_STREAM		39
#define	WORD_TYPE_VAR			40
#define	WORD_TYPE_CONST			41
#define	WORD_TYPE_INPUT			42
#define	WORD_TYPE_OUTPUT		43
#define	WORD_TYPE_PARAM			44
#define	WORD_TYPE_STATE			45
#define	WORD_TYPE_TEMP			46
#define	WORD_TYPE_ACTORS		47
#define	WORD_TYPE_SCHEDULE		48
#define	WORD_TYPE_DO			49
#define	WORD_TYPE_LOOP			50
#define	WORD_TYPE_END			51
#define	WORD_TYPE_RESERVED_NAME	52
#define	WORD_TYPE_SIGNALS		53
#define	WORD_TYPE_TOPOLOGY		54
#define	WORD_TYPE_CONTEXT		55

#define	WORD_TYPE_COMP_IMP_NAME	56

#define	WORD_TYPE_MANUAL		57
#define	WORD_TYPE_AUTO			58

#define WORD_NAME_LENGTH		COMMON_NAME_LENGTH

typedef struct _word
{
	struct _word	*next_hash; // used in the hash table
	struct _word	*root;
	value_t			value;		// this is a union
	int				type;		// lexical type of the word
//	int				clone;		// -1 if clone, 0 if original
	int				valid;		//
	int				value_type;	// type of the value stored in the value union
	int				comp_type;  // type of the value compiled into the code
	int				size;
	char			name[WORD_NAME_LENGTH + 1]; 		// string used for hashing
} word_t;

int		words_init(void);
void	*words_new_set(void);
void	*words_dup_set(void *set);
void	*words_clone_set(void *set);
word_t	*words_new(void *set);
int		words_get_name(void *word, char *str, int len);
void	words_prefix_name(word_t *w, char *prefix);
void	words_postfix_name(word_t *w, char *postfix);
void	words_store_name(word_t *w, char *name);
word_t	*words_dup_word(void *set, word_t *word);
word_t	*words_clone_word(void *set, word_t *word);
int		words_delete_set(void *set);
int		words_delete(void *set, word_t *word);
int		words_cleanup(void);

#endif
