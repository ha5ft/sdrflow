/*******************************************************************************
 * 							Common objects
 * *****************************************************************************
 * 	Filename:		objects.c
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
#include	"objects.h"

//#define	MAX_TYPE_INDEX	63
//#define	SET_ALLOC_COUNT	64

typedef struct	_link
{
	struct _link	*prev;
	struct _link	*next;
}link_t;

//typedef struct _object_type
//{
//	int		block_size;
//	int		obj_size;
//	int		alloc_count;
//	link_t	*first_block;
//	link_t	*first_free;	
//} object_type_t;

typedef struct _object_set
{
	unsigned int	type_index;
	link_t			*first;
	link_t			*last;
} object_set_t;

static	int				initialized = 0;
static	object_set_t	sets; // this is a set of object_set_t type of objects
//static	object_type_t	types[MAX_TYPE_INDEX + 1];

//static int	_objects_new_block(int type_index);
static int	_objects_verify_set(object_set_t *s);
static int	_objects_verify_object(object_set_t *s, void *obj);
void 		_objects_delete(object_set_t *s, void *obj);
void		_objects_delete_set(object_set_t *s);

/* *****************************************************************************
 * 							Public functions
 * ****************************************************************************/

//int		objects_register_type(int obj_size, int alloc_count)
//{
//	int	type_index;
//	
//	if (initialized == 0)
//	{
//		fprintf(stderr, "error: objects_register_type(): Not initialized\r\n");
//		return -1;
//	}
//	type_index = -1;
//	for (type_index = 0; type_index <= MAX_TYPE_INDEX; type_index++)
//	{
//		if (types[type_index].block_size == -1) break;
//	}
//	if (type_index < MAX_TYPE_INDEX)
//	{
//		types[type_index].block_size = sizeof(link_t) + (obj_size + sizeof(link_t)) * alloc_count;
//		types[type_index].alloc_count = alloc_count;
//		types[type_index].obj_size = obj_size;
//		types[type_index].first_block = NULL;
//		types[type_index].first_free = NULL;
//		if (_objects_new_block(type_index) == -1) 
//		{
//			types[type_index].block_size = -1; 
//			type_index = -1;
//			fprintf(stderr, "error: objects_register_type(): Can not allocate new block\r\n");
//		}
//	}
//	else
//	{
//		fprintf(stderr, "error: objects_register_type(): Too many types\r\n");
//	}
//	return type_index;
//}

int objects_init(void)
{
//	int i;
	
	if (initialized != 0)
	{
		fprintf(stderr, "error: objects_init(): Already initialized\r\n");
		return -1;
	}
	initialized = -1;
//	for (i = 0; i <= MAX_TYPE_INDEX; i++)
//	{
//		types[i].block_size = -1;
//	}
//	sets.type_index = objects_register_type(sizeof(object_set_t), SET_ALLOC_COUNT);
	sets.type_index = (unsigned int) OBJECTS_TYPE_OBJECT_SET;
//	if (sets.type_index != -1)
//	{
		sets.first = NULL;
		sets.last = NULL;
//	}
//	else
//	{
//		fprintf(stderr, "error: objects_init(): Can not register the object set type\r\n");
//		initialized = 0;
//	}
//	return sets.type_index;
	return 0;
}

int	objects_is_initialized(void)
{
	return initialized;
}

int		objects_verify_set(void *set)
{
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_verify_set(): Not initialized\r\n");
		return -1;
	}
	return _objects_verify_set((object_set_t *) set);
}

int		objects_verify_object(void *set, void *obj)
{
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_verify_object(): Not initialized\r\n");
		return -1;
	}
	if (_objects_verify_set((object_set_t *) set) != 0) 
	{
		fprintf(stderr, "error: objects_verify_object(): Can not verify object set\r\n");
		return -1;
	}
	return _objects_verify_object((object_set_t *) set, obj);
}

//int	objects_object_size(void *set)
//{
//	if (initialized == 0)
//	{
//		fprintf(stderr, "error: objects_object_size(): Not initialized\r\n");
//		return -1;
//	}
//	if (set == NULL)
//	{
//		fprintf(stderr, "error: objects_object_size(): Set is NULL\r\n");
//		return -1;
//	}
//	if (_objects_verify_set((object_set_t *) set) != 0)
//	{
//		fprintf(stderr, "error: objects_object_size(): Can not verify object set\r\n");
//		return -1;
//	}
//	return types[((object_set_t *) set)->type_index].obj_size;
//}

int	objects_get_set_size(void *set)
{
	link_t	*lnk;
	int		size;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_get_set_size(): Not initialized\r\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: objects_get_set_size(): Set is NULL\r\n");
		return -1;
	}
	if (_objects_verify_set((object_set_t *) set) != 0)
	{
		fprintf(stderr, "error: objects_get_set_size(): Can not verify object set\r\n");
		return -1;
	}
	lnk = ((object_set_t *) set)->first;
	size = 0;
	while (lnk != NULL)
	{
		size++;
		lnk = lnk->next;
	}
	return size;
}

