/*******************************************************************************
 * 							Common composite
 * *****************************************************************************
 * 	Filename:		composite.h
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

#ifndef	COMPOSITE_H
#define	COMPOSITE_H

//#define COMPOSITE_ENTRY_FIRE    0
//#define COMPOSITE_ENTRY_INIT    1
//#define COMPOSITE_ENTRY_CLEANUP 2

//#define COMPOSITE_META_ENTRY_LOAD    0
//#define COMPOSITE_META_ENTRY_MAKE    1
//#define COMPOSITE_META_ENTRY_DELETE  2

int     composite_init();
void	composite_cleanup(void);

void	*composite_read(char *, int platform);
void	*composite_read_hex(char *, int platform);
void	*composite_read_bin(char *, int platform);

void	*composite_load(char *name);
int		composite_init_instance(void *composite, char *name, void **data);
int		composite_cleanup_instance(void *data);
int		composite_delete(char *name);

int32_t	*composite_get_entry(void *composite, int entry);
void 	*composite_get_entry_table(char *name);
void	*composite_find(char *name);
int		composite_add_location(int chan_type, char *path);

int		composite_find_buffer(char **path, int path_index, int last_path_index, void *data, void **buffer, int *buffer_length);

#endif
