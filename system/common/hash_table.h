/*******************************************************************************
 * 							Common hash_table
 * *****************************************************************************
 * 	Filename:		hash_table.h
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

#ifndef	HASH_TABLE_H
#define	HASH_TABLE_H

#include	"words.h"

int		hash_table_init(void);
void	*hash_table_new(void);
int		hash_table_hash(char *str);
word_t	*hash_table_check(void *table, char *str);
word_t	*hash_table_lookup(void *table, char *str);
/*
word_t	*hash_table_get_empty_word(void* table);
word_t	*hash_table_dup_word(void *table, word_t *word);
*/
int		hash_table_delete(void *table);
int		hash_table_cleanup(void);
void	hash_table_print(void *table);

#endif
