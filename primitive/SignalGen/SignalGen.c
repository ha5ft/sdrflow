/*******************************************************************************
 * 							Primitive SignalGen
 * *****************************************************************************
 * 	Filename:		SignalGen.c
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
#include 	<math.h>
#include	"../../include/primitive_interface.h"
#include	"SignalGen.h"

#define	SAMPLE_RATE	48000.0

primitive_catalog_t	SignalGen_catalog =
{
	.name =			"SignalGen",
	.self_size =	sizeof(signalgen_self_t),
	.init =			&signalgen_init,
	.fire =			&signalgen_fire,
	.cleanup =		&signalgen_cleanup,
	.load =			&signalgen_load,
	.delete =		&signalgen_delete
};

int	signalgen_load(void *context)
{
	return 0;
}

int	signalgen_init(void *context)
{
	signalgen_self_t	*self;
	signalgen_context_t	*ctx = (signalgen_context_t *) context;

	self = ctx->self;
	self->phase = 0;
	self->freq = *ctx->freq;
	self->gain = *ctx->gain;
	self->phase_increment = 2.0 * M_PI * (double) self->freq / SAMPLE_RATE;

	return 0;
}

int	signalgen_fire(void *context)
{
#ifndef _BENCHMARK_
	signalgen_context_t	*ctx = (signalgen_context_t *) context;
	signalgen_self_t	*self = ctx->self;
	int					i = 0, negative_freq = 0;
	double				twopi = 2.0 * M_PI;
	double				phase;
	double				gain;
	double				phase_increment;
	float				*out_re = ctx->out_re;
	float				*out_im = ctx->out_im;
	if (self->freq != *ctx->freq)
	{
		self->freq = *ctx->freq;
		self->phase_increment = 2.0 * M_PI * self->freq / SAMPLE_RATE;
	}
	self->gain = *ctx->gain;
	gain = self->gain;
	phase = self->phase;
	phase_increment = self->phase_increment;

	if (self->freq < 0.0) negative_freq = -1;

	for (i = 0; i < VECTOR_SIZE; i++)
	{
		out_re[i] = (float) (gain * cos(phase));
		out_im[i] = (float) (gain * sin(phase));
		phase += phase_increment;
		if (negative_freq)
		{
			if (phase <= -twopi) phase += twopi;
		}
		else
		{
			if (phase >= twopi) phase -= twopi;
		}
	}
	self->phase = phase;
#endif
	return 0;
}

int	signalgen_cleanup(void *context)
{
	return 0;
}

int	signalgen_delete(void *context)
{
	return 0;
}

