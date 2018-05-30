/*******************************************************************************
 * 							Common io
 * *****************************************************************************
 * 	Filename:		io.h
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

#ifndef	IO_H
#define	IO_H

#include	"common.h"

#define	IO_CHAN_CLOSED		1
#define	IO_CHAN_OPEN_RD		2
#define	IO_CHAN_OPEN_WR		3
#define	IO_CHAN_EOF			4
#define	IO_CHAN_ERROR		-1

#define	IO_CHAN_TYPE_MEMORY	0
#define	IO_CHAN_TYPE_FILE	1
#define	IO_CHAN_TYPE_TCP	2
#define	IO_CHAN_TYPE_UDP	3

#define	IO_ITEM_TYPE_BIN_OBJ_CODE	0
#define	IO_ITEM_TYPE_HEX_OBJ_CODE	1
#define	IO_ITEM_TYPE_C_OBJ_CODE	2
#define	IO_ITEM_TYPE_PRIMITIVE	3
#define	IO_ITEM_TYPE_ASM_CODE	4
#define	IO_ITEM_TYPE_SRC_CODE	5
#define	IO_ITEM_TYPE_CONTEXT	6
#define	IO_ITEM_TYPE_C_HEADER	7
#define	IO_ITEM_TYPE_LST	8
#define	IO_ITEM_TYPE_PRN	9
#define	IO_ITEM_TYPE_SRC_LST	10

#define	IO_PATH_LENGTH			256
#define	IO_BASE_NAME_LENGTH		COMMON_NAME_LENGTH
#define	IO_EXTENSION_LENGTH		8
#define	IO_NAME_LENGTH			(IO_BASE_NAME_LENGTH + IO_EXTENSION_LENGTH)
#define	IO_FULL_NAME_LENGTH		(IO_PATH_LENGTH + IO_NAME_LENGTH)

#define	IO_CHAN_TYPE_COUNT		4
#define	IO_ITEM_TYPE_COUNT		11

int		io_init(void);
int		io_verify(int chan_type, char *path, char *name, int item_type);
int		io_get_name(void *chan, char *name, int len);
int		io_get_full_name(char *full_name, char *path, char *name, int item_type, int len);
char	*io_get_temp_path();
int		io_copy_to_temp(int type, char *path, char *name, int item_type);
int		io_delete_from_temp(char *name, int item_type);
void	*io_new_channel(int type, char *path, char *name, int item_type);
void	*io_new_memory_channel(char **memory, char *name, int item_type);
int		io_open_for_read(void *chan);
int		io_open_for_write(void *chan);
int 	io_open_stdout(void *chan);
int		io_state(void *chan);
int		io_get(void *chan, char *buf, int len);
int		io_put(void *chan, const char *buf);
int		io_read(void *chan, char *buf, int len);
int		io_write(void *chan, char *buf, int len);
void	io_close(void *chan);
void	io_delete_channel(void *chan);
int		io_cleanup(void);

#endif
