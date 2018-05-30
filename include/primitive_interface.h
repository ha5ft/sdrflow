/*******************************************************************************
 * 							Primitive interface
 * *****************************************************************************
 * 	Filename:		primitive_interface.h
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

#ifndef	PRIMITIVE_INTERFACE_H
#define	PRIMITIVE_INTERFACE_H

#define PRIMITIVE_ENTRY_COUNT	5

#define PRIMITIVE_ENTRY_FIRE	0
#define PRIMITIVE_ENTRY_INIT	1
#define PRIMITIVE_ENTRY_CLEANUP	2
#define PRIMITIVE_ENTRY_LOAD	3
#define PRIMITIVE_ENTRY_DELETE	4

typedef int (*primitive_entry_t)(void *context);

struct _primitive_catalog
{
	char				*name;
	size_t				self_size;
	primitive_entry_t	fire;
	primitive_entry_t	init;
	primitive_entry_t	cleanup;
	primitive_entry_t	load;
	primitive_entry_t	delete;
}__attribute__((packed));

typedef struct _primitive_catalog	primitive_catalog_t;

typedef int (*open_cmd_channel_t)(char *name);
typedef int (*read_cmd_channel_t)(int channel, char *cmd, int size);
typedef int (*close_cmd_channel_t)(int channel);

struct _system_catalog
{
//	char	*program_name;
	int		version;
	open_cmd_channel_t	open_cmd;
	read_cmd_channel_t	read_cmd;
	close_cmd_channel_t	close_cmd;
}__attribute__((packed));

typedef struct _system_catalog	system_catalog_t;

struct _default_self
{
	char	*instance_name;
}__attribute__((packed));

typedef struct _default_self default_self_t;

#endif

