/*******************************************************************************
 * 							Common scanner
 * *****************************************************************************
 * 	Filename:		scanner.c
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

 /* *****************************************************************************
 * State machine description
 * (0)	+-		(1)		.		(2)		0-9		(3)	0-9		(3)
 * 													eE		(4)	+-		(5)	0-9		(6)	0-9		(6)
 * 																						sp\t;\0	(7)*
 * 																						\		(7)*
 * 																						other	(8)*
 * 																			other	(8)*
 * 																0-9		(6)
 * 																other	(8)*
 * 													sp\t;\0	(7)*
 * 													\		(7)*
 * 													other	(8)*
 * 										other	(8)*

 * 						0-9		(9)		0-9		(9)
 * 										.		(2)
 * 										eE		(4)
 * 										sp\t;\0	(10)*
 * 										\		(10)*
 * 										other	(8)
 * 						other	(8)
 * 		1-9		(9)
 * 		0		(11)	0-9		(9)
 * 						.		(2)
 * 						eE		(4)
 * 						sp\t;\0	(10)*
 * 						\		(10)*
 * 						xX		(12)	0-9
 *										a-f
 * 										A-F		(13)	0-9
 * 														a-f
 * 														A-F		(13)
 * 														sp\t;\0	(14)*
 * 														\		(14)*
 * 														other	(8)*
 * 										other	(8)*
 * 						other	(8)*
 * 		.		(15)	0-9		(3)
 * 						a-z
 * 						A-Z		(16)	.
 * 										0-9
 * 										a-z
 * 										A-Z		(16)
 * 										sp\t;\0
 * 										\
 * 										[		(17)*
 * 										other	(8)*
 * 						other	(8)
 * 		a-z
 * 		A-Z		(16)
 * 		[		(18)	0-9		(19)	0-9		(19)
 * 										]		(20)	sp\t;\0	(21)*
 * 														\		(21)*
 * 														other	(8)*
 * 										other	(8)*
 * 						other	(8)*
 * 		;		(22)	\0		(23)*
 * 						other	(22)
  * 		sp\t	(0)
 * 		"		(24)	"		(25)	sp\t;\0	(26)*
 * 										\		(26)*
 * 										other	(8)*
 * 						\0		(8)
 * 						other	(24)
 * 		\		(27)	\0		(28)	EOF		(34)
 * 										notEOF	(0)
 * 						other	(8)*
 * 		'		(29)	other	(30)	'		(31)	sp\t;\0	(32)*
 * 														\		(32)*
 * 														other	(8)*
 * 										other	(8)
 * 		\0		(33)*
 * 		_		(35)	a-z
 * 						A-Z		(16)
 * 						other	(8)*
 * 		other	(8)*
 * 
 * States:	0	start
 * 			7	floating point number
 * 			8	error
 * 			10	integer number in decimal format
 * 			14	integer number in hex format
 * 			17	identifier
 * 			21	array size
 * 			23	comment
 * 			26	string
 * 			28	line continuation
 * 			32	character
 * 			33	end of line
 * 			34	end of file
 * An identifire could start with a letter or '.'. If it starts with '.' the 
 * second character should be a letter.
 ******************************************************************************/

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	"objects.h"
#include	"scanner.h"
#include	"hash_table.h"
#include	"words.h"
#include	"io.h"

#define	ERROR_COUNT				27

