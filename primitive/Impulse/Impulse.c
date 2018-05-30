/*******************************************************************************
 * 							Primitive Impulse
 * *****************************************************************************
 * 	Filename:		Impulse.c
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
#include	"Impulse.h"

#define		IMPULSE_SIZE	1025
#define		MIDPOINT		512
#define		SAMPLE_RATE		48000.0

primitive_catalog_t	Impulse_catalog =
{
	.name =			"Impulse",
	.self_size =	sizeof(impulse_self_t),
	.init =			&impulse_init,
	.fire =			&impulse_fire,
	.cleanup =		&impulse_cleanup,
	.load =			&impulse_load,
	.delete =		&impulse_delete
};

int	impulse_load(void *ctx)
{
	return 0;
}

int	impulse_init(void *ctx)
{
	return 0;
}

int	impulse_fire(void *context)
{
#ifndef _BENCHMARK_
	float	bandwidth = *(((impulse_context_t *)context)->bandwidth);
	float	*out_re = ((impulse_context_t *)context)->out_re;
	float	*out_im = ((impulse_context_t *)context)->out_im;

	double	cutoff = 0.5 * bandwidth;
	double	rc = cutoff / SAMPLE_RATE;
	double	twopi = 2.0 * M_PI;
	double	scale = 2.0;
	int		i;//, j, k;
	int		midpoint = MIDPOINT;
//	int		offset = 0;
	int		offset = VECTOR_SIZE - IMPULSE_SIZE;

//	fprintf(stderr,"Impulse\n");
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		out_re[i] = out_im[i] = 0.0;
	}

	for (i = 0; i < IMPULSE_SIZE; i++)
	{
		if (i != midpoint)
		{
			out_re[i + offset] = (float) (scale * sin(twopi * ((double)(i - midpoint)) *rc) / (M_PI * ((double)(i - midpoint))));
			out_im[i + offset] = 0.0;
		}
		else
		{
			out_re[i + offset] = (float) (scale * 2.0 * rc);
			out_im[i + offset] = 0.0;
		}
	}
#endif
	return 0;
}

int	impulse_cleanup(void *context)
{
	return 0;
}

int	impulse_delete(void *context)
{
	return 0;
}

