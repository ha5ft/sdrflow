/*******************************************************************************
 * 							Primitive Adder
 * *****************************************************************************
 * 	Filename:		Adder.c
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
 *	Selmeczi János		23-04-2018	original version
 *
 ******************************************************************************/

#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	"../../include/primitive_interface.h"
#include	"Adder.h"

primitive_catalog_t	Adder_catalog =
{
	.name =			"Adder",
	.self_size =	sizeof(adder_self_t),
	.init =			&adder_init,
	.fire =			&adder_fire,
	.cleanup =		&adder_cleanup,
	.load =			&adder_load,
	.delete =		&adder_delete
};

int	adder_load(void *ctx)
{
	return 0;
}

int	adder_init(void *ctx)
{
	return 0;
}

int	adder_fire(void *context)
{
#ifndef _BENCHMARK_
	float	*a_re = ((adder_context_t *)context)->a_re;
	float	*a_im = ((adder_context_t *)context)->a_im;
	float	*b_re = ((adder_context_t *)context)->b_re;
	float	*b_im = ((adder_context_t *)context)->b_im;
	float	*apb_re = ((adder_context_t *)context)->apb_re;
	float	*apb_im = ((adder_context_t *)context)->apb_im;
	int		i;

	for (i = 0; i < VECTOR_SIZE; i++)
	{
		apb_re[i] = a_re[i] + b_re[i];
		apb_im[i] = a_im[i] + b_im[i];
	}
#endif
	return 0;
}

int	adder_cleanup(void *context)
{
	return 0;
}

int	adder_delete(void *context)
{
	return 0;
}

