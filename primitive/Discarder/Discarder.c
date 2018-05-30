/*******************************************************************************
 * 							Primitive Discarder
 * *****************************************************************************
 * 	Filename:		Discarder.c
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
#include	"Discarder.h"

primitive_catalog_t	Discarder_catalog =
{
	.name =			"Discarder",
	.self_size =	sizeof(discarder_self_t),
	.init =			&discarder_init,
	.fire =			&discarder_fire,
	.cleanup =		&discarder_cleanup,
	.load =			&discarder_load,
	.delete =		&discarder_delete
};

int	discarder_load(void *context)
{
	return 0;
}

int	discarder_init(void *context)
{
	return 0;
}

int	discarder_fire(void *context)
{
#ifndef _BENCHMARK_
	float	*inp_re = ((discarder_context_t *)context)->inp_re;
	float	*inp_im = ((discarder_context_t *)context)->inp_im;
	float	*out_re = ((discarder_context_t *)context)->out_re;
	float	*out_im = ((discarder_context_t *)context)->out_im;
	int		i;
//	int		offset = INPUT_VECTOR_SIZE - OUTPUT_VECTOR_SIZE;
	int		offset = 0;
//	fprintf(stderr,"Discarder\n");
	for (i = 0; i < OUTPUT_VECTOR_SIZE; i++)
	{
		out_re[i] = inp_re[i + offset];
		out_im[i] = inp_im[i + offset];
	}
#endif
	return 0;
}

int	discarder_cleanup(void *context)
{
	return 0;
}

int	discarder_delete(void *context)
{
	return 0;
}

