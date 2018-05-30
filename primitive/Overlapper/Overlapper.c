/*******************************************************************************
 * 							Primitive Overlapper
 * *****************************************************************************
 * 	Filename:		Overlapper.c
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
#include	"Overlapper.h"

primitive_catalog_t	Overlapper_catalog =
{
	.name =			"Overlapper",
	.self_size =	sizeof(overlapper_self_t),
	.init =			&overlapper_init,
	.fire =			&overlapper_fire,
	.cleanup =		&overlapper_cleanup,
	.load =			&overlapper_load,
	.delete =		&overlapper_delete
};

int	overlapper_load(void *context)
{
	return 0;
}

int	overlapper_init(void *context)
{
	float	*last_re = ((overlapper_context_t *)context)->self->last_re;
	float	*last_im = ((overlapper_context_t *)context)->self->last_im;
	int		i;

	for (i = 0; i < INPUT_VECTOR_SIZE; i++)
	{
		last_re[i] = 0.0;
		last_im[i] = 0.0;
	}
	return 0;
}

int	overlapper_fire(void *context)
{
#ifndef _BENCHMARK_
	float	*inp_re = ((overlapper_context_t *)context)->inp_re;
	float	*inp_im = ((overlapper_context_t *)context)->inp_im;
	float	*out_re = ((overlapper_context_t *)context)->out_re;
	float	*out_im = ((overlapper_context_t *)context)->out_im;
	float	*last_re = ((overlapper_context_t *)context)->self->last_re;
	float	*last_im = ((overlapper_context_t *)context)->self->last_im;
	int		i, j;

//	fprintf(stderr,"Overlapper\n");
	for (i = 0; i < INPUT_VECTOR_SIZE; i++)
	{
		out_re[i] = last_re[i];
		out_im[i] = last_im[i];
	}
	for (i = 0, j = INPUT_VECTOR_SIZE; i < INPUT_VECTOR_SIZE; i++, j++)
	{
		out_re[j] = last_re[i] = inp_re[i];
		out_im[j] = last_im[i] = inp_im[i];
	}
#endif
	return 0;
}

int	overlapper_cleanup(void *context)
{
	return 0;
}

int	overlapper_delete(void *context)
{
	return 0;
}

