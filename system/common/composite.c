/*******************************************************************************
 * 							Common composite
 * *****************************************************************************
 * 	Filename:		composite.c
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
#include	"obj_format.h"
#include	"../../include/primitive_interface.h"
#include	"io.h"
#include	"location.h"
#include	"primitive.h"
#include	"composite.h"
#include	"pointer.h"
#include	"instructions.h"


#define HEX_HEADER_LENGTH			9
#define	HEX_DATA_COUNT				32
#define	HEX_DATA_LENGTH				(2 * HEX_DATA_COUNT)
#define HEX_CHECKSUM_LENGTH			2
#define HEX_REC_LENGTH				(HEX_HEADER_LENGTH + HEX_DATA_LENGTH + HEX_CHECKSUM_LENGTH)
#define HEX_EOF_REC_LENGTH			(HEX_HEADER_LENGTH + HEX_CHECKSUM_LENGTH)
#define HEX_BUF_LENGTH				(HEX_REC_LENGTH + 1)
#define HEX_DATA_START_POS			9
#define HEX_CHECKSUM_POS			(HEX_REC_LENGTH - HEX_CHECKSUM_LENGTH)
#define HEX_MIN_REC_LENGTH			HEX_EOF_REC_LENGTH

#define	C_BUF_LENGTH				64

struct _composite
{
	void	*object_code;
	char	*name;
	void	*data;
	int		flag;
	int		use_count;
	int		context_size;
	int32_t	*meta_entries[META_ENTRY_COUNT];
	int32_t	*entries[ENTRY_COUNT];
};

typedef struct _composite composite_t;

static void		*composite_locations = NULL;
static void		*composites = NULL;
static void		*object_codes = NULL;
static void		*data_segments;
//static void		*pointer_sets = NULL;

static int		composite_initialized = 0;
static int		loading_in_progress = 0;

static int			_composite_read_hex_rec(unsigned char *buf, int offs, int count, int rtype, void *inp);
static void			*_composite_read(char *name, int platform);
static void			*_composite_read_bin(char *name, int platform);
static void			*_composite_read_hex(char *name, int platform);
static void			*__composite_read_bin(void *inp, int platform);
static void			*__composite_read_hex(void *inp, int platform);

static int			_composite_load(char *name);
static int 			_composite_init_instance(composite_t *composite, char *name, void **data_seg);
static int 			_composite_cleanup_instance(void *data, int recursive);
static int			_composite_delete(composite_t *composite);

static void			_composite_remove_not_used();
static void			_composite_remove_being_loaded();
static void			_composite_clear_flag();

// -----------------------------------------------------------------------------
//							Public functions
// -----------------------------------------------------------------------------

int	composite_init(void)
{
	if (composite_initialized != 0) return -1;
	if ((composite_locations = location_new_set()) == NULL) return -1;
	if ((composites = objects_new_set(OBJECTS_TYPE_COMPOSITE)) == NULL)
	{
		location_delete_set(composite_locations);
		return -1;
	}
	if ((object_codes = objects_new_set(OBJECTS_TYPE_OBJ_CODE)) == NULL)
	{
		location_delete_set(composite_locations);
		location_delete_set(composites);
		return -1;
	}
	if ((data_segments = objects_new_set(OBJECTS_TYPE_DATA_SEG)) == NULL)
	{
		location_delete_set(composite_locations);
		objects_delete_set(composites);
		objects_delete_set(object_codes);
		return -1;
	}
	if (location_add(composite_locations, IO_CHAN_TYPE_FILE, "./actor/") == -1)
	{
		location_delete_set(composite_locations);
		objects_delete_set(composites);
		objects_delete_set(object_codes);
		objects_delete_set(data_segments);
		return -1;
	}
	composite_initialized = -1;
	return 0;
}

void	composite_cleanup(void)
{
	void	*d;
	
//	fprintf(stderr,"composite_cleanup()\r\n");
	location_delete_set(composite_locations);
//	fprintf(stderr,"composite_cleanup(): locations deleted\r\n");
	d = objects_get_first(data_segments);
	while (d != NULL)
	{
		_composite_cleanup_instance(d, 0);
	}
//	fprintf(stderr,"composite_cleanup(): delete scrits have run\r\n");
	objects_delete_set(composites);
//	fprintf(stderr,"composite_cleanup(): composites deleted\r\n");
	objects_delete_set(object_codes);
//	fprintf(stderr,"composite_cleanup(): object codes deleted\r\n");
	objects_delete_set(data_segments);
//	fprintf(stderr,"composite_cleanup(): data segments deleted\r\n");
	composite_locations = NULL;
	composites = NULL;
	object_codes = NULL;
	data_segments = NULL;
	composite_initialized = 0;
	loading_in_progress = 0;
	return;
}


void	*composite_load(char *name)
{
	void		*top = NULL;
	int			err = 0;
	
	if (_composite_load(name) != 0)
	{
		fprintf(stderr, "composite_load(%s) : Can not load composite\r\n", name);
		err = -1;
	}
	else
	{
		if ((top = composite_find(name)) == NULL)
		{
			fprintf(stderr, "composite_load(%s) : Can not find loaded composite\r\n", name);
			err = -1;
		}
	}
	if ((err == 0) && (((composite_t *) top)->context_size != 0))
	{
		err = -1;
		fprintf(stderr, "composite_load(%s) : Top level context size is not 0\r\n", name);
	}
	if (err != 0)
	{
		_composite_remove_being_loaded();
		primitive_remove_being_loaded();
	}
	_composite_clear_flag();
	primitive_clear_flag();
	return top;
}

int	composite_init_instance(void *composite, char *name, void **data)
{
	composite_t		*comp = (composite_t *) composite;
	int				ret = 0;

	*data = NULL;
	if (_composite_init_instance(comp, name, data) != 0)
	{
		fprintf(stderr,"composite_init_instance(%s): error in initializing data segment\r\n", name);		
		if (composite_cleanup_instance(*data) != 0)
			fprintf(stderr,"composite_init_instance(%s): error in cleanup data segment\r\n", name);	
		ret = -1;	
	}
	return ret;
}

int	composite_cleanup_instance(void *data)
{
	int	ret = 0;

	if (data != NULL)
	{
		if (objects_verify_object(data_segments, data) == 0)
		{
			_composite_cleanup_instance(data, -1);
		}
		else
		{
			ret = -1;
			fprintf(stderr, "composite_cleaup_instanve() : Invalid data segment object\r\n");
		}
	}
	return ret;
}

int	composite_delete(char *name)
{
	composite_t	*composite;
	int			ret = 0;

	if ((composite = (composite_t *) composite_find(name)) == NULL)
	{
		fprintf(stderr,"composite_delete(%s) : Can not find composit\r\n", name);
		ret = -1;
	}
	else
	{
		if (_composite_delete(composite) != 0)
		{
			ret = -1;
			fprintf(stderr, "composite_delete(%s) : Error in deleting composits\r\n", name);
		}
		_composite_clear_flag();
		_composite_remove_not_used();
		primitive_clear_flag();
		primitive_remove_not_used();
	}
	return ret;
}

// ============================================================================

void *composite_get_entry_table(char *name)
{
	composite_t		*comp = NULL;

	if ((comp = (composite_t *) composite_find(name)) == NULL)
	{
		fprintf(stderr,"composite_get_entry_table(%s): could not find composite object\r\n", name);
		return NULL;
	}

	return (void *) comp->entries;
}

int32_t	*composite_get_entry(void *composite, int entry)
{
	composite_t	*c = (composite_t *) composite;
	
	if ((entry >= 0) && (entry < 3) && (composites != NULL))
	{
		return c->entries[entry];
	}
	else
	{
		return NULL;
	}
}

void *composite_find(char *name)
{
	composite_t	*c = NULL;
	
	c = (composite_t *)objects_get_first(composites);
	while (c != NULL)
	{
		if (strcmp(c->name , name) == 0)
			break;
		c = (composite_t *) objects_get_next(composites, c);
	}
	return (void *) c;
}

int	composite_find_buffer(char **path, int path_index, int last_path_index, void *data, void **buffer, int *buffer_length)
{
	meta_mkbuf_t	*mkbuf;
	buf_instance_t	*bufinst;
	meta_mkcomp_t	*mkcomp;
	comp_instance_t	*compinst;
	data_header_t	*dheader = (data_header_t *) data;
	void			*meta = (void *) dheader->catalog;
	meta_header_t	*mheader = (meta_header_t *) (meta + sizeof(catalog_t));
	int32_t			*ip = (int32_t *) (meta + mheader->make_offset);
	char			*name;

//	fprintf(stderr,"composite_find_buffer()\r\n");
	while (*ip != META_EXIT_INSTR)
	{
		switch (*ip)
		{
			case	META_MKBUF_INSTR:
				if (path_index == last_path_index)
				{
					mkbuf = (meta_mkbuf_t *) ip;
					bufinst = (buf_instance_t *)(data + mkbuf->instance);
					name = (char *) (meta + mkbuf->name);
//					fprintf(stderr,"name:%s\r\n",name);
					if (strcmp(name, path[path_index]) == 0)
					{
						*buffer = bufinst->buf;
						*buffer_length = mkbuf->size;
						return 0;
					}
				}
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkbuf_t));
				break;
			case	META_MKCOMPINST_INSTR:
				if (path_index != last_path_index)
				{
					mkcomp = (meta_mkcomp_t *) ip;
					compinst = (comp_instance_t *)(data + mkcomp->instance);
					name = (char *) (meta + mkcomp->inst_name);
//					fprintf(stderr,"cname:%s\r\n",name);
					if (strcmp(name, path[path_index]) == 0)
					{
						if (composite_find_buffer(path, path_index + 1, last_path_index, compinst->data, buffer, buffer_length) == 0)
						{
							return 0;
						}
						else
						{
							return -1;
						}
					}
				}
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkcomp_t));
				break;
			case	META_MKPRIMINST_INSTR:
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkprim_t));
				break;
			default:
				return -1;
				break;
		}
	}

	return -1;
}

int	composite_add_location(int chan_type, char *path)
{
	return location_add(composite_locations, chan_type, path);
}

// -----------------------------------------------------------------------------
//							Private functions
// -----------------------------------------------------------------------------

static void	_composite_remove_not_used()
{
	composite_t	*c, *n;

	c = (composite_t *) objects_get_first(composites);
	while (c != NULL)
	{
		n = (composite_t *) objects_get_next(composites, (void *) c);
		if (c->use_count <= 0)
		{
			objects_delete(object_codes, c->object_code);
			objects_delete(composites, (void *) c);
		}
		c = n;
	}
}

static void	_composite_remove_being_loaded()
{
	composite_t	*c, *n;
	
	c = (composite_t *) objects_get_first(composites);
	while (c != NULL)
	{
		n = (composite_t *) objects_get_next(composites, (void *) c);
		if ((c->flag != 0) && (c->use_count == 1))
		{
			objects_delete(object_codes, c->object_code);
			objects_delete(composites, (void *) c);
		}
		c = n;
	}
}

static void	_composite_clear_flag()
{
	composite_t	*c;
	
	c = (composite_t *) objects_get_first(composites);
	while (c != NULL)
	{
		if (c->flag != 0)
		{
			c->flag = 0;
		}
		c = (composite_t *) objects_get_next(composites, (void *) c);			
	}
}


static int	_composite_load(char *name)
{
	int32_t			*ip;
	void			*obj;
	composite_t		*c;
	catalog_t		*cat;
	meta_header_t	*mheader;
	code_header_t	*cheader;
//	int				ret;
	
	if (name == NULL) return -1;
	if ((c = (composite_t *) composite_find(name)) != NULL)
	{
		if (c->flag == 0)
		{
			c->flag = -1;
			c->use_count++;
		}
		return 0;
	}
	if ((c = objects_new(composites, sizeof(composite_t))) == NULL)
	{
		return -1;
	}
	if ((obj = _composite_read(name, 0)) == NULL)
	{
		objects_delete(composites, c);
		return -1;
	}
	
	cat = (catalog_t *) obj;
	mheader = (meta_header_t *) (obj + sizeof(catalog_t));
	cheader = (code_header_t *) (obj + cat->code_offset);
	c->name = (char *)(obj + cat->name_offset);
	c->object_code = obj;
	c->data = obj + cat->data_offset;
	c->meta_entries[META_ENTRY_LOAD] = (int32_t *) (obj + mheader->load_offset);
	c->meta_entries[META_ENTRY_MAKE] = (int32_t *) (obj + mheader->make_offset);
	c->meta_entries[META_ENTRY_DELETE] = (int32_t *) (obj + mheader->delete_offset);
	c->entries[ENTRY_FIRE] = (int32_t *) (obj + cat->code_offset + cheader->fire_offset);
	c->entries[ENTRY_INIT] = (int32_t *) (obj + cat->code_offset + cheader->init_offset);
	c->entries[ENTRY_CLEANUP] = (int32_t *) (obj + cat->code_offset + cheader->cleanup_offset);
	c->use_count = 1;
	c->flag = -1;
	c->context_size = cat->context_size;

	ip = c->meta_entries[META_ENTRY_LOAD];
	while (*ip != META_EXIT_INSTR)
	{
		switch	(*ip)
		{
			case	META_LDCOMP_INSTR:
				if (_composite_load((char *) (obj + ((meta_ldcomp_t *) ip)->name)) != 0)
				{
					fprintf(stderr, "_composite_load(%s): Can not load composite %s\r\n", name, (char *)(obj + ((meta_ldcomp_t *) ip)->name));
					return -1;
				}
				ip = (int32_t *) (((void *) ip) + sizeof(meta_ldcomp_t));
				break;
			case	META_LDPRIM_INSTR:
				if (primitive_load((char *) (obj + ((meta_ldprim_t *) ip)->name)) != 0)
				{
					fprintf(stderr, "_composite_load(%s): Can not load primitive %s\r\n", name, (char *)(obj + ((meta_ldprim_t *) ip)->name));
					return -1;
				}
				ip = (int32_t *) (((void *) ip) + sizeof(meta_ldprim_t));
				break;
			default:
				return -1;
				break;
		}
	}
	return 0;
}

static int _composite_init_instance(composite_t *composite, char *name, void **data_seg)
{
	composite_t		*c = NULL;
	catalog_t		*cat = (catalog_t *) composite->object_code;
	void			*meta = composite->object_code;
	void			*rom_data, *data;
	int32_t			*ip;
	int				data_size;
	meta_mkbuf_t	*mkbuf;
	buf_instance_t	*bufinst;
	meta_mkcomp_t	*mkcomp;
	meta_mkprim_t	*mkprim;
	comp_instance_t	*compinst;
	prim_instance_t	*priminst;
	data_header_t	*dheader;
	
	if (composite == NULL) return -1;
	data_size = cat->size - cat->data_offset;
	if ((data = objects_new(data_segments, data_size)) == NULL)
	{
		fprintf(stderr,"_composite_init_instance(): could not create data segment object for %s\r\n", composite->name);
		return -1;
	}
	rom_data = composite->object_code + cat->data_offset;
	memcpy(data, rom_data, data_size);
	
//	Initialize data segment header
	dheader = (data_header_t *) data;
	dheader->catalog = cat;
	dheader->flag = -1;
	dheader->composite = (void *) composite;
	dheader->name = name;

	*data_seg = data;

	ip = composite->meta_entries[META_ENTRY_MAKE];
	while (*ip != META_EXIT_INSTR)
	{
		switch (*ip)
		{
			case	META_MKBUF_INSTR:
				mkbuf = (meta_mkbuf_t *) ip;
				bufinst = (buf_instance_t *)(data + mkbuf->instance);
				if (bufinst->buf == NULL)
				{
					fprintf(stderr, "_composite_init_data(): buffer %s offset is 0\r\n", (char *) (meta + mkbuf->name));
					return -1;
				}
				bufinst->buf = data + (long) bufinst->buf;
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkbuf_t));
				break;
			case	META_MKCOMPINST_INSTR:
				mkcomp = (meta_mkcomp_t *) ip;
				compinst = (comp_instance_t *)(data + mkcomp->instance);
				c = composite_find((char *)(meta + mkcomp->comp_name));
				if (c == NULL)
				{
					fprintf(stderr, "_composite_init_data(): Could not find the composite object %s\r\n",(char *)(meta + mkcomp->comp_name));
					return -1;
				}
				if (_composite_init_instance(c, meta + mkcomp->inst_name, &compinst->data) != 0)
				{
					fprintf(stderr, "_composite_init_data(): could not create data segment for %s\r\n", (char *)(meta + mkcomp->inst_name));
					return -1;
				}
				compinst->table = c->entries;
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkcomp_t));
				break;
			case	META_MKPRIMINST_INSTR:
				mkprim = (meta_mkprim_t *) ip;
				priminst = (prim_instance_t *)(data + mkprim->instance);
				priminst->table = (void **) primitive_get_entry_table((char *)(meta + mkprim->prim_name));
				if (priminst->table == NULL)
				{
					fprintf(stderr, "_composite_init_data(): invalid entry table for primitive %s\r\n", (char *) (meta + mkprim->prim_name));
					return -1;
				}
				if (primitive_create_self((char *)(meta + mkprim->prim_name), (char *)(meta + mkprim->inst_name), &priminst->self) != 0)
				{
					fprintf(stderr, "_composite_init_data(): can not create self for primitive %s\r\n", (char *) (meta + mkprim->prim_name));
					return -1;
				}
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkprim_t));
				break;
			default:
				fprintf(stderr, "_composite_init_data(): Invalide meta instruction in %s\r\n", composite->name);
				return -1;
				break;
		}
	}
	return 0;
}

static int _composite_cleanup_instance(void *data, int recursive)
{
	catalog_t		*cat;
	meta_header_t	*mheader;
	data_header_t	*dheader = (data_header_t *) data;
	meta_mkcomp_t	*mkcomp;
	meta_mkprim_t	*mkprim;
	comp_instance_t	*compinst;
	prim_instance_t	*priminst;
	void			*meta;
	int32_t			*ip;
	int				err = 0;

	cat = dheader->catalog;
	meta = (void *) cat;

	mheader = (meta_header_t *) (meta + sizeof(catalog_t));
	ip = (int32_t *) (meta + mheader->make_offset);
	while (*ip != META_EXIT_INSTR)
	{
		switch (*ip)
		{
			case	META_MKBUF_INSTR:
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkbuf_t));
				break;
			case META_MKCOMPINST_INSTR:
				mkcomp = (meta_mkcomp_t *) ip;
				compinst = (comp_instance_t *) (data + mkcomp->instance);
				if (recursive != 0)
				{
					if (objects_verify_object(data_segments, compinst->data) == 0)
					{
						if (_composite_cleanup_instance(compinst->data, -1) != 0)
							fprintf(stderr, "_composite_cleanup_instance() : error in cleaning up %s\r\n", (char *) (meta + mkcomp->inst_name));
					}
				}
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkcomp_t));
				break;
			case META_MKPRIMINST_INSTR:
				mkprim = (meta_mkprim_t *) ip;
				priminst = (prim_instance_t *) (data + mkprim->instance);
				if (primitive_delete_self(priminst->self) != 0)
					fprintf(stderr, "_composite_cleanup_instance() : error in deleting self for %s\r\n",(char *) (meta + mkprim->inst_name));
				ip = (int32_t *) (((void *) ip) + sizeof(meta_mkprim_t));
				break;
			default:
				fprintf(stderr, "_composite_cleanup_instance(): Invalide meta instruction in %s\r\n", (char *)(meta + cat->name_offset));
				err = -1;
				break;
		}
		if (err != 0) break;
	}
	if (objects_delete(data_segments, data) != 0)
	{
		fprintf(stderr, "_composite_cleanup_instance() : error in deleting data sehment object\r\n");
		err = -1;
	}
	return err;
}

static int	_composite_delete(composite_t *composite)
{
	int32_t			*ip;
	void			*obj;
	composite_t		*c = NULL;
	meta_ldcomp_t	*ldcomp;
	meta_ldprim_t	*ldprim;
	int				err = 0;
	
	if (composite == NULL) return -1;
	if (composite->flag != 0) return 0;
	composite->flag = -1;
	composite->use_count--;
	obj = composite->object_code;

	ip = composite->meta_entries[META_ENTRY_LOAD];
	while (*ip != META_EXIT_INSTR)
	{
		switch	(*ip)
		{
			case	META_LDCOMP_INSTR:
				ldcomp = (meta_ldcomp_t *) ip;
				if ((c = (composite_t *) composite_find((char *) (obj + ldcomp->name))) == NULL)
				{
					fprintf(stderr, "_composite_delete() : Can not finde composite ob ject %s\r\n",(char *) (obj + ldcomp->name));
				}
				else
				{
					if (_composite_delete(c) != 0)
					{
						fprintf(stderr, "_composite_delete(): Can not delete composite %s\r\n", (char *) (obj + ldcomp->name));
					}
				}
				ip = (int32_t *) (((void *) ip) + sizeof(meta_ldcomp_t));
				break;
			case	META_LDPRIM_INSTR:
				ldprim = (meta_ldprim_t *) ip;
				if (primitive_delete((char *) (obj + ldprim->name)) != 0)
				{
					fprintf(stderr, "_composite_delete(%s): Can not delete primitive %s\r\n", c->name, (char *)(obj + ldprim->name));
				}
				ip = (int32_t *) (((void *) ip) + sizeof(meta_ldprim_t));
				break;
			default:
				fprintf(stderr, "_composite_delete(%s) : illegal instruction in load section\r\n", c->name);
				err = -1;
				break;
		}
		if (err != 0) break;
	}
	return err;
}




static void *_composite_read(char *name, int platform)
{
	void	*comp;
	
//	fprintf(stderr, "_composite_read(%s): start reading the component\r\n", name);
	if ((comp = _composite_read_bin(name, platform)) == NULL)
		comp = _composite_read_hex(name, platform);
	return comp;
}

static void *_composite_read_hex(char *name, int platform)
{
	void	*channel, *obj;
	int		chan_type, item_type;
	char	*path;
	
	item_type = IO_ITEM_TYPE_HEX_OBJ_CODE;
	if (location_find(composite_locations, name, item_type, &chan_type, &path) != 0)
		return NULL;
	if ((channel = io_new_channel(chan_type, path, name, item_type)) == NULL)
		return NULL;
	if (io_open_for_read(channel) != 0)
	{
		io_delete_channel(channel);
		return NULL;
	}
	obj = __composite_read_hex(channel, platform);
	io_close(channel);
	io_delete_channel(channel);
	return obj;
}

static void *_composite_read_bin(char *name, int platform)
{
	void		*channel, *obj;
	int			chan_type, item_type;
	char		*path;
	
//	fprintf(stderr, "_composite_read_bin(%s): start reading the component\r\n", name);
	item_type = IO_ITEM_TYPE_BIN_OBJ_CODE;
	if (location_find(composite_locations, name, item_type, &chan_type, &path) != 0)
	{
//		fprintf(stderr, "_composite_read_bin(%s): could not find component file\r\n", name);
		return NULL;
	}
	if ((channel = io_new_channel(chan_type, path, name, item_type)) == NULL)
		return NULL;
	if (io_open_for_read(channel) != 0)
	{
		io_delete_channel(channel);
		return NULL;
	}
	obj = __composite_read_bin(channel, platform);
	io_close(channel);
	io_delete_channel(channel);
	return obj;
}

static int _composite_read_hex_rec(unsigned char *buf, int offs, int count, int rtype, void *inp)
{
	char			line[HEX_BUF_LENGTH];
	char			*r;
	int				sum, rec_size/*, n*/;
	unsigned int	len, org, type, b, chksum;
	
	if (buf == NULL) return -1;
	line[HEX_BUF_LENGTH-1] = 0;
	rec_size = HEX_EOF_REC_LENGTH + (2 * count);
	if (io_get(inp, line, HEX_BUF_LENGTH) == rec_size + 1)
	{
		r = line;
		if (*r != ':') return -1;
		line[HEX_BUF_LENGTH-1] = 0;
		if (sscanf(r+1, "%02x%04x%02x", &len, &org, &type) != 3) return -1;
		if (len != count) return -1;
		if (org != offs) return -1;
		if (type != rtype) return -1;
		sum = (len & 255) + ((org >> 8) & 255) + (org & 255) + (type & 255);
		r += HEX_HEADER_LENGTH;
		buf += offs;
		while (len--)
		{
			if (sscanf(r, "%02x", &b) != 1) return -1;
			*buf++ = b & 255;
			sum += b & 255;
			r += 2;
		}
		if (sscanf(r, "%02x", &chksum) != 1) return -1;
		if ((((sum & 255) + (chksum & 255)) & 255) != 0) return -1;
	}
	else
	{
		return -1;
	}
	return 0;
}

