/*******************************************************************************
 * 							Primitive Overlapper
 * *****************************************************************************
 * 	Filename:		Overlapper.h
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

#ifndef	OVERLAPPER_PRIMITIVE_H
#define	OVERLAPPER_PRIMITIVE_H

#include	"../../include/primitive_interface.h"

#define	INPUT_VECTOR_SIZE	1024
#define	OUTPUT_VECTOR_SIZE	2048

struct	_overlapper_self
{
	char	*instance_name;
	float	last_re[INPUT_VECTOR_SIZE];
	float	last_im[INPUT_VECTOR_SIZE];
};

typedef	struct _overlapper_self	overlapper_self_t;

struct	_overlapper_context
{
	overlapper_self_t	*self;
	float			*inp_re;
	float			*inp_im;
	float			*out_re;
	float			*out_im;
}__attribute__((packed));

typedef struct _overlapper_context	overlapper_context_t;

int	overlapper_load(void *context);
int	overlapper_init(void *context);
int	overlapper_fire(void *context);
int	overlapper_cleanup(void *context);
int	overlapper_delete(void *context);

#endif


