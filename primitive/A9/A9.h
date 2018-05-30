/*******************************************************************************
 * 							Primitive A9
 * *****************************************************************************
 * 	Filename:		A9.h
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

#ifndef	A9_PRIMITIVE_H
#define	A9_PRIMITIVE_H

struct _A9_self
{
	char	*instance_name;
	int		fire_count;
}__attribute__((packed));

typedef struct _A9_self	A9_self_t;

struct _A9
{
	A9_self_t	*self;
    int     	*i1;
    int     	*o1;
}__attribute__((packed));

typedef struct _A9	A9_t;

int A9_init(void *ctx);
int A9_fire(void *ctx);
int A9_cleanup(void *ctx);
int	A9_load(void *ctx);
int	A9_delete(void *ctx);

#endif
