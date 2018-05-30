/*******************************************************************************
 * 							Common io
 * *****************************************************************************
 * 	Filename:		io.c
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
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<errno.h>
#include	"objects.h"
#include	"io.h"

#define	IO_BUFFER_SIZE	512
//#define	IO_ALLOC_SIZE	16

static char	*item_extensions[IO_ITEM_TYPE_COUNT] =
{
	[IO_ITEM_TYPE_BIN_OBJ_CODE] = ".sdf.bin",
	[IO_ITEM_TYPE_HEX_OBJ_CODE] = ".sdf.hex",
	[IO_ITEM_TYPE_C_OBJ_CODE] = ".sdf.c",
	[IO_ITEM_TYPE_PRIMITIVE] = ".sdf.so",
	[IO_ITEM_TYPE_ASM_CODE] = ".sdf.asm",
	[IO_ITEM_TYPE_SRC_CODE] = ".sdf.src",
	[IO_ITEM_TYPE_CONTEXT] = ".sdf.ctx",
	[IO_ITEM_TYPE_C_HEADER] = ".sdf.h",
	[IO_ITEM_TYPE_LST] = ".sdf.lst",
	[IO_ITEM_TYPE_PRN] = ".sdf.prn",
	[IO_ITEM_TYPE_SRC_LST] = ".sdf.src.lst"
};

struct _io_channel
{
	int			type;
	char		path[IO_PATH_LENGTH + 1];
	char		name[IO_BASE_NAME_LENGTH + 1];
	int			item_type;
	int			state;
	char		**memory;
	int			descriptor;
	int			ri;
	int			wi;
	char		buf[IO_BUFFER_SIZE];
};

typedef struct _io_channel	io_channel_t;

static int	io_initialized = 0;
static unsigned int	io_channel_type_index = (unsigned int) OBJECTS_TYPE_IO_CHANNEL;
static void	*io_channel_set = NULL;

/*******************************************************************************
 * 							Public functions
 * ****************************************************************************/
 
 //=============================================================================
 //							Object management functions
 // These functions are used to create and delete channel objects.
 // The functions are:
 //		io_init() : Initializes the io program unit. It should be called before
 //					any other function of this unit.
 //		io_new_channel(): Creates a new io channel object. It should be used for
 //					creating channel object of any type except the memory type.
 //		io_create_memory_channel(): Creates a memory type channel object.
 //		io_delete_channel(): Deletes a channel object. If it is necessary it
 //					closes the open channel before deleting the channel object.
 //		io_cleanup(): Cleans up the io program unit. It closes all the channels
 //					and delete the channel objects.
 //=============================================================================

int	io_init()
{
	if(io_initialized != 0)
	{
		fprintf(stderr, "error: io_init(): Already initialized\r\n");
		return -1;
	}
//	if ((io_channel_type_index = objects_register_type(sizeof(io_channel_t), IO_ALLOC_SIZE)) < 0)
//	{
//		fprintf(stderr, "error: io_init(): Can not register object type\r\n");
//		return -1;
//	}
	io_channel_type_index = (unsigned int) OBJECTS_TYPE_IO_CHANNEL;
	if ((io_channel_set = objects_new_set(io_channel_type_index)) == NULL)
	{
		objects_delete_type(io_channel_type_index);
		io_channel_type_index = -1;
		fprintf(stderr, "error: io_init(): Can not create object set\r\n");
		return -1;		
	}
	io_initialized = -1;
	return 0;
}

