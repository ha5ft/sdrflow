/*******************************************************************************
 * 							Common primitive
 * *****************************************************************************
 * 	Filename:		primitive.h
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

#ifndef	PRIMITIVE_H
#define	PRIMITIVE_H

//#include    "instructions.h"
//#include    "obj_format.h"
#include	"../../include/primitive_interface.h"

// typedef	int (*primreg_func_t)(char *name, entry_t init, entry_t fire, entry_t cleanup);
// typedef	void (*reg_func_t)(primreg_func_t prim_reg_func);

int					primitive_init(void);
int					primitive_add_location(int type, char *path);
int					primitive_new(char *name, primitive_catalog_t *catalog);
int					primitive_load(char *name);
primitive_entry_t	*primitive_get_entry_table(char *name);
int					primitive_create_self(char *name, char *instance_name, void **self);
int 				primitive_delete_self(void *self);
void				primitive_remove_not_used();
void				primitive_remove_being_loaded();
void				primitive_clear_flag();
int					primitive_delete(char *name);
int					primitive_cleanup(void);

#endif