#define STATE_LINE_START		0
#define	STATE_NUM_SIGN			1
#define STATE_NUM_DEC_POINT		2
#define	STATE_NUM_FRAC_DIGIT	3
#define STATE_NUM_EXP_START		4
#define	STATE_NUM_EXP_SIGN		5
#define STATE_NUM_EXP_DIGIT		6
#define STATE_NUM_REAL_END		7
#define STATE_ERROR				8
#define STATE_NUM_INT_DIGIT		9
#define STATE_NUM_INT_END		10
#define STATE_NUM_ZERO_DIGIT	11
#define STATE_NUM_HEX_START		12
#define STATE_NUM_HEX_DIGIT		13
#define STATE_NUM_HEX_END		14
#define STATE_DOT				15
#define STATE_IDENT_CHAR		16
#define STATE_IDENT_END			17
#define STATE_ARR_LIM_START		18
#define STATE_ARR_LIM_DIGIT		19
#define STATE_ARR_LIM_CLOSE		20
#define STATE_ARR_LIM_END		21
#define STATE_COMMENT_START		22
#define STATE_COMMENT_END		23
#define STATE_STRING_START		24
#define STATE_STRING_START		24
#define STATE_STRING_START		24
#define STATE_STRING_CLOSE		25
#define STATE_STRING_END		26
#define STATE_LINE_CONT_START	27
#define STATE_LINE_CONT_END		28
#define STATE_CHAR_START		29
#define STATE_CHAR_CHAR			30
#define STATE_CHAR_CLOSE		31
#define STATE_CHAR_END			32
#define STATE_EOL				33
#define STATE_EOF				34
#define STATE_UNDERLINE			35

#define	STATE_IN_OPERATOR_START	36
#define	STATE_IN_OPERATOR_CLOSE	37
#define	STATE_IN_OPERATOR_END	38

#define	STATE_OUT_OPERATOR_START	39
#define	STATE_OUT_OPERATOR_CLOSE	40
#define	STATE_OUT_OPERATOR_END		41

#define	STATE_EQU_OPERATOR_START	42
#define	STATE_EQU_OPERATOR_END		43

#define	STATE_ARR_LIM_CLOSE_1		44
#define	STATE_ARR_LIM_END_1			45

#define LINE_BUF_LENGTH			256
#define LINE_LENGTH				255
//#define	SCANNER_ALLOC_SIZE		2


typedef struct	_scanner
{
	//void		*inp;
	//void		*hash;
	char		line[LINE_BUF_LENGTH];
	char		*pos;
	char		*start;
	char		ch;
	int			state;
	sdferr_t	err;
	void		*words;
//	void		*names;
	word_t		*push_back_word;
} scanner_t;

static void	*scanner_set = NULL;
static int	scanner_initialized = 0;
static unsigned int	scanner_type_index = (unsigned int) OBJECTS_TYPE_SCANNER;

//void	*HashTable = NULL;

static	char	*error_description[ERROR_COUNT] =
{
	"No Error", // 0
	"End of line", // 1
	"End of file", // 2
	"Illegal character", // 3
	"'.' or '0'-'9' are expected", // 4
	"'0'-'9' are expected", // 5
	"'0'-'9','e','E',';',white space or EOL expected", // 6
	"'0'-'9','+' or '-' are expected", // 7
	"'0'-'9',';',white space or EOL expected", // 8
	"Can not allocate word structure", // 9
	"Can not store scanned token", // 10
	"Error in converting a number", // 11
	"'0'-'9','e','E','.',';',white space or EOL expected", // 12
	"'0'-'9','e','E','.',';','x','X',white space or EOL expected", // 13
	"letters or digits are expected", // 14
	"letters,digits,';','[', '_',white space or EOL are expected", // 15
	"hash table lookup failed", // 16
	"'0'-'9' or ']' are expected", // 17
	"';', white space or EOL are expected", // 18
	"End of line before string end", // 19
	"End of line is expected", // 20
	"Unexpected end of line", // 21
	"''' is expected", // 22
	"Letters are expected", // 23
	"Unexpected EOF" // 24
	"Read_line failed", // 25
	"Can not duplicate token word" // 26
};

