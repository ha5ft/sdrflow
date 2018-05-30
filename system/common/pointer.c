/*******************************************************************************
 * 							Common pointer
 * *****************************************************************************
 * 	Filename:		pointer.c
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

#define	_POINTER_

#include	"objects.h"
#include	"pointer.h"

//#define		POINTER_ALLOC_SIZE		128

static unsigned int	pointer_type_index = (unsigned int) OBJECTS_TYPE_POINTER;
static int	pointer_initialized = 0;

int		pointer_initialize(void)
{
	if(pointer_initialized != 0) return -1;
	if (objects_is_initialized() == 0)
	{
		if (objects_init() != 0) return -1;
	}
//	if ((pointer_type_index = objects_register_type(sizeof(pointer_t), POINTER_ALLOC_SIZE)) < 0) return -1;
	pointer_type_index = (unsigned int) OBJECTS_TYPE_POINTER;
	pointer_initialized = -1;
	return 0;	
}

void	*pointer_new_set(void)
{
	if(pointer_initialized == 0) return NULL;
	return objects_new_set(pointer_type_index);			
}

void	*pointer_dup_set(void *set)
{
	void	*dup_set = NULL;
	void	*ptr;
	
	if(pointer_initialized == 0) return NULL;
	if (set == NULL) return NULL;
	if ((dup_set = objects_new_set(pointer_type_index)) == NULL) return NULL;
	ptr = objects_get_first(set);
	while (ptr)
	{
		if (pointer_dup(ptr, dup_set) == NULL)
		{
			pointer_delete_set(dup_set);
			return NULL;
		}
		ptr = objects_get_next(set, ptr);
	}
	return dup_set;
}


void	*pointer_new(void *set, void *ptr)
{
	pointer_t	*p;
	
	if(pointer_initialized == 0) return NULL;
	if (set == NULL) return NULL;
	p = (pointer_t *) objects_new(set, sizeof(pointer_t));
	if (p != NULL)
	{
		p->ptr = ptr;
	}
	return p;	
}

void	*pointer_dup(void *ptr, void *dup_set)
{
	void	*dup;
	
	if(pointer_initialized == 0) return NULL;
	if ((dup_set == NULL) || (ptr == NULL)) return NULL;
	if ((dup = objects_new(dup_set, sizeof(pointer_t))) == NULL) return NULL;
	memcpy(dup, ptr, sizeof(pointer_t));
	return dup;
}

void	*pointer_get_raw(void *ptr)
{
	if (ptr == NULL) return NULL;
	return ((pointer_t *)ptr)->ptr;
}

void	**pointer_get_raw_pointers(void *set)
{
	pointer_t	*ptr;
	void		**raw;
	int			len, i;
	
	if(pointer_initialized == 0) return NULL;
	if (set == NULL) return NULL;
	if ((len = objects_get_set_size(set)) == -1) return NULL;
	if (len == 0) return NULL;
	raw = (void **) malloc(len * sizeof(void *));
	if (raw == NULL) return NULL;
	i = 0;
	ptr = objects_get_first(set);
	while ((ptr != NULL) && (i < len))
	{
		raw[i] = ptr->ptr;
		i++;
		ptr = objects_get_next(set, ptr);
	}
	if (ptr != NULL)
	{
		free(raw);
		return NULL;
	}
	return raw;
}

int		pointer_delete(void *set, void *ptr)
{
	if(pointer_initialized == 0) return -1;
	return objects_delete(set, ptr);	
}

int		pointer_delete_set(void *set)
{
	if(pointer_initialized == 0) return -1;
	return objects_delete_set(set);	
}

int		pointer_cleanup(void)
{
	if(pointer_initialized == 0) return -1;
	objects_delete_type(pointer_type_index);
//	pointer_type_index = -1;
	pointer_initialized = 0;
	return 0;
}