void	*io_new_channel(int type, char *path, char *name, int item_type)
{
	io_channel_t	*ch;
	
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_new_channel(): NOT initialized\r\n");
		return NULL;
	}
	if (path == NULL)
	{
		fprintf(stderr, "error: io_new_channel(): Path is NULL\r\n");
		return NULL;
	}
	if (name == NULL)
	{
		fprintf(stderr, "error: io_new_channel(): Name is NULL\r\n");
		return NULL;
	}
	if ((type < 1) || (type >= IO_CHAN_TYPE_COUNT))
	{
		fprintf(stderr, "error: io_new_channel(): Invalid channel type\r\n");
		return NULL;
	}
	if ((item_type < 0) || (item_type >= IO_ITEM_TYPE_COUNT))
	{
		fprintf(stderr, "error: io_new_channel(): Invalid channel item type\r\n");
		return NULL;
	}
	if (strlen(path) > IO_PATH_LENGTH)
	{
		fprintf(stderr, "error: io_new_channel(): Too long path\r\n");
		return NULL;
	}
	if (strlen(name) > IO_BASE_NAME_LENGTH)
	{
		fprintf(stderr, "error: io_new_channel(): Too long name\r\n");
		return NULL;
	}
	if ((ch = (io_channel_t *)objects_new(io_channel_set, sizeof(io_channel_t))) == NULL)
	{
		fprintf(stderr, "error: io_new_channel(): Can not create channel object\r\n");
		return NULL;
	}
	strcpy(ch->path, path);
	strcpy(ch->name, name);
	ch->type = type;
	ch->item_type = item_type;
	ch->memory = NULL;
	ch->state = IO_CHAN_CLOSED;
	ch->ri = ch->wi = 0;
	ch->descriptor = 0;
	ch->buf[0] = 0;
	return ch;
}

void	*io_new_memory_channel(char **memory, char *name, int item_type)
{
	io_channel_t	*ch;
	
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_new_memory_channel(): NOT initialized\r\n");
		return NULL;
	}
	if (memory == NULL)
	{
		fprintf(stderr, "error: io_new_memory_channel(): Memory pointer is NULL\r\n");
		return NULL;
	}
	if (name == NULL)
	{
		fprintf(stderr, "error: io_new_memory_channel(): Name is NULL\r\n");
		return NULL;
	}
	if ((item_type < 0) || (item_type >= IO_ITEM_TYPE_COUNT))
	{
		fprintf(stderr, "error: io_new_memory_channel(): Invalid channel item type\r\n");
		return NULL;
	}
	if (strlen(name) > IO_BASE_NAME_LENGTH)
	{
		fprintf(stderr, "error: io_new_memory_channel(): Too long name\r\n");
		return NULL;
	}
	if ((ch = (io_channel_t *)objects_new(io_channel_set, sizeof(io_channel_t))) == NULL)
	{
		fprintf(stderr, "error: io_new_memory_channel(): Can not create channel object\r\n");
		return NULL;
	}
	ch->memory = memory;
	ch->state = IO_CHAN_CLOSED;
	ch->type = IO_CHAN_TYPE_MEMORY;
	strcpy(ch->name, name);
	strcpy(ch->path, "");
	ch->ri = ch->wi = 0;
	ch->descriptor = 0;
	ch->buf[0] = 0;
	return ch;
}

void	io_delete_channel(void *chan)
{
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_delete_channel(): NOT initialized\r\n");
		return;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_delete_channel(): Channel is NULL\r\n");
		return;
	}
	io_close(chan);
	objects_delete(io_channel_set, chan);
}

int	io_cleanup(void)
{
	io_channel_t	*ch;
	
	if (io_initialized == 0)
	{
		fprintf(stderr, "error: io_cleanup(): NOT initialized\r\n");
		return -1;
	}
	ch = (io_channel_t *) objects_get_first(io_channel_set);
	while(ch != NULL)
	{
		io_close((void *) ch);
		ch = (io_channel_t *) objects_get_next(io_channel_set, ch);
	}
	if (objects_delete_type(io_channel_type_index) != 0)
	{
		fprintf(stderr, "error: io_cleanup(): Can not delete object type\r\n");
		return -1;
	}
//	io_channel_type_index = -1;
	io_channel_set = NULL;
	io_initialized = 0;
	return 0;
}

//==============================================================================
//					Channel independent functions
//==============================================================================

int		io_verify(int chan_type, char *path, char *name, int item_type)
{
	char	full_name[IO_FULL_NAME_LENGTH + 1];
	int		fd, ret = 0;
	
//	fprintf(stderr, "error: io_verify(): Verifying name=%s, type=%d, item type=%d, path=%s\r\n",name, chan_type, item_type, path);
	if (chan_type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: io_verify(): Channel is not FILE type\r\n");
		return -1;
	}
	//it should be implemented for other channel types
	if (io_get_full_name(full_name, path, name, item_type, IO_FULL_NAME_LENGTH + 1) != 0)
	{
		fprintf(stderr, "error: io_verify(): Can not compose item's full name\r\n");
		return -1;
	}
//	fprintf(stderr, "error: io_verify(): full name=%s\r\n", full_name);
	if ((fd = open(full_name,O_RDONLY)) == -1)
	{
		if (errno == ENOENT)
		{
//			fprintf(stderr, "error: io_verify(): errno == ENOENT\r\n");
			ret = 1;
		}
		else
		{
			fprintf(stderr, "error: io_verify(): Can not open the item\r\n");
			fprintf(stderr, "\t[item: %s]\r\n", full_name);
			ret = -1;
		}
	}
	close(fd);
	return ret;
}