int	scanner_init(void)
{
	if (scanner_initialized != 0) return -1;
	if (objects_is_initialized() == 0)
	{
		if (objects_init() != 0) return -1;
	}
//	if ((scanner_type_index = objects_register_type(sizeof(scanner_t), SCANNER_ALLOC_SIZE)) < 0) return -1;
	scanner_type_index = (unsigned int) OBJECTS_TYPE_SCANNER;
	//fprintf(stderr, "scanner_init(): type index = %d\n", scanner_type_index);
	if ((scanner_set = objects_new_set(scanner_type_index)) == NULL)
	{
		objects_delete_type(scanner_type_index);
//		scanner_type_index = -1;
		return -1;
	}
	scanner_initialized = -1;
	//fprintf(stderr, "scanner_init(): scanner set = %ld\n", (long) scanner_set);
	return 0;
}
/*
void	scanner_print_hash(void * scan)
{
	hash_table_print(((scanner_t *)scan)->hash);
}

word_t	*scanner_lookup(void *scan, char *name)
{
	return hash_table_check(((scanner_t *) scan)->hash, name);
}
*/
void	*scanner_new(void)
{
	scanner_t	*s;
	
	if (scanner_initialized == 0) return NULL;
	if ((s = (scanner_t *) objects_new(scanner_set, sizeof(scanner_t))) == NULL) return NULL;
	if ((s->words = words_new_set()) == NULL)
	{
		objects_delete(scanner_set, s);
		return NULL;
	}
//	if ((s->names = names_new_set()) == NULL)
//	{
//		words_delete_set(s->words);
//		objects_delete(scanner_set, s);
//		return NULL;
//	}
	s->pos = s->line;
	s->start = s->line;
	s->ch = 0;
	s->state = STATE_EOL;
	s->err.lineno = 0;
	s->err.errnr = SCANNER_ERROR_NO_ERROR;
	s->err.errstr = error_description;
	//s->inp = inp;
	s->line[LINE_LENGTH] = 0;
	s->push_back_word = NULL;
	/*
	if ((s->hash = hash_table_new()) != NULL)
	{
		platform_init_hash_table(s->hash);
		HashTable = s->hash;
		hash_table_print(s->hash);
		return (void *) s;
	}
	else
	{
		objects_delete(scanner_set, (void *)s);
		return NULL;
	}
	*/
	return (void *) s;
}

sdferr_t	*scanner_error(void *scan)
{
	scanner_t	*s = (scanner_t *) scan;
	
	return &s->err;
}

int		scanner_lineno(void *scan)
{
	return ((scanner_t *) scan)->err.lineno;
}
/*
word_t	*scanner_dup_word(void *scan, word_t *word)
{
	return hash_table_dup_word(((scanner_t *) scan)->hash, word);
}

word_t	*scanner_get_label(void *scan, unsigned int label)
{
	char	*name;
	//word_t	*w;
	
	if ((name = platform_label_name(label)) == NULL) return NULL;
	return hash_table_check(((scanner_t *) scan)->hash, name);
	//return hash_table_dup_word(((scanner_t *) scan)->hash, w);
}

word_t	*scanner_get_alloc(void *scan, unsigned int alloc)
{
	char	*name;
	//word_t	*w;
	
	if ((name = platform_alloc_name(alloc)) == NULL) return NULL;
	return hash_table_check(((scanner_t *) scan)->hash, name);	
	//return hash_table_dup_word(((scanner_t *) scan)->hash, w);
}

word_t	*scanner_get_segdef(void *scan, unsigned int segdef)
{
	char	*name;
	//word_t	*w;
	
	if ((name = platform_segdef_name(segdef)) == NULL) return NULL;
	return hash_table_check(((scanner_t *) scan)->hash, name);	
	//return hash_table_dup_word(((scanner_t *) scan)->hash, w);
}

word_t	*scanner_get_new_word(void * scan)
{
	return hash_table_get_empty_word(((scanner_t *) scan)->hash);
}
*/

void	scanner_push_back_word(void *scan, word_t *w)
{
	((scanner_t *) scan)->push_back_word = w;
}