void	*objects_new(void *set, size_t size)
{
	link_t 			*lnk;
	object_set_t	*s = (object_set_t *) set;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_new(): Not initialized\r\n");
		return NULL;
	}
	if (set == NULL) 
	{
		fprintf(stderr, "error: objects_new(): the set pointer is NULL\r\n");
		return NULL;
	}
	if (_objects_verify_set(s) != 0)
	{
		fprintf(stderr, "error: objects_new(): Can not verify set\r\n");
		return NULL;
	}
//	if ((lnk = types[s->type_index].first_free) == NULL)
//	{
//		if (_objects_new_block(s->type_index) == 0)
//		{
//			lnk = types[s->type_index].first_free;
//		}
//		else
//		{
//			fprintf(stderr, "error: objects_new(): Can not allocate new block\r\n");
//			return NULL;
//		}
//	}
//	if(lnk->next != NULL)
//		lnk->next->prev = NULL;
//	types[s->type_index].first_free = lnk->next;
	
	lnk = malloc(size + sizeof(link_t));
	if (lnk == NULL)
	{
		fprintf(stderr, "error: objects_new(): Can not allocate new object\r\n");
		return NULL;
	}
	if (s->first == NULL)
	{
		s->first = s->last = lnk;
		lnk->prev = lnk->next = NULL;
	}
	else
	{
		s->last->next = lnk;
		lnk->prev = s->last;
		lnk->next = NULL;
		s->last = lnk;
	}
	lnk += 1;
	return (void *) lnk;
}

void	*objects_new_set(unsigned int type_index)
{
	object_set_t	*s;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_new_set(): Not initialized\r\n");
		return NULL;
	}
//	if ((type_index < 0) || (type_index > MAX_TYPE_INDEX))
//	{
//		fprintf(stderr, "error: objects_new_set(): Wrong type index value\r\n");
//		return NULL;
//	}
//	if ((type_index == sets.type_index) || (types[type_index].block_size == -1))
//	{
//		fprintf(stderr, "error: objects_new_set(): Not a registered type index\r\n");
//		return NULL;
//	}
	if ((s = (object_set_t *) objects_new((void *) &sets, sizeof(object_set_t) + sizeof(link_t))) != NULL)
	{
		s->type_index = type_index;
		s->first = s->last = NULL;
//		if (type_index == OBJECTS_TYPE_POINTER)
//		{
//			fprintf(stderr, "pointer set = %ld\r\n", (long) s);
//		}
	}
	else
	{
		fprintf(stderr, "error: object_new_set(): Can not create new set object\r\n");
	}
	return (void *) s;
}

void	*objects_get_first(void *set)
{
	link_t 	*lnk;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_get_first(): Not initialized\r\n");
		return NULL;
	}
	if (set == NULL) 
	{
		fprintf(stderr, "error: objects_get_first(): The set pointer is NULL\r\n");
		return NULL;
	}
	if (_objects_verify_set((object_set_t *) set) != 0)
	{
		fprintf(stderr, "error: objects_get_first(): Can not verify set\r\n");
		return NULL;
	}
	lnk = ((object_set_t *) set)->first;
	if (lnk == NULL) return NULL;
	return (void *)(lnk + 1);
}

void	*objects_get_last(void *set)
{
	link_t 	*lnk;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_get_last(): Not initialized\r\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: objects_get_last(): The set pointer is NULL\r\n");
		return NULL;
	}
	if (_objects_verify_set((object_set_t *) set) != 0)
	{
		fprintf(stderr, "error: objects_get_last(): Can not verify set\r\n");
		return NULL;
	}
	lnk = ((object_set_t *) set)->last;
	if (lnk == NULL) return NULL;
	return (void *)(lnk + 1);
}

void	*objects_get_prev(void *set, void *current)
{
	link_t 	*lnk;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_get_prev(): Not initialized\r\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: objects_get_prev(): The set pointer is NULL\r\n");
		return NULL;
	}
	if (_objects_verify_set((object_set_t *) set) != 0)
	{
		fprintf(stderr, "error: objects_get_prev(): Can not verify set\r\n");
		return NULL;
	}
	if (_objects_verify_object((object_set_t *) set, current) != 0)
	{
		fprintf(stderr, "error: objects_get_prev(): Can not verify object\r\n");
		return NULL;
	}
	lnk = (link_t *) current -1;
	if (lnk->prev == NULL) return NULL;
	return (void *)(lnk->prev + 1);
}

