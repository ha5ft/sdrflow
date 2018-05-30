/*******************************************************************************
 * 							Primitive MultiplierExt
 * *****************************************************************************
 * 	Filename:		MultiplierExt.c
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
#include	"MultiplierExt.h"

primitive_catalog_t	MultiplierExt_catalog =
{
	.name =			"MultiplierExt",
	.self_size =	sizeof(multiplierext_self_t),
	.init =			&multiplierext_init,
	.fire =			&multiplierext_fire,
	.cleanup =		&multiplierext_cleanup,
	.load =			&multiplierext_load,
	.delete =		&multiplierext_delete
};

int	multiplierext_load(void *context)
{
	return 0;
}

int	multiplierext_init(void *context)
{
	return 0;
}

int	multiplierext_fire(void *context)
{
#ifndef _BENCHMARK_
	float	*a_re = ((multiplierext_context_t *)context)->a_re;
	float	*a_im = ((multiplierext_context_t *)context)->a_im;
	float	*b_re = ((multiplierext_context_t *)context)->b_re;
	float	*b_im = ((multiplierext_context_t *)context)->b_im;
	float	*axb_re = ((multiplierext_context_t *)context)->axb_re;
	float	*axb_im = ((multiplierext_context_t *)context)->axb_im;
	int		i = 0;

//	fprintf(stderr,"MultplierExt\n");
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		axb_re[i] = a_re[i] * b_re[i] - a_im[i] * b_im[i];
		axb_im[i] = a_re[i] * b_im[i] + a_im[i] * b_re[i];
	}
#endif
	return 0;
}

int	multiplierext_cleanup(void *context)
{
	return 0;
}

int	multiplierext_delete(void *context)
{
	return 0;
}