word_t	*scanner_scan(void *scan, void *inp, void *hash)
{
	scanner_t	*s = (scanner_t *) scan;
	word_t	*w;
	char	c,*save_pos = NULL;
	int		size;
	char	inp_name[IO_NAME_LENGTH + 1];
	
	if (s->state == STATE_ERROR)
	{
		return NULL;
	}
	if ((w = s->push_back_word) != NULL)
	{
		s->push_back_word = NULL;
		return w;
	}
	if ((s->state == STATE_EOL) || (s->state == STATE_EOF))
	{
		if ((size = io_get(inp, s->line, LINE_LENGTH)) > 0) 
		{
			
			s->err.lineno++;
			s->pos = s->line;
			s->ch = *s->pos;
		}
		else
		{
			if (size == 0) // end of file reached
			{
				s->err.errnr = SCANNER_ERROR_EOF;
				s->state = STATE_EOF;
			}
			else
			{
				s->err.errnr = SCANNER_ERROR_READ;
				s->state = STATE_ERROR;
			}
			return NULL;
		}
	}
	s->state = STATE_LINE_START;
	while (1)
	{
		switch (s->state)
		{
			case	STATE_LINE_START:
				s->start = s->pos;
				//fprintf(stderr, "Token starting char: %d\n", (int) s->ch);
				if ((s->ch == 32) || (s->ch == 9)) break;
				if ((s->ch == '+') || (s->ch == '-')) {s->state = STATE_NUM_SIGN; break;}
				if ((s->ch >= '1') && (s->ch <= '9')) {s->state = STATE_NUM_INT_DIGIT; break;}
				if (s->ch == '0') {s->state = STATE_NUM_ZERO_DIGIT; break;}
				if (s->ch == '.') {s->state = STATE_DOT; break;}
				if (s->ch == '_') {s->state = STATE_UNDERLINE; break;}
				if ((s->ch >= 'a') && (s->ch <= 'z')) {s->state = STATE_IDENT_CHAR; break;}
				if ((s->ch >= 'A') && (s->ch <= 'Z')) {s->state = STATE_IDENT_CHAR; break;}
				if (s->ch == '[') {s->state = STATE_ARR_LIM_START; break;}
				if (s->ch == '<') {s->state = STATE_IN_OPERATOR_START; break;}
				if (s->ch == '>') {s->state = STATE_OUT_OPERATOR_START; break;}
				if (s->ch == '=') {s->state = STATE_EQU_OPERATOR_START; break;}
				if (s->ch == ';') {s->state = STATE_COMMENT_START; break;}
				if (s->ch == 0) {s->state = STATE_EOL; break;}
				if (s->ch == '"') {s->state = STATE_STRING_START; break;}
				if (s->ch == '\\') {s->state = STATE_LINE_CONT_START; break;}
				if (s->ch == '\'') {s->state = STATE_CHAR_START; break;}
				{s->state = STATE_ERROR;s->err.errnr = 3; fprintf (stderr, "illegal character = %d", (int)s->ch); break;} //illegal character
// **************************** Literal number *********************************
			case	STATE_NUM_SIGN:
				if (s->ch == '.') {s->state = STATE_NUM_DEC_POINT; break;}
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_INT_DIGIT; break;}
				{s->state = STATE_ERROR; s->err.errnr = 4; break;}
			case	STATE_NUM_DEC_POINT:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_FRAC_DIGIT; break;}
				{s->state = STATE_ERROR; s->err.errnr = 5; break;}
			case	STATE_NUM_FRAC_DIGIT:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_FRAC_DIGIT; break;}
				if ((s->ch == 'e') || (s->ch == 'E')) {s->state = STATE_NUM_EXP_START; break;}
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_NUM_REAL_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 6; break;}
			case	STATE_NUM_EXP_START:
				if ((s->ch == '+') || (s->ch == '-')) {s->state = STATE_NUM_EXP_SIGN; break;}
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_EXP_DIGIT; break;}
				{s->state = STATE_ERROR; s->err.errnr = 7; break;}
			case	STATE_NUM_EXP_SIGN:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_EXP_DIGIT; break;}
				{s->state = STATE_ERROR; s->err.errnr = 5; break;}
			case	STATE_NUM_EXP_DIGIT:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = 6; break;}
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = 7; save_pos = s->pos; break;}
				{s->state = 8; s->err.errnr = 8; break;}
			case	STATE_NUM_REAL_END: // floating point number
				c = *save_pos;
				*save_pos = 0;
				if ((w = words_new(s->words)) == NULL)
				{s->state = STATE_ERROR; s->err.errnr = 9; *save_pos = c; break;}