void	*objects_get_next(void *set, void *current)
{
	link_t 	*lnk;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_get_next(): Not initialized\r\n");
		return NULL;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: objects_get_next(): The set pointer is NULL\r\n");
		return NULL;
	}
	if (_objects_verify_set((object_set_t *) set) != 0)
	{
		fprintf(stderr, "error: objects_get_next(): Can not verify set\r\n");
		return NULL;
	}
	if (_objects_verify_object((object_set_t *) set, current) != 0)
	{
		fprintf(stderr, "error: objects_get_next(): Can not verify object\r\n");
		return NULL;
	}
	lnk = (link_t *) current -1;
	if (lnk->next == NULL) return NULL;
	return (void *)(lnk->next + 1);
}

int 	objects_delete(void *set, void *obj)
{
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_delete(): Not initialized\r\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: objects_delete(): The set pointer is NULL\r\n");
		return -1;
	}
	if (_objects_verify_set((object_set_t *) set) != 0)
	{
		fprintf(stderr, "error: objects_delete(): Can not verify set\r\n");
		return -1;
	}
	if (_objects_verify_object((object_set_t *) set, obj) != 0)
	{
		fprintf(stderr, "error: objects_delete(): Can not verify object\r\n");
		return -1;
	}
	_objects_delete((object_set_t *) set, obj);
	return 0;
}


int	objects_delete_set(void *set)
{
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_delete_set(): Not initialized\r\n");
		return -1;
	}
	if (set == NULL)
	{
		fprintf(stderr, "error: objects_delete_set(): The set pointer is NULL\r\n");
		return -1;
	}
	if (set == (void *) &sets)
	{
		fprintf(stderr, "error: objects_delete_set(): Can not delete set of sets\r\n");
		return -1;
	}
	if (_objects_verify_set((object_set_t *) set) != 0)
	{
		fprintf(stderr, "error: objects_delete_set(): Can not verify set\r\n");
		return -1;
	}
	_objects_delete_set((object_set_t *) set);
	return 0;
}

void	*objects_get_first_set(unsigned int type_index)
{
	object_set_t	*s;
	link_t			*lnk;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_get_first_set(): Not initialized\r\n");
		return NULL;
	}
//	if ((type_index < 0) || (type_index > MAX_TYPE_INDEX))
//	{
//		fprintf(stderr, "error: objects_get_first_set(): Bad set type value\r\n");
//		return NULL;
//	}
//	if ((type_index == sets.type_index) || (types[type_index].block_size == -1))
//	{
//		fprintf(stderr, "error: objects_get_first_set(): Not a registered set type\r\n");
//		return NULL;
//	}
	lnk = sets.first;
	while (lnk)
	{
		s = (object_set_t *)(lnk + 1);
		lnk = lnk->next;
		if (s->type_index == type_index)
		{
			return (void *)s;
		}
	}
	return NULL;
}

void	*objects_get_next_set(unsigned int type_index, void *set)
{
	object_set_t	*s = (object_set_t *) set;
	link_t			*lnk;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_get_next_set(): Not initialized\r\n");
		return NULL;
	}
	lnk = ((link_t *) set) -1;
//	if ((type_index < 0) || (type_index > MAX_TYPE_INDEX))
//	{
//		fprintf(stderr, "error: objects_get_next_set(): Bad set type value\r\n");
//		return NULL;
//	}
//	if ((type_index == sets.type_index) || (types[type_index].block_size == -1))
//	{
//		fprintf(stderr, "error: objects_get_next_set(): Not a registered set type\r\n");
//		return NULL;
//	}
	if (_objects_verify_set(s) != 0)
	{
		fprintf(stderr, "error: objects_get_next_set(): Can not verify set\r\n");
		return NULL;
	}
	if (s->type_index != type_index)
	{
		fprintf(stderr, "error: objects_get_next_set(): Wrong set type\r\n");
		return NULL;
	}
	lnk = lnk->next;
	while (lnk)
	{
		s = (object_set_t *)(lnk + 1);
		if (s->type_index == type_index)
		{
			return (void *)s;
		}
		lnk = lnk->next;
	}
	return NULL;
}


int	objects_delete_type(unsigned int type_index)
{
	object_set_t	*s;
	link_t			*lnk/*, *lnk1*/;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_delete_type(): Not initialized\r\n");
		return -1;
	}
//	if ((type_index < 0) || (type_index > MAX_TYPE_INDEX))
//	{
//		fprintf(stderr, "error: objects_delete_type(): Bad set type value\r\n");
//		return -1;
//	}
//	if ((type_index == sets.type_index) || (types[type_index].block_size == -1))
//	{
//		fprintf(stderr, "error: objects_delete_type(): Not a registered set type\r\n");
//		return -1;
//	}
	lnk = sets.first;
	while (lnk)
	{
		s = (object_set_t *)(lnk + 1);
		lnk = lnk->next;
		if (s->type_index == type_index)
		{
			_objects_delete_set(s);
		}
	}
