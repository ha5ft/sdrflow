/*******************************************************************************
 * 							Common hash_table
 * *****************************************************************************
 * 	Filename:		hash_table.c
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
#include	"hash_table.h"
#include	"objects.h"
#include	"words.h"
//#include	"names.h"

#define	TABLE_SIZE				256
#define HASH_SEED				5381
//#define	HASH_TABLE_ALLOC_SIZE	1

typedef	struct _hash_table
{
	void	*words;
//	void	*names;
	word_t	*table[TABLE_SIZE];
} hash_table_t;

static int	hash_table_initialized = 0;
static unsigned int	hash_table_type_index = (unsigned int) OBJECTS_TYPE_HASH_TABLE;
static void	*hash_table_set = NULL;

int hash_table_init(void)
{
	if (hash_table_initialized != 0) return -1;
	if (objects_is_initialized() == 0)
	{
		if (objects_init() != 0) return -1;
	}
//	if ((hash_table_type_index = objects_register_type(sizeof(hash_table_t), HASH_TABLE_ALLOC_SIZE)) < 0) return -1;
	hash_table_type_index = (unsigned int) OBJECTS_TYPE_HASH_TABLE;
	if ((hash_table_set = objects_new_set(hash_table_type_index)) == NULL)
	{
		objects_delete_type(hash_table_type_index);
		hash_table_type_index = -1;
		return -1;
	}
	hash_table_initialized = -1;
	return 0;
}

void	*hash_table_new(void)
{
	hash_table_t	*t = NULL;
	int				i;
	
	if (hash_table_initialized == 0) return NULL;
	if ((t = (hash_table_t *) objects_new(hash_table_set, sizeof(hash_table_t))) != NULL)
	{
		if ((t->words = words_new_set()) != NULL)
		{
//			if ((t->names = names_new_set()) != NULL)
//			{
				for (i = 0; i < TABLE_SIZE; i++) t->table[i] = NULL;
//			}
//			else
//			{
//				objects_delete_set(t->words);
//				objects_delete(hash_table_set, t);
//			}
		}
		else
		{
			objects_delete(hash_table_set, t);
		}
	}
	return t;
}

int hash_table_hash(char *str)
{
	unsigned int h = HASH_SEED;
	int c;

	while ((c = *str++) != 0)
	{
		h = ((h << 5) + h) + c;
	}
	h &= (TABLE_SIZE -1);
	return h;
}

word_t *hash_table_check(void *table, char *str)
{
	hash_table_t	*t = (hash_table_t *) table;
	unsigned int	h;
	word_t			*c/*,*p*/;
	int				s;
	
	if (hash_table_initialized == 0) return NULL;
	if ((s = strlen(str)) > WORD_NAME_LENGTH)
		return NULL;
	h = hash_table_hash(str); // hash value for the string
	c = t->table[h]; // current word
//	p = NULL; // previous word
	while (c != NULL)
	{
		if (c->size == s)
		{
			if (strcmp(c->name,str) == 0)
			{
				return c;
			}
		}
//		p = c;
		c = c->next_hash;
	}
	return c;
}

word_t *hash_table_lookup(void *table, char *str)
{
	hash_table_t	*t = (hash_table_t *) table;
	unsigned int	h;
	word_t			*c,*p;
	int				s;
	
	if (hash_table_initialized == 0) return NULL;
	if ((s = strlen(str)) > WORD_NAME_LENGTH)
		return NULL;
	h = hash_table_hash(str); // hash value for the string
	c = t->table[h]; // current word
	p = NULL; // previous word
	while (c != NULL)
	{
		if (c->size == s)
		{
			if (strcmp(c->name,str) == 0)
			{
				return c;
			}
		}
		p = c;
		c = c->next_hash;
	}
	if ((c = words_new(t->words)) != NULL)
	{
//		if ((c->name = names_new(t->names, str)) != NULL)
//		{
			words_store_name(c, str);
			c->size = s;
			c->next_hash = NULL;
			if (p != NULL)
			{
				p->next_hash = c;
			}
			else
			{
				t->table[h] = c;
			}
			return c;
//		}
//		else
//		{
//			words_delete(c);
//		}
	}
	return NULL;
}
/*
word_t	*hash_table_get_empty_word(void *table)
{
	return words_new(((hash_table_t *) table)->words);
}

word_t	*hash_table_dup_word(void *table, word_t *word)
{
	return words_dup_word(((hash_table_t *) table)->words, word);
}
*/
int	hash_table_delete(void *table)
{
	//fprintf(stderr, "hash_table_delete()\n");
	if (hash_table_initialized == 0) return -1;
	if (table == NULL) return 0;
	if (hash_table_set == NULL) return 0;
	if (objects_verify_object(hash_table_set, table) != 0) 
	{
		fprintf(stderr, "hash_table_delete(): can not verify hash table object\n");
		return -1;
	}
	objects_delete_set(((hash_table_t *) table)->words);
//	objects_delete_set(((hash_table_t *) table)->names);
	objects_delete(hash_table_set, table);
	return 0;
}

int	hash_table_cleanup(void)
{
	hash_table_t	*t;
	
	if (hash_table_initialized == 0) return -1;
	if (hash_table_set == NULL) return 0;
	t = (hash_table_t *) objects_get_first(hash_table_set);
	while(t != NULL)
	{
		objects_delete_set(t->words);
//		objects_delete_set(t->names);
		t = (hash_table_t *) objects_get_next(hash_table_set, t);
	}
	objects_delete_type(hash_table_type_index);
//	hash_table_type_index = -1;
	hash_table_set = NULL;
	hash_table_initialized = 0;
	return 0;
}

void hash_table_print(void *table)
{
	hash_table_t	*t = (hash_table_t *) table;
	int 	i;
	word_t	*w;
	
	if (hash_table_initialized == 0) return;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		w = t->table[i];
		if (w == NULL)
		{
			printf ("[%03d]\tNULL\n",i);
		}
		else
		{
			while (w != NULL)
			{
				printf("[%03d]\t%s, type=%d\n",i,w->name,w->type);
				w = w->next_hash;
			}
		}
	}
}