int		io_get_full_name(char *full_name, char *path, char *name, int item_type, int len)
{
	int	lp, ln;
	
	if (full_name == NULL)
	{
		fprintf(stderr, "error: io_get_full_name(): Full name is NULL\r\n");
		return -1;
	}
	if (path == NULL)
	{
		fprintf(stderr, "error: io_get_full_name(): Path is NULL\r\n");
		return -1;
	}
	if (name == NULL)
	{
		fprintf(stderr, "error: io_get_full_name(): Name is NULL\r\n");
		return -1;
	}
	if ((item_type < 0) || (item_type >= IO_ITEM_TYPE_COUNT))
	{
		fprintf(stderr, "error: io_get_full_name(): Invalid channel item type\r\n");
		return -1;
	}
	if ((lp = strlen(path)) > IO_PATH_LENGTH)
	{
		fprintf(stderr, "error: io_get_full_name(): Too long path\r\n");
		return -1;
	}
	if ((ln = strlen(name)) > IO_BASE_NAME_LENGTH)
	{
		fprintf(stderr, "io_get_full_name(): Too long name\r\n");
		return -1;
	}
	if (len <= IO_FULL_NAME_LENGTH)
	{
		fprintf(stderr, "io_get_full_name(): Too short buffer for the full name\r\n");
		return -1;
	}
	strcpy(full_name, path);
	strcpy(&full_name[lp], name);
	strcpy(&full_name[lp + ln], item_extensions[item_type]);
	return 0;
}

char	*io_get_temp_path()
{
	return NULL;
}

int		io_copy_to_temp(int type, char *path, char *name, int item_type)
{
	return -1;
}

int		io_delete_from_temp(char *name, int item_type)
{
	return -1;
}

//==============================================================================
//					Channel management functions
//==============================================================================

int		io_get_name(void *chan, char *name, int len)
{
	io_channel_t	*ch = (io_channel_t *) chan;
	int	l;
	
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_get_name(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_get_name(): Channel is NULL\r\n");
		return -1;
	}
	if (name == NULL)
	{
		fprintf(stderr, "error: io_get_name(): Buffer is NULL\r\n");
		return -1;		
	}
	l = strlen(ch->name);
	if (len < l+strlen(item_extensions[ch->item_type])+1)
	{
		fprintf(stderr, "error: io_get_name(): Buffer length is too short\r\n");
		return -1;
	}
	strcpy(name, ch->name);
	strcpy(name + l, item_extensions[ch->item_type]);
	return 0;
}

int	io_open_for_read(void *chan)
{
	io_channel_t	*ch = (io_channel_t *) chan;
	int				oflag;
	char			full_name[IO_FULL_NAME_LENGTH + 1];
	
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_open_for_read(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_open_for_read(): Channel is NULL\r\n");
		return -1;
	}
	if (ch->state != IO_CHAN_CLOSED)
	{
		fprintf(stderr, "error: io_open_for_read(): Channel is not closed\r\n");
		return -1;
	}
	if (ch->type == IO_CHAN_TYPE_MEMORY)
	{
		ch->ri = 0;
		ch->state = IO_CHAN_OPEN_RD;
		return 0;
	}
	if (ch->type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: io_open_for_read(): Channel is not FILE type\r\n");
		return -1;		
	}
	if (io_get_full_name(full_name, ch->path, ch->name, ch->item_type, IO_FULL_NAME_LENGTH + 1) != 0)
	{
		fprintf(stderr, "error: io_open_for_read(): Can not get item full name\r\n");
		return -1;
	}
	oflag = O_RDONLY;
//	fprintf(stderr,"io_open_for_read(): full_name=%s\r\n", full_name);
	if ((ch->descriptor = open(full_name,oflag)) == -1)
	{
		fprintf(stderr, "error: io_open_for_read(): Can not open the file\r\n");
		return -1;		
	}
	ch->ri = 0;
	ch->wi = 0;
	ch->state = IO_CHAN_OPEN_RD;
//	fprintf(stderr,"io_open_for_read(): channel_state=%d",ch->state);
	return 0;
}

