/*******************************************************************************
 * 							Common objects
 * *****************************************************************************
 * 	Filename:		objects.h
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

#ifndef	OBJECTS_H
#define	OBJECTS_H

#include	<stddef.h>

#define OBJECTS_TYPE_NULL		0
#define OBJECTS_TYPE_OBJECT_SET	1
#define OBJECTS_TYPE_POINTER	2
#define OBJECTS_TYPE_WORD		3
#define OBJECTS_TYPE_IO_CHANNEL	4
#define OBJECTS_TYPE_VALUE		5
#define OBJECTS_TYPE_NAME		6
#define OBJECTS_TYPE_LOCATION	7
#define OBJECTS_TYPE_SENTENCE	8
#define OBJECTS_TYPE_SCANNER	9
#define OBJECTS_TYPE_HASH_TABLE	10
#define OBJECTS_TYPE_ASM_CODE	11
#define OBJECTS_TYPE_SRC_CODE	12
#define OBJECTS_TYPE_OBJ_CODE	13
#define OBJECTS_TYPE_COMPOSITE	14
#define OBJECTS_TYPE_PRIMITIVE	15
#define OBJECTS_TYPE_PROGRAM	16
#define OBJECTS_TYPE_CONTEXT	17
#define OBJECTS_TYPE_DATA		18
#define OBJECTS_TYPE_DATA_SEG    19
#define OBJECTS_TYPE_SIGNAL		20
#define OBJECTS_TYPE_SELF		21
#define	OBJECTS_TYPE_COMMAND	22

#define OBJECTS_NR_OF_TYPES		23

int 	objects_init(void);
int     objects_is_initialized(void);
//int		objects_register_type(int obj_size, int alloc_count);
void	*objects_new_set(unsigned int type_index);
//int		objects_object_size(void *set);
void	*objects_new(void *set, size_t size);
int		objects_get_set_size(void *set);
void	*objects_get_first(void *set);
void	*objects_get_last(void *set);
void	*objects_get_next(void *set, void *current);
void	*objects_get_prev(void *set, void *current);
int 	objects_delete(void *set, void *obj);
int 	objects_delete_set(void *set);
int 	objects_delete_type(unsigned int type_index);
void	objects_cleanup(void);
int		objects_verify_set(void *set);
int		objects_verify_object(void *set, void *obj);
void	*objects_get_first_set(unsigned int type_index);
void	*objects_get_next_set(unsigned int type_index, void *set);

#endif
