/*******************************************************************************
 * 							Common location
 * *****************************************************************************
 * 	Filename:		location.c
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

#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	"objects.h"
#include	"io.h"
#include	"location.h"

#define	ALLOC_COUNT	16

struct	_location
{
	int		type;
	char	path[IO_PATH_LENGTH + 1];
};
 
typedef	struct _location	location_t;

static int	location_initialized = 0;

int	location_init(void)
{
	if (location_initialized != 0) return -1;
	location_initialized = -1;
	return 0;
}

void	*location_new_set(void)
{
	return objects_new_set(OBJECTS_TYPE_LOCATION);	
}

int	location_add(void *set, int type, char *path)
{
	location_t	*loc;
	
	if (strlen(path) > IO_PATH_LENGTH) return -1;
	loc = (location_t *) objects_new(set, sizeof(location_t));
	if (loc == NULL) return -1;
	strcpy(loc->path, path);
	loc->type = type;
	return 0;
}

int	location_delete(void *set, int type, char *path)
{
	location_t	*loc;
	
	loc = objects_get_first(set);
	while (loc != NULL)
	{
		if ((loc->type == type) && (strcmp(loc->path, path) == 0))
		{
			objects_delete(set, loc);
			return 0;
		}
		loc = objects_get_next(set, loc);
	}
	return -1;
}

int	location_find(void *set, char *name, int item_type, int *type, char **path)
{
	location_t	*loc;
	
	loc = objects_get_first(set);
	while (loc != NULL)
	{
//		fprintf(stderr, "location_find(): searching location %s for %s\r\n", loc->path, name);		
		if (io_verify(loc->type, loc->path, name, item_type) == 0)
		{
			*type = loc->type;
			*path = &(loc->path[0]);
			return 0;
		}
		loc = objects_get_next(set, loc);
	}
	return -1;
}

int	location_delete_set(void *set)
{
//	fprintf(stderr, "location_delete_set()\r\n");
	return objects_delete_set(set);
}

int	location_cleanup(void)
{
	return objects_delete_type(OBJECTS_TYPE_LOCATION);
}
