/*******************************************************************************
 * 							Common words
 * *****************************************************************************
 * 	Filename:		words.c
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
#include	"objects.h"
#include	"words.h"

//#define	WORDS_ALLOC_SIZE	512

static int	words_initialized = 0;
static unsigned int	words_type_index = (unsigned int) OBJECTS_TYPE_WORD;

int	words_init(void)
{
	if (words_initialized == -1)
	{
		fprintf(stderr, "error: words_init(): Already initialized\n");
		return -1;
	}
//	if ((words_type_index = objects_register_type(sizeof(word_t), WORDS_ALLOC_SIZE)) < 0)
//	{
//		fprintf(stderr, "error: words_init(): Can not register object type\n\n");
//		return -1;
//	}
	words_type_index = (unsigned int) OBJECTS_TYPE_WORD;
	words_initialized = -1;
	return 0;
}

void	*words_new_set(void)
{	
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_new_set(): Not initialized\n");
		return NULL;
	}
	return objects_new_set(words_type_index);
}

word_t *words_new(void *set)
{
	word_t	*w;
	
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_new(): Not initialized\n");
		return NULL;
	}
	w = (word_t *) objects_new(set, sizeof(word_t));
	if (w != NULL)
	{
		w->root = NULL;
		w->type = WORD_TYPE_UNDEF;
//		w->clone = 0;
		w->valid = 0;
		w->value_type = VALUE_TYPE_NULL;
		w->comp_type = 0;
		w->value.l = 0;
		w->next_hash = NULL;
		w->name[0] = 0;
		w->size = 0;
	}
	else
	{
		fprintf(stderr, "error: words_new(): Can not create new word\n");
	}
	return w;
}

int	words_get_name(void *word, char *str, int len)
{
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_get_name(): Not initialized\n");
		return -1;
	}
	if (word == NULL)
	{
		fprintf(stderr, "error: words_get_name(): The word is NULL\n");
	}
	if (len < (WORD_NAME_LENGTH + 1))
	{
		fprintf(stderr, "error: words_get_name(): The buffer for the name is too short\n");
		return -1;
	}
	memcpy(str, ((word_t *)word)->name, WORD_NAME_LENGTH + 1);
	return 0;
}

void	words_store_name(word_t *w, char *name)
{
	int	len;
	
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_store_name(): Not initialized\n");
		return;
	}
	if ((len = strlen(name)) > WORD_NAME_LENGTH)
	{
		len = WORD_NAME_LENGTH;
		fprintf(stderr, "error: words_store_name(): The name has been truncated\n");
	}
	memcpy(w->name, name, len);
	w->name[len] = 0;
	w->size = len;
}

void	words_prefix_name(word_t *w, char *prefix)
{
	int		len;
	char	tmp[WORD_NAME_LENGTH + 1];
	
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_prefix_name(): Not initialized\n");
		return;
	}
	if ((len = strlen(w->name) + strlen(prefix)) > WORD_NAME_LENGTH)
	{
		len = WORD_NAME_LENGTH;
		fprintf(stderr, "error: words_prefix_name(): The name has been truncated\n");
	}
	strcpy(tmp,prefix);
	memcpy(tmp + strlen(tmp), w->name, len - strlen(prefix));
	tmp[len] = 0;
	strcpy(w->name,tmp);
	return;
}

void	words_postfix_name(word_t *w, char *postfix)
{
	int		len;
	
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_prefix_name(): Not initialized\n");
		return;
	}
	if ((len = strlen(w->name) + strlen(postfix)) > WORD_NAME_LENGTH)
	{
		len = WORD_NAME_LENGTH;
		fprintf(stderr, "error: words_prefix_name(): The name has been truncated\n");
	}
	memcpy(w->name + strlen(w->name), postfix, len - strlen(w->name));
	w->name[len] = 0;
	return;
}

word_t	*words_dup_word(void *set, word_t *word)
{
	word_t	*w;
	
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_dup_word(): Not initialized\n");
		return NULL;
	}
	if ((w = (word_t *) objects_new(set, sizeof(word_t))) == NULL)
	{
		fprintf(stderr, "error: words_dup_word(): Can not create new word\n");
		return NULL;
	}
	memcpy((void *) w, (void *) word, sizeof(word_t));
	return w;
}

word_t	*words_clone_word(void *set, word_t *word)
{
	word_t	*w;
	
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_clone_word(): Not initialized\n");
		return NULL;
	}
	if ((w = words_dup_word(set, word)) == NULL)
	{
		fprintf(stderr, "error: words_clone_word(): Can not create duplicated word\n");
		return NULL;
	}
	if (word->root == NULL)
	{
		w->root = word;
//		w->value_type = VALUE_TYPE_PTR;
//		w->value.p = word;
	}
	return w;
}

void	*words_clone_set(void *set)
{
	void	*clone = NULL;
	void	*w;
	
	if(words_initialized == 0)
	{
		fprintf(stderr, "error: words_clone_set(): Not initialized\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: words_clone_set(): Set is NULL\n");
		return NULL;
	}
	if ((clone = objects_new_set(words_type_index)) == NULL)
	{
		fprintf(stderr, "error: words_clone_set(): Can not create new set\n");
		return NULL;
	}
	w = objects_get_first(set);
	while (w)
	{
		if (words_clone_word(clone, w) == NULL)
		{
			fprintf(stderr, "error: words_clone_set(): Can not create cloned word\n");
			words_delete_set(clone);
			return NULL;
		}
		w = objects_get_next(set, w);
	}
	return clone;
}

void	*words_dup_set(void *set)
{
	void	*dup_set = NULL;
	void	*w;
	
	if(words_initialized == 0)
	{
		fprintf(stderr, "error: words_dup_set(): Not initialized\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: words_dup_set(): Set is NULL\n");
		return NULL;
	}
	if ((dup_set = objects_new_set(words_type_index)) == NULL)
	{
		fprintf(stderr, "error: words_dup_set(): Can not create new set\n");
		return NULL;
	}
	w = objects_get_first(set);
	while (w)
	{
		if (words_dup_word(dup_set, w) == NULL)
		{
			fprintf(stderr, "error: words_clone_set(): Can not create duplicated word\n");
			words_delete_set(dup_set);
			return NULL;
		}
		w = objects_get_next(set, w);
	}
	return dup_set;
}



int	words_delete(void *set, word_t *word)
{
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_delete(): Not initialized\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: words_delete(): The set is NULL\n");
		return -1;		
	}
	if (word == NULL) return 0;
	return objects_delete(set, (void *) word);
}

int words_delete_set(void *set)
{
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_delete_set(): Not initialized\n");
		return -1;
	}
	if (set == NULL) return 0;
	return objects_delete_set(set);
}

int	words_cleanup(void)
{
	if (words_initialized == 0)
	{
		fprintf(stderr, "error: words_cleanup(): Not initialized\n");
		return -1;
	}
	if (objects_delete_type(words_type_index) != 0)
	{
		fprintf(stderr, "error: words_cleanup(): Can not delete object type\n");
		return -1;
	}
//	words_type_index = -1;
	words_initialized = 0;
	return 0;
}