//				if ((w->name = names_new(s->names, s->start)) == NULL)
//				{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start);
				if (sscanf(s->start,"%lf",&(w->value.d)) != 1)
					{s->state = 8; s->err.errnr = 11; *save_pos = c; break;}
				*save_pos = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				w->type = WORD_TYPE_LITERAL;
				w->value_type = VALUE_TYPE_DOUBLE;
				w->valid = -1;
				return w;
			case	STATE_NUM_INT_DIGIT:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_INT_DIGIT; break;}
				if ((s->ch == 'e') || (s->ch == 'E')) {s->state = STATE_NUM_EXP_START; break;}
				if (s->ch == '.') {s->state = STATE_NUM_DEC_POINT; break;}
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_NUM_INT_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 12; break;}
			case	STATE_NUM_INT_END: // long integer in decimal format
				c = *save_pos;
				*save_pos = 0;
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; *save_pos = c; break;}
//				if ((w->name = names_new(s->names, s->start)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start);
				if (sscanf(s->start,"%ld",&(w->value.l)) != 1)
					{s->state = STATE_ERROR; s->err.errnr = 11; *save_pos = c; break;}
				*save_pos = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				w->type = WORD_TYPE_LITERAL;
				w->value_type = VALUE_TYPE_LONG;
				w->valid = -1;
				return w;
			case	STATE_NUM_ZERO_DIGIT:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_INT_DIGIT; break;}
				if (s->ch == '.') {s->state = STATE_NUM_DEC_POINT; break;}
				if ((s->ch == 'e') || (s->ch == 'E')) {s->state = STATE_NUM_EXP_START; break;}
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_NUM_INT_END; save_pos = s->pos; break;}
				if ((s->ch == 'x') || (s->ch == 'X')) {s->state = STATE_NUM_HEX_START; break;}
				{s->state = STATE_ERROR; s->err.errnr = 13; break;}
			case	STATE_NUM_HEX_START:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_HEX_DIGIT; break;}
				if ((s->ch >= 'a') && (s->ch <= 'f')) {s->state = STATE_NUM_HEX_DIGIT; break;}
				if ((s->ch >= 'A') && (s->ch <= 'F')) {s->state = STATE_NUM_HEX_DIGIT; break;}
				{s->state = STATE_ERROR; s->err.errnr = 13; break;}
			case	STATE_NUM_HEX_DIGIT:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_HEX_DIGIT; break;}
				if ((s->ch >= 'a') && (s->ch <= 'f')) {s->state = STATE_NUM_HEX_DIGIT; break;}
				if ((s->ch >= 'A') && (s->ch <= 'F')) {s->state = STATE_NUM_HEX_DIGIT; break;}
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_NUM_HEX_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 13; break;}
			case	STATE_NUM_HEX_END: //long integer in hex format
				c = *save_pos;
				*save_pos = 0;
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; *save_pos = c; break;}
//				if ((w->name = names_new(s->names, s->start)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start);
					//{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				if (sscanf(s->start,"%lx",&(w->value.l)) != 1)
					{s->state = STATE_ERROR; s->err.errnr = 11; *save_pos = c; break;}
				*save_pos = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				w->type = WORD_TYPE_LITERAL;
				w->value_type = VALUE_TYPE_LONG;
				w->valid = -1;
				return w;
