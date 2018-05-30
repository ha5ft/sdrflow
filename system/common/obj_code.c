/*******************************************************************************
 * 							Common obj_code
 * *****************************************************************************
 * 	Filename:		object_code.c
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
#include	"obj_format.h"
#include	"asm_lang.h"
#include	"obj_code.h"
#include	"io.h"

/*struct _obj_code
{
	catalog_t			catalog;
};

typedef	struct _obj_code	obj_code_t;*/

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

static int obj_code_read_hex_rec(unsigned char *buf, int offs, int count, int rtype, void *inp);

// -----------------------------------------------------------------------------
//							Public functions
// -----------------------------------------------------------------------------

int	obj_code_save_hex(void *obj_code, void *out)
{
	catalog_t			*cat = (catalog_t *) obj_code;
	char				line[HEX_BUF_LENGTH];
	int					/*count,*/sum, size, i, j, k, l;
	
	size = cat->size;
	i = 0;
	while(i < size)
	{
		j = 0;
		sum = 0;
		if (i > (size - 32))
			k = size - i;
		else
			k = HEX_DATA_COUNT;
		sprintf(&line[j], ":%02X%04X00",k,(i & 0xFFFF));
		j += 9;
		sum = (k & 255) + ((i >> 8) & 255) + (i & 255);
		for (l = 0; l < k; l++)
		{
			sprintf(&line[j], "%02X", ((unsigned char *)cat)[i]);
			sum += ((unsigned char *)cat)[i];
			j +=2;
			i++;
		}
		sprintf(&line[j], "%02X", (-sum) & 255);
		j += 2;
		line[j] = 0;
		if (io_put(out, line) != 0)
			return -1;
	}
	sprintf(line, ":00000001FF");
	line[HEX_EOF_REC_LENGTH] = 0;
	if (io_put(out, line) != 0)
		return -1;
	return 0;
}

int	obj_code_save_c(void *obj_code, void *out)
{
	catalog_t			*cat = (catalog_t *) obj_code;
	char				line[C_BUF_LENGTH];
	int					size, i;
	unsigned char		*b = (unsigned char *) obj_code;
	
	size = cat->size;
	//fprintf(stderr, "size = %d\n", size);
	sprintf(line,"const unsigned char _object_codes[%d] =", size);
	line[33] = 0;
	if (io_write(out, line, 33) != 0)
		return -1;
	sprintf(line, "{");
	if (io_put(out, line) != 0)
		return -1;

	i = 0;
	while (i < size)
	{
		if ((size - i) > 4)
		{
			sprintf(line,"\t0x%02X, 0x%02X, 0x%02X, 0x%02X,", (unsigned int)b[i], (unsigned int)b[i+1], (unsigned int)b[i+2], (unsigned int)b[i+3]);
			if (io_put(out, line) != 0)
			return -1;
		}
		else
		{
			switch(size - i)
			{
				case	4:
					sprintf(line,"\t0x%02X, 0x%02X, 0x%02X, 0x%02X", (unsigned int)b[i], (unsigned int)b[i+1], (unsigned int)b[i+2], (unsigned int)b[i+3]);
					if (io_put(out, line) != 0)
						return -1;
				case	3:
					sprintf(line,"\t0x%02X, 0x%02X, 0x%02X", (unsigned int)b[i], (unsigned int)b[i+1], (unsigned int)b[i+2]);
					if (io_put(out, line) != 0)
						return -1;
					break;
				case	2:
					sprintf(line,"\t0x%02X, 0x%02X", (unsigned int)b[i], (unsigned int)b[i+1]);
					if (io_put(out, line) != 0)
						return -1;
					break;
				case	1:
					sprintf(line,"\t0x%02X", (unsigned int)b[i]);
					if (io_put(out, line) != 0)
						return -1;
					break;
			}
			break;
		}
		i += 4;
	}
	sprintf(line, "};");
	if (io_put(out, line) != 0)
		return -1;
	return 0;
}

int	obj_code_save_bin(void *obj_code, void *out)
{
	catalog_t	*cat = (catalog_t *) obj_code;
	int			size;
	
	size = cat->size;
	//fprintf(stderr, "size = %d\n", size);
	return io_write(out, (char *) obj_code, size);
}


void *obj_code_read_hex(void *inp)
{
	catalog_t		cat;
	int				offs, count, size;
	unsigned char	*buf;
	
	buf = NULL;
	offs = 0;
	count = HEX_DATA_COUNT;
	if (obj_code_read_hex_rec((unsigned char *)(&cat), offs, count, 0, inp) == -1) return NULL;
	size = cat.size;
	if ((buf = malloc(size)) == NULL) return NULL;
	memcpy((void *) buf, (void *)(&cat), size);
	offs += HEX_DATA_COUNT;
	while (offs < size)
	{
		if (offs > size - HEX_DATA_COUNT)
			count = size - offs;
		if (obj_code_read_hex_rec(buf, offs, count, 0, inp) == -1) return NULL;
		offs += count;
	}
	if (obj_code_read_hex_rec(buf, 0, 0, 1, inp) == -1) return NULL;
	return buf;
}

static int obj_code_read_hex_rec(unsigned char *buf, int offs, int count, int rtype, void *inp)
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

void	*obj_code_read_bin(void *inp)
{
	catalog_t	cat;
	int			size, n, cat_size;
	void		*obj_code;
	
	cat_size = sizeof(catalog_t);
	n = io_read(inp, (char *)&cat, cat_size);
	if (n != cat_size) return NULL;
	size = cat.size;
	if ((obj_code = malloc(size)) == NULL) return NULL;
	memcpy(obj_code, (void *) &cat, cat_size);
	size -= cat_size;
	n = io_read(inp, (char *)(obj_code + cat_size), size);
	if (n != size)
	{
		free(obj_code);
		return NULL;
	}
	return obj_code;
}
