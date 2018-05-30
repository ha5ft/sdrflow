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
#include 	<math.h>
#include 	<pulse/simple.h>
#include 	<pulse/error.h>
#include	"../../include/primitive_interface.h"
#include	"AudioSink.h"

#define SAMPLE_RATE			48000
#define CHANNEL_COUNT		1

primitive_catalog_t	AudioSink_catalog =
{
	.name =			"AudioSink",
	.self_size =	sizeof(audiosink_self_t),
	.init =			&audiosink_init,
	.fire =			&audiosink_fire,
	.cleanup =		&audiosink_cleanup,
	.load =			&audiosink_load,
	.delete =		&audiosink_delete
};

static const	pa_sample_spec audiosink_ss =
				{
        			.format = PA_SAMPLE_FLOAT32LE,
        			.rate = SAMPLE_RATE,
        			.channels = CHANNEL_COUNT
    			};


int	audiosink_load(void *ctx)
{
	return 0;
}

int	audiosink_init(void *context)
{
	audiosink_self_t	*self = ((audiosink_context_t *) context)->self;
    int 				error;

    if (!(self->s = pa_simple_new(NULL, "sdrflow", PA_STREAM_PLAYBACK, NULL, "cwsignal", &audiosink_ss, NULL, NULL, &error))) 
	{
        fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
		return -1;
    }

	return 0;
}

int	audiosink_fire(void *context)
{
#ifndef _BENCHMARK_
	audiosink_self_t	*self = ((audiosink_context_t *) context)->self;
	float				*buf;
	float				*inp = ((audiosink_context_t *) context)->inp_re;
	int					i;
    int 				error;
	size_t				buf_byte_size;
	buf = self->buf;
	buf_byte_size = VECTOR_SIZE * sizeof(float);

	i = 0;
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		buf[i] = inp[i];
	}

    if (pa_simple_write(self->s, buf, buf_byte_size, &error) < 0)
	{
		fprintf(stderr, __FILE__": pa_simple_write() failed: %s\n", pa_strerror(error));
		return -1;
	}
#endif
	return 0;
}

int	audiosink_cleanup(void *context)
{
	audiosink_self_t	*self = ((audiosink_context_t *) context)->self;
    int 				error;

    if (pa_simple_drain(self->s, &error) < 0)
	{
        fprintf(stderr, __FILE__": pa_simple_drain() failed: %s\n", pa_strerror(error));
    }
	if (self->s)
        pa_simple_free(self->s);

	return 0;
}

int	audiosink_delete(void *context)
{
	return 0;
}