int io_open_stdout(void *chan)
{
	io_channel_t	*ch = (io_channel_t *) chan;

	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_open_stdout(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_open_stdout(): Channel is NULL\r\n");
		return -1;
	}
	if (ch->type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: io_open_stdout(): Channel is not FILE type\r\n");
		return -1;
	}
	ch->descriptor = fileno(stderr);
	ch->state = IO_CHAN_OPEN_WR;
	ch->ri = 0;
	ch->wi = 0;
	return 0;
}

int	io_open_for_write(void *chan)
{
	io_channel_t	*ch = (io_channel_t *) chan;
	int				oflag;
	mode_t			mode;
	char			full_name[IO_FULL_NAME_LENGTH + 1];
	
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_open_for_write(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_open_for_write(): Channel is NULL\r\n");
		return -1;
	}
	if (ch->state != IO_CHAN_CLOSED)
	{
		fprintf(stderr, "error: io_open_for_write(): Channel is not closed\r\n");
		return -1;
	}
	if (ch->type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: io_open_for_write(): Channel is not FILE type\r\n");
		return -1;		
	}
	if (io_get_full_name(full_name, ch->path, ch->name, ch->item_type, IO_FULL_NAME_LENGTH + 1) != 0)
	{
		fprintf(stderr, "error: io_open_for_write(): Can not get item full name\r\n");
		return -1;
	}
	oflag = O_CREAT | O_WRONLY | O_TRUNC;
	mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	if ((ch->descriptor = open(full_name,oflag, mode)) == -1)
	{
		fprintf(stderr, "error: io_open_for_write(): Can not open the file\r\n");
		return -1;		
	}
	ch->ri = 0;
	ch->wi = 0;
	ch->state = IO_CHAN_OPEN_WR;
	return 0;
}

int	io_state(void *chan)
{
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_state(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_state(): Channel is NULL\r\n");
		return -1;
	}
	return ((io_channel_t *) chan)->state;
}

int	io_get(void *chan, char *buf, int len)
{
	io_channel_t	*ch = (io_channel_t *) chan;
	int oi,l;
	
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_get(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_get(): Channel is NULL\r\n");
		return -1;
	}
	if (buf == NULL)
	{
		fprintf(stderr, "error: io_get(): Buffer is NULL\r\n");
		return -1;
	}
	if (ch->state != IO_CHAN_OPEN_RD)
	{	
		fprintf(stderr, "error: io_get(): Channel is not open for read\r\n");
		return -1;
	}
	if (ch->state == IO_CHAN_EOF) return 0;
	if (ch->type == IO_CHAN_TYPE_MEMORY)
	{
		if (ch->memory[ch->ri] == NULL) return 0;
		if ((l = strlen(ch->memory[ch->ri])) >= len)
		{	
			fprintf(stderr, "error: io_get(): Buffer length is too short\r\n");
			return -1;
		}
		strcpy(buf, ch->memory[ch->ri]);
		ch->ri++;
		return (l + 1);
	}
	if (ch->type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: io_get(): Channel is not FILE type\r\n");
		return -1;		
	}
	oi = 0;
	buf[len - 1] = buf[0] = 0;
	while (1)
	{
		if (ch->ri == ch->wi)
		{
			ch->wi = read(ch->descriptor, ch->buf, IO_BUFFER_SIZE);
			if (ch->wi == -1)
			{
				fprintf(stderr, "error: io_get(): Can not read the file\r\n");
				io_close(chan);
				return -1;
			}
			if (ch->wi == 0)
			{
				ch->state = IO_CHAN_EOF;
				buf[oi] = 0;
				return oi;
			}
			ch->ri = 0;
		}
		while ((ch->ri < ch->wi) && (oi < len))
		{
			if ((buf[oi++] = ch->buf[ch->ri++]) == 10) break;
		}
		if (buf[oi - 1] == 10)
		{
			buf[oi - 1] = 0;
			return oi;
		}
		if (oi == len)
		{
			buf[oi - 1] = 0; // we may overwrite the last character here
			fprintf(stderr, "error: io_get(): Buffer length is too short\r\n");
			return -1;
		}
	}
	
}

