/*******************************************************************************
 * 							Common value
 * *****************************************************************************
 * 	Filename:		value.h
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

#ifndef	VALUE_H
#define	VALUE_H

// all value types are little endian encoded

#define VALUE_TYPE_COUNT		16

#define VALUE_TYPE_NULL			0
#define VALUE_TYPE_CHAR			1
#define VALUE_TYPE_UCHAR		2
#define VALUE_TYPE_SHORT		3
#define VALUE_TYPE_USHORT		4
#define VALUE_TYPE_INT			5
#define VALUE_TYPE_UINT			6
#define VALUE_TYPE_LONG			7
#define VALUE_TYPE_ULONG		8
#define VALUE_TYPE_FLOAT		9
#define VALUE_TYPE_DOUBLE		10
#define VALUE_TYPE_PTR			11
#define VALUE_TYPE_STRING		12
#define	VALUE_TYPE_ADDR			13
#define	VALUE_TYPE_OFFSET		14
#define	VALUE_TYPE_CHARARR		15

#define VALUE_NULL_SIZE_64		0
#define VALUE_CHAR_SIZE_64		1
#define VALUE_UCHAR_SIZE_64		1
#define VALUE_SHORT_SIZE_64		2
#define VALUE_USHORT_SIZE_64	2
#define VALUE_INT_SIZE_64		4
#define VALUE_UINT_SIZE_64		4
#define VALUE_LONG_SIZE_64		8
#define VALUE_ULONG_SIZE_64		8
#define VALUE_FLOAT_SIZE_64		4
#define VALUE_DOUBLE_SIZE_64	8
#define VALUE_PTR_SIZE_64		8
#define VALUE_STRING_SIZE_64	1
#define	VALUE_ADDR_SIZE_64		4
#define	VALUE_OFFSET_SIZE_64	4
#define	VALUE_CHARARR_SIZE_64	8

#define VALUE_NULL_SIZE_32		0
#define VALUE_CHAR_SIZE_32		1
#define VALUE_UCHAR_SIZE_32		1
#define VALUE_SHORT_SIZE_32		2
#define VALUE_USHORT_SIZE_32	2
#define VALUE_INT_SIZE_32		4
#define VALUE_UINT_SIZE_32		4
#define VALUE_LONG_SIZE_32		4
#define VALUE_ULONG_SIZE_32		4
#define VALUE_FLOAT_SIZE_32		4
#define VALUE_DOUBLE_SIZE_32	8
#define VALUE_PTR_SIZE_32		4
#define VALUE_STRING_SIZE_32	1
#define	VALUE_ADDR_SIZE_32		4
#define	VALUE_OFFSET_SIZE_32	4
#define	VALUE_CHARARR_SIZE_32	8

#define VALUE_NULL_SIZE_16		0
#define VALUE_CHAR_SIZE_16		1
#define VALUE_UCHAR_SIZE_16		1
#define VALUE_SHORT_SIZE_16		2
#define VALUE_USHORT_SIZE_16	2
#define VALUE_INT_SIZE_16		2
#define VALUE_UINT_SIZE_16		2
#define VALUE_LONG_SIZE_16		4
#define VALUE_ULONG_SIZE_16		4
#define VALUE_FLOAT_SIZE_16		4
#define VALUE_DOUBLE_SIZE_16	8
#define VALUE_PTR_SIZE_16		2
#define VALUE_STRING_SIZE_16	1
#define	VALUE_ADDR_SIZE_16		2
#define	VALUE_OFFSET_SIZE_16	2
#define	VALUE_CHARARR_SIZE_16	8

#define	VALUE_INT64_MAX			0x7FFFFFFFFFFFFFFF
#define VALUE_INT64_MIN			0x8000000000000000
#define	VALUE_UINT64_MAX		0x7FFFFFFFFFFFFFFF
#define	VALUE_UINT64_MIN		0
#define	VALUE_INT32_MAX			0x000000007FFFFFFF
#define	VALUE_INT32_MIN			0xFFFFFFFF80000000
#define	VALUE_UINT32_MAX		0x00000000FFFFFFFF
#define	VALUE_UINT32_MIN		0
#define	VALUE_INT16_MAX			0x0000000000007FFF
#define	VALUE_INT16_MIN			0xFFFFFFFFFFFF8000
#define	VALUE_UINT16_MAX		0x000000000000FFFF
#define	VALUE_UINT16_MIN		0
#define	VALUE_INT8_MAX			0x000000000000007F
#define	VALUE_INT8_MIN			0xFFFFFFFFFFFFFF80
#define	VALUE_UINT8_MAX			0x00000000000000FF
#define	VALUE_UINT8_MIN			0


typedef struct	_addr
{
	unsigned int	seg;
	unsigned int	offs;
} addr_t;

typedef union _value
{
	char			ch;
	unsigned char	uch;
	short			sh;
	unsigned short	ush;
	int				i;
	unsigned int	ui;
	long			l;
	unsigned long	ul;
	float			f;
	double			d;
	char			*s;
	void			*p;
	addr_t			a;
	char			c[8];
} value_t;

#endif