static void	*__composite_read_bin(void *inp, int platform)
{
	catalog_t	cat;
	int			size, n, cat_size;
	void		*obj;
	
//	fprintf(stderr, "__composite_read_bin(): start reading the component\r\n");
	cat_size = sizeof(catalog_t);
	n = io_read(inp, (char *)(&cat), cat_size);
	if (n != cat_size)
	{
		fprintf(stderr,"__composite_read_bin(): could not read catalog\r\n");
		fprintf(stderr,"__composite_read_bin(): cat_size=%d, read_returned=%d \r\n", cat_size,n);
		return NULL;
	}
	if ((platform != -1) && (platform != cat.platform))
	{
		fprintf(stderr,"composite_load(): wrong platform \r\n");
		return NULL;
	}
	size = cat.size;
	if ((obj = objects_new(object_codes, size)) == NULL)
	{
		fprintf(stderr,"composite_load(): could not create object code object \r\n");
		return NULL;
	}
	memcpy(obj, (void *) (&cat), cat_size);
	size -= cat_size;
	n = io_read(inp, ((char *) obj) + cat_size, size);
	if (n != size)
	{
		objects_delete(object_codes, obj);
		obj = NULL;
		fprintf(stderr,"composite_load(): error in reading the object code \r\n");
	}
	return obj;
}

static void *__composite_read_hex(void *inp, int platform)
{
	catalog_t		cat;
	int				offs, count, size;
	unsigned char	*buf;
	
	buf = NULL;
	offs = 0;
	count = HEX_DATA_COUNT;
	if (_composite_read_hex_rec((unsigned char *)(&cat), offs, count, 0, inp) == -1) return NULL;
	if ((platform != -1) && (platform != cat.platform)) return NULL;
	size = cat.size;
	if ((buf = (unsigned char *) objects_new(object_codes, size)) == NULL) return NULL;
	memcpy((void *) buf, (void *)(&cat), sizeof(catalog_t));
	offs += HEX_DATA_COUNT;
	while (offs < size)
	{
		if (offs > size - HEX_DATA_COUNT)
			count = size - offs;
		if (_composite_read_hex_rec(buf, offs, count, 0, inp) == -1)
		{
			objects_delete(object_codes, (void *) buf);
			return NULL;
		}
		offs += count;
	}
	if (_composite_read_hex_rec(buf, 0, 0, 1, inp) == -1)
	{
		objects_delete(object_codes, (void *) buf);
		return NULL;
	}
	return buf;
}

