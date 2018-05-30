/*******************************************************************************
 * 							Common sentence
 * *****************************************************************************
 * 	Filename:		sentence.c
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
#include	"objects.h"
//#include	"../include/words.h"
#include	"pointer.h"
#include	"sentence.h"

//#define	SET_SIZE	512

typedef struct _sentence
{
	int					lineno;
	int					size;
	void				*words;
//	struct _sentence	*prev;
//	struct _sentence	*next;
//	word_t				*first_word;
//	word_t				*last_word;
} sentence_t;

static unsigned int	sentence_type_index = (unsigned int) OBJECTS_TYPE_SENTENCE;
static int			sentence_initialized = 0;

void sentence_init(void)
{
	if (sentence_initialized == 0)
	{
//		sentence_type_index = objects_register_type(sizeof(sentence_t), SET_SIZE);
		sentence_type_index = (unsigned int) OBJECTS_TYPE_SENTENCE;
		//fprintf(stderr,"debug: sentences_ini(): type_index=%d\n", sentences_type_index);
		sentence_initialized = -1;
	}
}

void	*sentence_new_set(void)
{	
	void	*set;
	
	set = objects_new_set(sentence_type_index);
	if (set == NULL)
	{
		fprintf(stderr, "error: sentences_new_set(): Can not create new sentence set\n");
	}
	return set;
}

void *sentence_new(void *set, int lineno)
{
	sentence_t	*s;
	
	s = (sentence_t *) objects_new(set, sizeof(sentence_t));
	if (s != NULL)
	{
//		s->prev = NULL;
//		s->next = NULL;
		s->lineno = lineno;
		s->size = 0;
		if ((s->words = pointer_new_set()) == NULL)
		{
			objects_delete(set, s);
			s = NULL;
			fprintf(stderr, "error: sentences_new(): Can not create new pointer set\n");
		}
	}
	else
	{
		fprintf(stderr, "error: sentences_new(): Can not create new sentence object\n");
	}
	return (void *)s;
}

int sentence_add_word(void *sent, void *word)
{
//	void	*p;
	
	if (sent == NULL)
	{
		fprintf(stderr,"error: sentences_add_word(): Sentence pointer is NULL\n");
		return -1;
	}
	if (pointer_new(((sentence_t *)sent)->words,word) == NULL)
//	if (words_clone_word(((sentence_t *)sent)->words,word) == NULL)
	{
		fprintf(stderr,"error, sentences_add_word(): Can not create a pointer object\n");
		return -1;
	}
	return 0;
}

void *sentence_get_first_word(void *sent, void **iter)
{
	void *p, *w;
	
	//fprintf(stderr,"debug: sentence_get_first_word()\n");
	if (sent == NULL)
	{
		fprintf(stderr,"error: sentences_get_first_word(): Sentence pointer is NULL\n");
		return NULL;
	}
	p = objects_get_first(((sentence_t *)sent)->words);
	//fprintf(stderr,"debug: sentence_get_first_word(): Got first pointer %ld\n",(long)p);
	*iter = p;
	if (p == NULL) return NULL;
	w = pointer_get_raw(p);
	//fprintf(stderr,"debug: sentence_get_first_word(): Got first word %ld\n",(long)w);
	return w;
}

void *sentence_get_next_word(void *sent, void **iter)
{
	void *p, *w;
	
	if (sent == NULL)
	{
		fprintf(stderr,"error: sentence_get_next_word(): Sentence pointer is NULL\n");
		return NULL;
	}
	if (*iter == NULL)
	{
		fprintf(stderr,"error: sentence_get_next_word(): Iterator is NULL\n");
		return NULL;
	}
	p = objects_get_next(((sentence_t *)sent)->words, *iter);
	//fprintf(stderr,"debug: sentence_get_next_word(): Got next pointer %ld\n",(long)p);
	*iter = p;
	if (p == NULL) return NULL;
	w = pointer_get_raw(p);
	//fprintf(stderr,"debug: sentence_get_next_word(): Got next word %ld\n",(long)w);
	return w;
}

int	sentence_set_size(void *sent, int size)
{
	if (sent == NULL)
	{
		fprintf(stderr,"error: sentence_set_size(): Sentence pointer is NULL\n");
		return -1;
	}
	if (size < 0)
	{
		fprintf(stderr,"error: sentence_set_size(): Size is negative\n");
		return -1;
	}
	((sentence_t *)sent)->size = size;
	return 0;
}

int	sentence_get_size(void *sent)
{
	if (sent == NULL)
	{
		fprintf(stderr,"error: sentence_get_size(): Sentence pointer is NULL\n");
		return -1;
	}
	return ((sentence_t *)sent)->size;
}

int	sentence_get_lineno(void *sent)
{
	if (sent == NULL)
	{
		fprintf(stderr,"error: sentence_get_lineno(): Sentence pointer is NULL\n");
		return -1;
	}
	return ((sentence_t *)sent)->lineno;
}

int sentence_delete(void *set, void * sent)
{
	int	ret;
	
	if (set == NULL)
	{
		fprintf(stderr,"error: sentence_delete(): Set pointer is NULL\n");
		return -1;
	}
	if (sent == NULL)
	{
		fprintf(stderr,"error: sentence_delete(): Sentence pointer is NULL\n");
		return -1;
	}
	if (pointer_delete_set(((sentence_t *) sent)->words) == -1)
	{
		fprintf(stderr,"error: sentence_delete(): Can not delete pointer set\n");
		objects_delete(set, (void *) sent);
		return -1;
	}
	ret = objects_delete(set, (void *) sent);
	if (ret == -1)
	{
		fprintf(stderr,"error: sentence_delete(): Can not delete sentence object\n");
		return -1;
	}
	return 0;
}

int sentence_delete_set(void *set)
{
	void	*s;
	int		ret;
	
	if (set == NULL)
	{
		fprintf(stderr,"error: sentence_delete_set(): Set pointer is NULL\n");
		return -1;
	}
	s = objects_get_first(set);
	while (s != NULL)
	{
		if (objects_delete_set(((sentence_t *)s)->words) == -1)
		{
			fprintf(stderr,"error: sentences_delete_set(): Can not delete pointer set\n");
		}
		s = objects_get_next(set,s);
	}
	ret = objects_delete_set(set);
	if (ret == -1)
	{
		fprintf(stderr,"error: sentence_delete_set(): Can not delete sentence set\n");
		return -1;
	}
	return 0;
}

int	sentence_cleanup(void)
{
//	void	*set, *s;
	int		ret;
	
	if (sentence_initialized == 0)
	{
		fprintf(stderr,"error: sentence_cleanup(): Not initialized\n");
		return -1;
	}
//	set = objects_get_first_set(sentence_type_index);
//	while (set != NULL)
//	{
//		s = objects_get_first(set);
//		while (s != NULL)
//		{
//			if (objects_delete_set(((sentence_t *)s)->words) == -1)
//			{
//				fprintf(stderr,"error: sentence_cleanup(): Can not delete pointer set\n");
//			}
//			s = objects_get_next(set,s);
//		}
//		set = objects_get_next_set(sentence_type_index, set);
//	}
	ret = objects_delete_type(sentence_type_index);
	if (ret == -1)
	{
		fprintf(stderr,"error: sentence_cleanup(): Can not delete object type\n");
		return -1;
	}
	sentence_initialized = 0;
	return 0;
}