// ****************************** Identifier ***********************************
			case	STATE_UNDERLINE:
				if ((s->ch >= 'a') && (s->ch <= 'z')) {s->state = STATE_IDENT_CHAR; break;}
				if ((s->ch >= 'A') && (s->ch <= 'Z')) {s->state = STATE_IDENT_CHAR; break;}
				{s->state = STATE_ERROR; s->err.errnr = 23; break;}
			case	STATE_DOT:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_NUM_FRAC_DIGIT; break;}
				if ((s->ch >= 'a') && (s->ch <= 'z')) {s->state = STATE_IDENT_CHAR; break;}
				if ((s->ch >= 'A') && (s->ch <= 'Z')) {s->state = STATE_IDENT_CHAR; break;}
				{s->state = STATE_ERROR; s->err.errnr = 14; break;}
			case	STATE_IDENT_CHAR:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_IDENT_CHAR; break;}
				if ((s->ch >= 'a') && (s->ch <= 'z')) {s->state = STATE_IDENT_CHAR; break;}
				if ((s->ch >= 'A') && (s->ch <= 'Z')) {s->state = STATE_IDENT_CHAR; break;}
				if ((s->ch == '.') || (s->ch == '_')) {s->state = STATE_IDENT_CHAR; break;}
				if (s->ch == '[') {s->state = 17; save_pos = s->pos; break;}
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_IDENT_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 15; break;}
			case	STATE_IDENT_END:
				c = *save_pos;
				*save_pos = 0;
				if ((w = hash_table_lookup(hash, s->start)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 16; *save_pos = c; break;}
				*save_pos = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				if (w->type == WORD_TYPE_UNDEF)
				{
					w->type = WORD_TYPE_LABEL;
					w->value_type = VALUE_TYPE_ADDR;
					w->comp_type = 0;
					w->valid = 0;
				}
				//if ((w1 = hash_table_dup_word(hash, w)) == NULL)
					//{s->state = STATE_ERROR; s->err.errnr = 26; break;}
				return w;
// ****************************** Array limit **********************************
			case	STATE_ARR_LIM_START:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_ARR_LIM_DIGIT; break;}
				if (s->ch == ']') {s->state = STATE_ARR_LIM_CLOSE_1; break;}
				{s->state = STATE_ERROR; s->err.errnr = 5; break;}
			case	STATE_ARR_LIM_DIGIT:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_ARR_LIM_DIGIT; break;}
				if (s->ch == ']') {s->state = STATE_ARR_LIM_CLOSE; break;}
				{s->state = STATE_ERROR; s->err.errnr = 17; break;}
			case	STATE_ARR_LIM_CLOSE:
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_ARR_LIM_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 18; break;}
			case	STATE_ARR_LIM_END:
				//s->start++;
				//s->pos -= 2;
				//c = *(save_pos-1);
				c = *save_pos;
				//*(save_pos-1) = 0;
				*save_pos = 0;
				//fprintf(stderr,"array limit: %s\n", s->start);
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; *(save_pos-1) = c; break;}
//				if ((w->name = names_new(s->names, s->start+1)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				//words_store_name(w, s->start+1);
				words_store_name(w, s->start);
					//{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				//*(save_pos-1) = c;
				*save_pos = c;
				c = *(save_pos -1);
				*(save_pos - 1) = 0;
				s->pos = save_pos;
				s->ch = *save_pos;
				//s->start++;
				//save_pos--;
				//c = *save_pos;
				//*save_pos = 0;
				if (sscanf(s->start+1,"%ld",&(w->value.l)) != 1)
					{s->state = STATE_ERROR; s->err.errnr = 11; *(save_pos-1) = c; break;}
				*(save_pos-1) = c;
				w->type = WORD_TYPE_ARRAY_SIZE;
				w->value_type = VALUE_TYPE_LONG;
				w->valid = -1;
				return w;
			case	STATE_ARR_LIM_CLOSE_1:
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_ARR_LIM_END_1; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 18; break;}
			case	STATE_ARR_LIM_END_1:
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; break;}
				words_store_name(w, "");
				s->pos = save_pos;
				s->ch = *save_pos;
				w->type = WORD_TYPE_ARRAY_SIZE;
				w->value_type = VALUE_TYPE_LONG;
				w->value.l = 0;
				w->valid = -1;
				return w;
			
//***************************** input operator *********************************				
			case	STATE_IN_OPERATOR_START:
				if ((s->ch >= '0') && (s->ch <= '9')) {s->state = STATE_IN_OPERATOR_START; break;}
				if (s->ch == '<') {s->state = STATE_IN_OPERATOR_CLOSE; break;}
				{s->state = STATE_ERROR; s->err.errnr = 17; break;}
			case	STATE_IN_OPERATOR_CLOSE:
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_IN_OPERATOR_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 18; break;}
			case	STATE_IN_OPERATOR_END:
				c = *(save_pos-1);
				*(save_pos-1) = 0;
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; *(save_pos-1) = c; break;}
//				if ((w->name = names_new(s->names, s->start+1)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start+1);
				if ((save_pos - s->start) > 2)
				{
					if (sscanf(s->start+1,"%ld",&(w->value.l)) != 1)
						{s->state = STATE_ERROR; s->err.errnr = 11; *(save_pos-1) = c; break;}
					w->type = WORD_TYPE_DELEYED_IN_OP;
				}
				else
				{
					w->value.l = 0;
					w->type = WORD_TYPE_IN_OP;
				}
				w->value_type = VALUE_TYPE_LONG;
				w->valid = -1;
				*(save_pos-1) = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				return w;