//	lnk = types[type_index].first_block;
//	while (lnk)
//	{
//		lnk1 = lnk;
//		lnk = lnk->next;
//		free(lnk1);
//	}
//	types[type_index].block_size = -1;
	return 0;
}

void	objects_cleanup()
{
	object_set_t	*s;
	link_t			*lnk;
//	link_t			*lnk1;
//	int				i;
	
	if (initialized == 0)
	{
		fprintf(stderr, "error: objects_cleanup(): Not initialized\r\n");
		return;
	}
//	for (i = 0; i <= MAX_TYPE_INDEX; i++)
//	{
//		if (types[i].block_size != -1)
//		{
//			lnk = types[i].first_block;
//			while (lnk)
//			{
//				lnk1 = lnk;
//				lnk = lnk->next;
//				free(lnk1);
//			}
//			types[i].block_size = -1;
//		}
//	}
	lnk = sets.first;
	while (lnk)
	{
		s = (object_set_t *)(lnk + 1);
		lnk = lnk->next;
		_objects_delete_set(s);
	}	
	sets.type_index = (unsigned int) OBJECTS_TYPE_NULL;
	sets.first = sets.last = NULL;
	initialized = 0;
}

/* *****************************************************************************
 * 							Private functions
 * ****************************************************************************/

//static int _objects_new_block(int type_index)
//{
//	link_t	*lnk;
//	int		i, j;
//	object_type_t *t = &types[type_index];
//	
//	if ((lnk = (link_t *)malloc(t->block_size)) != NULL)
//	{
//		lnk->prev = NULL;
//		if (t->first_block == NULL)
//		{
//			lnk->next = NULL;
//		}
//		else
//		{
//			lnk->next = t->first_block;
//			t->first_block->prev = lnk;
//		}
//		t->first_block = lnk;
//		lnk += 1;
//		j = 0;
//		if (t->first_free == NULL)
//		{
//			t->first_free = lnk;
//			lnk->prev = lnk->next = NULL;
//			lnk += 1;
//			lnk = (link_t *)(((void *) lnk) + t->obj_size);
//			j = 1;
//		}
//		for (i = j; i < t->alloc_count; i++)
//		{
//			t->first_free->prev = lnk;
//			lnk->next = t->first_free;
//			lnk->prev = NULL;
//			t->first_free = lnk;
//			lnk += 1;
//			lnk = (link_t *)(((void *) lnk) + t->obj_size);
//		}
//		return 0;
//	}
//	fprintf(stderr, "_object_new_block(): Can not allocate new block\r\n");
//	return -1;
//}

static int	_objects_verify_set(object_set_t *s)
{
	link_t	*lnk;
	
	if (s == &sets) return 0;
	lnk = sets.first;
	while (lnk != NULL)
	{
		if (s == (object_set_t*)(lnk + 1)) return 0;
		lnk = lnk->next;
	}
	fprintf(stderr, "_object_verify_set(): Can not verify object set\r\n");
	return -1;
}

static int	_objects_verify_object(object_set_t *s, void *obj)
{
	link_t 	*lnk;

	lnk = s->first;
	while (lnk != NULL)
	{
		if ((void *)(lnk + 1) == obj) return 0;
		lnk = lnk->next;
	}
	fprintf(stderr, "_object_verify_object(): Can not verify object\r\n");
	return -1;
}

void 	_objects_delete(object_set_t *s, void *obj)
{
	link_t	*lnk = ((link_t *) obj) - 1;

	if (s->first == lnk)
	{
		if (s->last == lnk)
		{
			s->first = s->last = NULL;
		}
		else
		{
			s->first = lnk->next;
			lnk->next->prev = NULL;
		}
	}
	else
	{
		if (s->last == lnk)
		{
			s->last = lnk->prev;
			lnk->prev->next = NULL;
		}
		else
		{
			lnk->prev->next = lnk->next;
			lnk->next->prev = lnk->prev;
		}
	}
//	if (types[s->type_index].first_free == NULL)
//	{
//		types[s->type_index].first_free = lnk;
//		lnk->prev = lnk->next = NULL;
//	}
//	else
//	{
//		(types[s->type_index].first_free)->prev = lnk;
//		lnk->next = types[s->type_index].first_free;
//		lnk->prev = NULL;
//		types[s->type_index].first_free = lnk;
//	}
	free((void *) lnk);
	return;
}

void	_objects_delete_set(object_set_t *s)
{
	link_t	*lnk;
	
	lnk = s->last;
	while (lnk != NULL)
	{
		_objects_delete(s, (void *)(lnk + 1));
		lnk =  s->last;
	}
	_objects_delete(&sets, (void *) s);
	return;
}

