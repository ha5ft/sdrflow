/*******************************************************************************
 * 							Primitive Multiplier
 * *****************************************************************************
 * 	Filename:		Multiplier.c
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
#include	"Multiplier.h"

primitive_catalog_t	Multiplier_catalog =
{
	.name =			"Multiplier",
	.self_size =	sizeof(multiplier_self_t),
	.init =			&multiplier_init,
	.fire =			&multiplier_fire,
	.cleanup =		&multiplier_cleanup,
	.load =			&multiplier_load,
	.delete =		&multiplier_delete
};

int	multiplier_load(void *context)
{
	return 0;
}

int	multiplier_init(void *context)
{
	return 0;
}

int	multiplier_fire(void *context)
{
#ifndef _BENCHMARK_
	float	*a_re = ((multiplier_context_t *)context)->a_re;
	float	*a_im = ((multiplier_context_t *)context)->a_im;
	float	*b_re = ((multiplier_context_t *)context)->b_re;
	float	*b_im = ((multiplier_context_t *)context)->b_im;
	float	*axb_re = ((multiplier_context_t *)context)->axb_re;
	float	*axb_im = ((multiplier_context_t *)context)->axb_im;
	int		i = 0;

	for (i = 0; i < VECTOR_SIZE; i++)
	{
		axb_re[i] = a_re[i] * b_re[i] - a_im[i] * b_im[i];
		axb_im[i] = a_re[i] * b_im[i] + a_im[i] * b_re[i];
	}
#endif
	return 0;
}

int	multiplier_cleanup(void *context)
{
	return 0;
}

int	multiplier_delete(void *context)
{
	return 0;
}