int	io_read(void *chan, char *buf, int len)
{
	io_channel_t	*ch = (io_channel_t *) chan;
	char			*ptr;
	int				/*size,*/ l, n;

//	fprintf(stderr,"io_read(): started\r\n");
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_read(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_read(): Channel is NULL\r\n");
		return -1;
	}
	if (buf == NULL)
	{
		fprintf(stderr, "error: io_read(): Buffer is NULL\r\n");
		return -1;
	}
	if (ch->state != IO_CHAN_OPEN_RD)
	{	
		fprintf(stderr, "error: io_read(): Channel is not open for read\r\n");
		return -1;
	}
	if (ch->type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: io_read(): Channel is not FILE type\r\n");
		return -1;		
	}
	if (ch->state == IO_CHAN_EOF) return 0;
	l = len;
	ptr = buf;
	while(l)
	{
//		size = (l > IO_BUFFER_SIZE) ? IO_BUFFER_SIZE : l;
		n = read(ch->descriptor, ptr, l/*size*/);
//		fprintf(stderr,"io_read(): size=%d n=%d\r\n",l,n);
		if (n == -1)
		{
			fprintf(stderr, "error: io_read(): Can not read the file\r\n");
			io_close(chan);
			return -1;
		}
		l -= n;
		ptr += n;
		if (n == 0)
			return (len - l);
	}
	return len;
}


int	io_put(void *chan, const char *buf)
{
	io_channel_t	*ch = (io_channel_t *) chan;
	int	len;
	int	n;
	
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_put(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_put(): Channel is NULL\r\n");
		return -1;
	}
	if (buf == NULL)
	{
		fprintf(stderr, "error: io_put(): Buffer is NULL\r\n");
		return -1;
	}
	if (ch->type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: io_put(): Channel is not FILE type\r\n");
		return -1;		
	}
	if (ch->state != IO_CHAN_OPEN_WR)
	{	
		fprintf(stderr, "error: io_put(): Channel is not open for write\r\n");
		return -1;
	}
	len = strlen(buf);
	if (len >= IO_BUFFER_SIZE)
	{	
		fprintf(stderr, "error: io_put(): The length of the line is too long\r\n");
		return -1;
	}
	memcpy(ch->buf, buf, len);
	ch->buf[len] = '\n';
	n = write(ch->descriptor, ch->buf, len+1);
	if ((n == -1) || (n != len + 1))
	{
		fprintf(stderr, "error: io_put(): Can not write to the file\r\n");
		io_close(chan);
		return -1;
	}
	return 0;
}

int	io_write(void *chan, char *buf, int len)
{
	io_channel_t	*ch = (io_channel_t *) chan;
	char	*ptr = buf;
	int		size;
	int		n;
	
	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_write(): NOT initialized\r\n");
		return -1;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_write(): Channel is NULL\r\n");
		return -1;
	}
	if (buf == NULL)
	{
		fprintf(stderr, "error: io_write(): Buffer is NULL\r\n");
		return -1;
	}
	if (ch->type != IO_CHAN_TYPE_FILE)
	{
		fprintf(stderr, "error: iio_write(): Channel is not FILE type\r\n");
		return -1;		
	}
	if (ch->state != IO_CHAN_OPEN_WR)
	{	
		fprintf(stderr, "error: io_write(): Channel is not open for write\r\n");
		return -1;
	}
	while (len > 0)
	{
		size = (len > IO_BUFFER_SIZE) ? IO_BUFFER_SIZE : len;
		n = write(ch->descriptor, ptr, size);
		if ((n == -1) || (n != size))
		{
			fprintf(stderr, "error: io_put(): Can not write to the file\r\n");
			io_close(chan);
			return -1;
		}
		ptr += size;
		len -= size;
	}
	return 0;
}

void	io_close(void *chan)
{
	io_channel_t	*ch = (io_channel_t *) chan;

	if(io_initialized == 0)
	{
		fprintf(stderr, "error: io_close(): NOT initialized\r\n");
		return;
	}
	if (chan == NULL)
	{
		fprintf(stderr, "error: io_close(): Channel is NULL\r\n");
		return;
	}
	if (ch->state != IO_CHAN_CLOSED)
	{
		if (ch->type != IO_CHAN_TYPE_MEMORY)
		{
			if (ch->descriptor != fileno(stdout))
				close(ch->descriptor);
		}
		ch->state = IO_CHAN_CLOSED;
	}
}