//***************************** output operator *********************************				
			case	STATE_OUT_OPERATOR_START:
				if (s->ch == '>') {s->state = STATE_OUT_OPERATOR_CLOSE; break;}
				{s->state = STATE_ERROR; s->err.errnr = 17; break;}
			case	STATE_OUT_OPERATOR_CLOSE:
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_OUT_OPERATOR_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 18; break;}
			case	STATE_OUT_OPERATOR_END:
				c = *save_pos;
				*save_pos = 0;
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; *save_pos = c; break;}
//				if ((w->name = names_new(s->names, s->start)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start);
				w->type = WORD_TYPE_OUT_OP;
				w->value_type = VALUE_TYPE_LONG;
				w->value.l = 0;
				w->valid = -1;
				*save_pos = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				return w;
//***************************** equ operator *********************************				
			case	STATE_EQU_OPERATOR_START:
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_EQU_OPERATOR_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 18; break;}
			case	STATE_EQU_OPERATOR_END:
				c = *save_pos;
				*save_pos = 0;
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; *save_pos = c; break;}
//				if ((w->name = names_new(s->names, s->start)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start);
				w->type = WORD_TYPE_EQU_OP;
				w->value_type = VALUE_TYPE_LONG;
				w->value.l = 0;
				w->valid = -1;
				*save_pos = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				return w;
//***************************** comment *********************************				
			case	STATE_COMMENT_START:
				//fprintf(stderr, "comment char = %d\n", (int) s->ch);
				if (s->ch == 0) {s->state = STATE_EOL; break;}
				{s->state = STATE_COMMENT_START; break;}
			case	STATE_COMMENT_END: //not used currently
				s->pos--;
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; break;}
//				if ((w->name = names_new(s->names, s->start)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start);
					//{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				w->value.p = w->name;
				w->type = WORD_TYPE_COMMENT;
				w->value_type = VALUE_TYPE_STRING;
				w->valid = -1;
				return w;
// ************************** String literal ***********************************
			case	STATE_STRING_START:
				if (s->ch == '"') {s->state = STATE_STRING_CLOSE; break;}
				if (s->ch == 0) {s->state = STATE_ERROR; s->err.errnr = 19; break;}
				{s->state = STATE_STRING_START; break;}
			case	STATE_STRING_CLOSE:
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_STRING_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 18; break;}
			case	STATE_STRING_END:
				//fprintf(stderr, "String terminating char: %d\n", (int) s->ch);
				s->start++;
				c = *(--save_pos);
				*save_pos = 0;
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; *save_pos = c; break;}
//				if ((w->name = names_new(s->names, s->start)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start);
					//{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				w->value.p = w->name;
				*save_pos++ = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				w->type = WORD_TYPE_LITERAL;
				w->value_type = VALUE_TYPE_STRING;
				w->valid = -1;
				return w;
// **************************** Line continuation ******************************
			case	STATE_LINE_CONT_START:
				if (s->ch == 0) {s->state = STATE_LINE_CONT_END; break;}
				{s->state = STATE_ERROR; s->err.errnr = 20; break;}
			case	STATE_LINE_CONT_END:
				if ((size = io_get(inp, s->line, LINE_LENGTH)) <= 0) // unexpected end of file reached
					{s->state = STATE_ERROR; s->err.errnr = 24; break;} // or error
				s->err.lineno++;
				s->pos = s->line-1;
				s->state = 0;
				s->ch = 32; // not '\0', so new char will be fetched
				break;
