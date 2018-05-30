/*******************************************************************************
 * 							Primitive Adder
 * *****************************************************************************
 * 	Filename:		FFT.h
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

#ifndef	NEGATE_PRIMITIVE_H
#define	NEGATE_PRIMITIVE_H

#include	"../../include/primitive_interface.h"

#define	VECTOR_SIZE	1

struct	_negate_self
{
	char	*instance_name;
};

typedef	struct _negate_self	negate_self_t;

struct	_negate_context
{
	negate_self_t	*self;
	float			*inp;
	float			*out;
}__attribute__((packed));

typedef struct _negate_context	negate_context_t;

int	negate_load(void *context);
int	negate_init(void *context);
int	negate_fire(void *context);
int	negate_cleanup(void *context);
int	negate_delete(void *context);

#endif


