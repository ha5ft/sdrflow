/*******************************************************************************
 * 							Common primitive
 * *****************************************************************************
 * 	Filename:		primitive.c
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
#include	<unistd.h>
#include	<stdio.h>
#include	<string.h>
#include	<sys/stat.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	"obj_format.h"
#include	"../../include/primitive_interface.h"
#include	"objects.h"
#include	"io.h"
#include	"location.h"
#include	"primitive.h"

/****************************** Local constants *******************************/

#define	PRIMITIVE_TYPE_BUILTIN	0
#define	PRIMITIVE_TYPE_DLL		1

//#define	PRIMITIVE_ALLOC_SIZE	32

struct _primitive
{
	char				name[MAX_NAME_LENGTH + 1];
	int					type;
	int					from_temp;
	int					use_count;
	int					flag;
	size_t				self_size;
	void				*handle;
	primitive_entry_t	entries[PRIMITIVE_ENTRY_COUNT];
};

static system_catalog_t	sys_catalog =
{
	.version = 1,
	.open_cmd = (open_cmd_channel_t) NULL,
	.read_cmd = (read_cmd_channel_t) NULL,
	.close_cmd = (close_cmd_channel_t) NULL
};

typedef struct _primitive	primitive_t;

static int	primitive_initialized = 0;
static void	*locations;
static void	*primitives;
static void	*self_objects;

static primitive_t	*_primitive_find(char *name);

/******************* Externally accessible functions **************************/

int	primitive_init()
{
	if (primitive_initialized != 0) return -1;
	if ((locations = location_new_set()) == NULL) return -1;
	if ((primitives = objects_new_set(OBJECTS_TYPE_PRIMITIVE)) == NULL)
	{
		location_delete_set(locations);
		return -1;
	}
	if ((self_objects = objects_new_set(OBJECTS_TYPE_SELF)) == NULL)
	{
		location_delete_set(locations);
		objects_delete_set(primitives);
		return -1;
	}
	if (location_add(locations, IO_CHAN_TYPE_FILE, "./actor/") == -1)
	{
		location_delete_set(locations);
		objects_delete_set(primitives);
		objects_delete_set(self_objects);
		return -1;
	}
	primitive_initialized = -1;
	return 0;
}

int	primitive_add_location(int type, char *path)
{
	return location_add(locations, type, path);
}

int	primitive_new(char *name, primitive_catalog_t *catalog)
{
	primitive_t	*p;

	if (primitive_initialized == 0) return -1;
	if ((catalog->name == NULL) || (name == NULL)) return -1;
	if (strlen((char *) catalog->name) > MAX_NAME_LENGTH) return -1;
	if (strlen(name) > MAX_NAME_LENGTH) return -1;
	if (strcmp(name, (char *) catalog->name) != 0) return -1;
	if (_primitive_find((char *) catalog->name) != NULL) return 0;
	if ((p = (primitive_t *) objects_new(primitives, sizeof(primitive_t))) == NULL) return -1;
	strcpy(p->name,(char *) catalog->name);
	p->type = PRIMITIVE_TYPE_BUILTIN;
	p->handle = NULL;
	p->flag = -1;
	p->use_count = 1;
	p->entries[PRIMITIVE_ENTRY_INIT] = catalog->init;
	p->entries[PRIMITIVE_ENTRY_FIRE] = catalog->fire;
	p->entries[PRIMITIVE_ENTRY_CLEANUP] = catalog->cleanup;
	p->entries[PRIMITIVE_ENTRY_LOAD] = catalog->load;
	p->entries[PRIMITIVE_ENTRY_DELETE] = catalog->delete;
	p->from_temp = 0;
	p->self_size = catalog->self_size;
	return 0;
}

int primitive_is_loaded(char *name)
{
	if (primitive_initialized == 0) return 0;
	if (name == NULL) return 0;
	if (strlen(name) > MAX_NAME_LENGTH) return 0;
	if (_primitive_find(name) != NULL)
		return -1;
	else
		return 0;
}

int	primitive_load(char *name)
{
	char					full_name[IO_FULL_NAME_LENGTH + 1];
	char					*path;
	int						type, ln, from_temp, ret;
	void					*handle, *res;

	primitive_t				*p;
	primitive_catalog_t		*catalog;

	if (primitive_initialized == 0) return -1;
	if (name == NULL) return -1;
	if ((ln = strlen(name)) > MAX_NAME_LENGTH) return -1;
	if ((p = _primitive_find(name)) != NULL)
	{
		if (p->flag == 0)
		{
			p->flag = -1;
			p->use_count++;
		}
		return 0;
	}
	if (location_find(locations, name, IO_ITEM_TYPE_PRIMITIVE, &type, &path) != 0)
	{
		fprintf(stderr, "primitive_load(%s): Can not find primitive file\r\n", name);		
		return -1;
	}
	from_temp = 0;
	if (type != IO_CHAN_TYPE_FILE)
	{
		if (io_copy_to_temp(type, path, name, IO_ITEM_TYPE_PRIMITIVE) != 0)
			return -1;
		path = io_get_temp_path();
		from_temp = -1;
	}
	io_get_full_name(full_name, path, name, IO_ITEM_TYPE_PRIMITIVE,IO_FULL_NAME_LENGTH + 1);
	if ((handle = dlopen(full_name, RTLD_NOW)) == NULL)
	{
		fprintf(stderr, "primitive_load(%s): Can not load %s\r\n", name, full_name);
		if (from_temp != 0) io_delete_from_temp(name, IO_ITEM_TYPE_PRIMITIVE);
		return -1;
	}
	res = dlerror();
	strcpy(full_name,name);
	strcpy(&full_name[ln], "_catalog");
	catalog = (primitive_catalog_t *) dlsym(handle, full_name);
	res = dlerror();
	if ((res != NULL) || (catalog == NULL))
	{
		fprintf(stderr, "primitive_load(%s): Can not find symbole %s\r\n", name, full_name);
		dlclose(handle);
		if (from_temp != 0) io_delete_from_temp(name, IO_ITEM_TYPE_PRIMITIVE);
		return -1;
	}
	if (primitive_new(name, catalog) != 0)
	{
		fprintf(stderr, "primitive_load(%s): Failed to register enries\r\n", name);
		dlclose(handle);
		if (from_temp != 0) io_delete_from_temp(name, IO_ITEM_TYPE_PRIMITIVE);
		return -1;
	}
	if ((p = _primitive_find(name)) == NULL)
	{
		fprintf(stderr, "primitive_load(%s): Can not find loaded primitive\r\n", name);
		dlclose(handle);
		if (from_temp != 0) io_delete_from_temp(name, IO_ITEM_TYPE_PRIMITIVE);
		return -1;
	}
	p->type = PRIMITIVE_TYPE_DLL;
	p->handle = handle;
	p->from_temp = from_temp;
	ret = (*(p->entries[PRIMITIVE_ENTRY_LOAD]))(&sys_catalog);
	if (ret != 0)
	{
		fprintf(stderr, "primitive_load() : Primitive %s load function returned error\r\n", p->name);
		return -1;
	}
	return 0;
}

