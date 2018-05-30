/*******************************************************************************
 * 							Primitive Window
 * *****************************************************************************
 * 	Filename:		Window.c
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
#include	<math.h>
#include	"../../include/primitive_interface.h"
#include	"Window.h"

#define	WINDOW_SIZE 1025

primitive_catalog_t	Window_catalog =
{
	.name =			"Window",
	.self_size =	sizeof(window_self_t),
	.init =			&window_init,
	.fire =			&window_fire,
	.cleanup =		&window_cleanup,
	.load =			&window_load,
	.delete =		&window_delete
};

int	window_load(void *ctx)
{
	return 0;
}

int	window_init(void *ctx)
{
	return 0;
}

int	window_fire(void *context)
{
#ifndef _BENCHMARK_
	float	*out_re = ((window_context_t *)context)->out_re;
	float	*out_im = ((window_context_t *)context)->out_im;
	double	a0 = 0.35875;
	double	a1 = 0.48829;
	double	a2 = 0.14128;
	double	a3 = 0.01168;
	double	twopi = 2.0 * M_PI;
	double	size = (double) (WINDOW_SIZE - 1);
	double	r;
	int		i;
//	int		offset = 0;
	int		offset = VECTOR_SIZE - WINDOW_SIZE;

//	fprintf(stderr,"Window\n");
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		out_re[i] = out_im[i] = 0.0;
	}

	for (i = 0; i < WINDOW_SIZE; i++)
	{
		r = ((double) i / size) * twopi;
		out_re[i + offset] =	(	a0 -
						a1 * cos(r) +
						a2 * cos(2.0 * r) -
						a3 * cos(3.0 * r)
					);
		out_im[i + offset] = 0.0;
	}
#endif
	return 0;
}

int	window_cleanup(void *context)
{
	return 0;
}

int	window_delete(void *context)
{
	return 0;
}

