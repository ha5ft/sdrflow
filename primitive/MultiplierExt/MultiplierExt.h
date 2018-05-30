/*******************************************************************************
 * 							Primitive Multiplier
 * *****************************************************************************
 * 	Filename:		Multiplier.h
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

#ifndef	MULTIPLIEREXT_PRIMITIVE_H
#define	MULTIPLIEREXT_PRIMITIVE_H

#include	"../../include/primitive_interface.h"

#define	VECTOR_SIZE	2048

struct	_multiplierext_self
{
	char	*instance_name;
};

typedef	struct _multiplierext_self	multiplierext_self_t;

struct	_multiplierext_context
{
	multiplierext_self_t	*self;
	float				*a_re;
	float				*a_im;
	float				*b_re;
	float				*b_im;
	float				*axb_re;
	float				*axb_im;
}__attribute__((packed));

typedef struct _multiplierext_context	multiplierext_context_t;

int	multiplierext_load(void *context);
int	multiplierext_init(void *context);
int	multiplierext_fire(void *context);
int	multiplierext_cleanup(void *context);
int	multiplierext_delete(void *context);

#endif