void	primitive_remove_not_used()
{
	primitive_t	*p, *n;

	p = (primitive_t *) objects_get_first(primitives);
	while (p != NULL)
	{
		n = (primitive_t*) objects_get_next(primitives, (void *) p);
		if (p->use_count <= 0)
		{
			if (p->type == PRIMITIVE_TYPE_DLL)
			{
				if( (*(p->entries[PRIMITIVE_ENTRY_DELETE]))(NULL) != 0)
					fprintf(stderr, "primitive_remove_not_used() : Primitive %s delete function returned error\r\n", p->name);
				dlclose(p->handle);
				objects_delete(primitives,(void *) p);
			}
		}
		p = n;
	}
}

void	primitive_remove_being_loaded()
{
	primitive_t	*p, *n;
//	int			ret;
	
	p = (primitive_t *) objects_get_first(primitives);
	while (p != NULL)
	{
		n = (primitive_t *) objects_get_next(primitives, (void *) p);
		if ((p->flag != 0) && (p->use_count == 1))
		{
			if (p->type == PRIMITIVE_TYPE_DLL)
			{
				if( (*(p->entries[PRIMITIVE_ENTRY_DELETE]))(NULL) != 0)
					fprintf(stderr, "primitive_remove_not_used() : Primitive %s delete function returned error\r\n", p->name);
				dlclose(p->handle);
				objects_delete(primitives,(void *) p);
			}
		}
		p = n;
	}
}

void	primitive_clear_flag()
{
	primitive_t	*p;
	
	p = (primitive_t *) objects_get_first(primitives);
	while (p != NULL)
	{
		if (p->flag != 0)
		{
			p->flag = 0;
		}
		p = (primitive_t *) objects_get_next(primitives, (void *) p);			
	}
}


primitive_entry_t	*primitive_get_entry_table(char *name)
{
	primitive_t	*p;
	
	if (primitive_initialized == 0) return NULL;
	if (name == NULL) return NULL;
	if (strlen(name) > MAX_NAME_LENGTH) return NULL;
	if ((p = _primitive_find(name)) == NULL) return NULL;
	return &p->entries[0];
}

int	primitive_create_self(char *name, char *instance_name, void **self)
{
	primitive_t	*p;
	void		*s;
	
	*self = NULL;
	if (primitive_initialized == 0) return -1;
	if (name == NULL) return -1;
	if (strlen(name) > MAX_NAME_LENGTH) return -1;
	if ((p = _primitive_find(name)) == NULL) return -1;
	if (p->self_size == 0) return 0;
	if ((s = objects_new(self_objects, p->self_size)) == NULL) return -1;
	((default_self_t *) s)->instance_name = instance_name;
	*self = s;
	return 0;
}

int primitive_delete_self(void *self)
{
	return objects_delete(self_objects, self);
}


int	primitive_delete(char *name)
{
	primitive_t	*p;

	if ((p = _primitive_find(name)) == NULL)
	{
		fprintf(stderr, "primitive_delete(%s) : Can not finde primitive\r\n", name);
		return -1;
	}
	if (p->flag == 0)
	{
		p->flag = -1;
		p->use_count--;
	}
	return 0;
}

int		primitive_cleanup(void)
{
	primitive_t	*p;
	
	if (primitive_initialized == 0) return -1;
	location_delete_set(locations);
	p = objects_get_first(primitives);
	while (p != NULL)
	{
		if (p->type == PRIMITIVE_TYPE_DLL)
		{
			if( (*(p->entries[PRIMITIVE_ENTRY_DELETE]))(NULL) != 0)
				fprintf(stderr, "primitive_cleanup() : Primitive %s delete function returned error\r\n", p->name);
			dlclose(p->handle);
			if (p->from_temp != 0)
				io_delete_from_temp(p->name, IO_ITEM_TYPE_PRIMITIVE);
		}
		p = objects_get_next(primitives, p);
	}
	objects_delete_set(self_objects);
	objects_delete_set(primitives);
	primitive_initialized = 0;
	return 0;
}

/****************************** Local functions *******************************/

static primitive_t	*_primitive_find(char *name)
{
	primitive_t	*p;
	
	p = (primitive_t *) objects_get_first(primitives);
	while (p != NULL)
	{
		if (strcmp(p->name, name) == 0) return p;
		p = objects_get_next(primitives, p);
	}
	return NULL;
}

