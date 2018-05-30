/*******************************************************************************
 * 							Common Allocators
 * *****************************************************************************
 * 	Filename:		allocators.h
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
#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#define ALLOC_COUNT			16

#define	NULL_ALLOC			0
#define	CHAR_ALLOC			1
#define	UCHAR_ALLOC			2
#define SHORT_ALLOC			3
#define USHORT_ALLOC		4
#define INT_ALLOC			5
#define UINT_ALLOC			6
#define LONG_ALLOC			7
#define ULONG_ALLOC			8
#define FLOAT_ALLOC			9
#define DOUBLE_ALLOC		10
#define PTR_ALLOC			11
#define	STRING_ALLOC		12
#define ADDR_ALLOC			13
#define OFFSET_ALLOC		14
#define	CHARARR_ALLOC		15

#define	NULL_ALLOC_STR		"null"
#define	CHAR_ALLOC_STR		"char"
#define	UCHAR_ALLOC_STR		"uchar"
#define SHORT_ALLOC_STR		"short"
#define USHORT_ALLOC_STR	"ushort"
#define INT_ALLOC_STR		"int"
#define UINT_ALLOC_STR		"uint"
#define LONG_ALLOC_STR		"long"
#define ULONG_ALLOC_STR		"ulong"
#define FLOAT_ALLOC_STR		"float"
#define DOUBLE_ALLOC_STR	"double"
#define PTR_ALLOC_STR		"ptr"
#define STRING_ALLOC_STR	"string"
#define	ADDR_ALLOC_STR		"address"
#define	OFFSET_ALLOC_STR	"offset"
#define	CHARARR_ALLOC_STR	"chararr"

#endif