// *************************** Character constant ******************************
			case	STATE_CHAR_START:
				if (s->ch == 0) {s->state = STATE_ERROR; s->err.errnr = 21; break;}
				{s->state = STATE_CHAR_CHAR; break;}
			case	STATE_CHAR_CHAR:
				if (s->ch == '\'') {s->state = STATE_CHAR_CLOSE; break;}
				{s->state = STATE_ERROR; s->err.errnr = 22; break;}
			case	STATE_CHAR_CLOSE:
				if ((s->ch == 32) || (s->ch == 9) || (s->ch == ';') || (s->ch == 0) || (s->ch == '\\'))
					{s->state = STATE_CHAR_END; save_pos = s->pos; break;}
				{s->state = STATE_ERROR; s->err.errnr = 18; break;}
			case	STATE_CHAR_END:
				c = *save_pos;
				*save_pos = 0;
				if ((w = words_new(s->words)) == NULL)
					{s->state = STATE_ERROR; s->err.errnr = 9; *save_pos = c; break;}
//				if ((w->name = names_new(s->names, s->start)) == NULL)
//					{s->state = STATE_ERROR; s->err.errnr = 10; break;}
				words_store_name(w, s->start);
				*save_pos = c;
				s->pos = save_pos;
				s->ch = *save_pos;
				w->value.l = (long)(*(save_pos - 2));
				//fprintf(stderr, "char value=%ld", w->value.l);
				w->type = WORD_TYPE_LITERAL;
				w->value_type = VALUE_TYPE_LONG;
				w->valid = -1;
				//fprintf(stderr, "%s\n", w->name);
				return w;
// ***************** End of line and end of file *******************************
			case	STATE_EOL:
				s->err.errnr = SCANNER_ERROR_EOL;
				return NULL;
			case	STATE_EOF: // not used currently
				s->err.errnr = SCANNER_ERROR_EOF;
				return NULL;
// *********************** Error ***********************************************
			case	STATE_ERROR: // we can not get here
				return NULL;
		}
		if (s->state == STATE_ERROR)
		{
			if (io_get_name(inp,inp_name, IO_NAME_LENGTH + 1) != 0)
				inp_name[0] = 0;
			fprintf(stderr, "[input:%s] [line:%d] [error:%d]\n", inp_name, s->err.lineno, s->err.errnr);
			fprintf(stderr, "[source] %s\n", s->line);
			fprintf(stderr, "[description] %s\n", error_description[s->err.errnr]);
			return NULL;
		}
		else
		{
			if (s->ch != 0)
			{
				s->pos++;
				s->ch = *s->pos;
			}
		}
	}
}

int	scanner_get_line(void *scan, char *line, int len)
{
	scanner_t	*s = (scanner_t *) scan;
	
	if (strlen(s->line) > len -1) return -1;
	strcpy(line, s->line);
	return 0;
}

int scanner_delete(void *scan)
{
	//fprintf(stderr, "scanner_delete()\n");
	if (scanner_initialized == 0) return -1;
	if (scan == NULL) return 0;
	if (scanner_set == NULL) return 0;
	if (objects_verify_object(scanner_set, scan) != 0)
	{
		fprintf(stderr, "scanner_delete(): can not verify scanner object\n");
		return -1;
	}
	objects_delete_set(((scanner_t *) scan)->words);
//	objects_delete_set(((scanner_t *) scan)->names);
	objects_delete(scanner_set, scan);
	return 0;
}

int scanner_cleanup(void)
{
	scanner_t	*s;
	
	if (scanner_initialized == 0) return -1;
	if (scanner_set == NULL) return 0;
	if (objects_verify_set(scanner_set) != 0)
		fprintf(stderr, "scanner_cleanup(): Can not verify set\n");
	s = (scanner_t *) objects_get_first(scanner_set);
	//fprintf(stderr, "scanner_cleanup(): got first object\n");
	while(s != NULL)
	{
		objects_delete_set(s->words);
//		objects_delete_set(s->names);
		s = (scanner_t *) objects_get_next(scanner_set, s);
		//fprintf(stderr, "scanner_cleanup(): got next object\n");
	}
	objects_delete_type(scanner_type_index);
	//fprintf(stderr, "scanner_cleanup(): type deleted\n");
//	scanner_type_index = -1;
	scanner_set = NULL;
	scanner_initialized = 0;
	return 0;
}
