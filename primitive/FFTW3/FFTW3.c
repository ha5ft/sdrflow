/*******************************************************************************
 * 							Primitive FFTW3
 * *****************************************************************************
 * 	Filename:		FFTW3.c
 * 	Platform:		ubuntu 16.04 64 bit
 * 	Author:			
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
 *  along with sdrflow.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 *							Revision history
 *******************************************************************************
 *	Author				Date		Comment
 *******************************************************************************
 *	
 *
 ******************************************************************************/

#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	"FFTW3.h"

primitive_catalog_t	FFTW3_catalog =
{
	.name =			"FFTW3",
	.self_size =	sizeof(FFTW3_self_t),
	.init =			&FFTW3_init,
	.fire =			&FFTW3_fire,
	.cleanup =		&FFTW3_cleanup,
	.load =			&FFTW3_load,
	.delete =		&FFTW3_delete
};

int	FFTW3_load(void *sys_catalog)
{
// You should put your own code here
	return 0;
}

int	FFTW3_init(void *context)
{
	FFTW3_context_t	*ctx = (FFTW3_context_t *) context;
	FFTW3_self_t	*self = ctx->self;
	int				size = (1 << *(ctx->log2point));

	self->fft_inp = (double complex *) fftw_malloc(sizeof(double complex) * size);
	self->fft_out = (double complex *) fftw_malloc(sizeof(double complex) * size);
	if (*(ctx->inverse))
	{
		self->plan = fftw_plan_dft_1d(size, self->fft_inp, self->fft_out, FFTW_BACKWARD, FFTW_ESTIMATE);
	}
	else
	{
		self->plan = fftw_plan_dft_1d(size, self->fft_inp, self->fft_out, FFTW_FORWARD, FFTW_ESTIMATE);
	}
	return 0;
}

int	FFTW3_fire(void *context)
{
	FFTW3_context_t	*ctx = (FFTW3_context_t *) context;
	FFTW3_self_t	*self = ctx->self;
	int				size = (1 << *(ctx->log2point));
	int				i;
	double complex	*fft_inp = self->fft_inp;
	double complex	*fft_out = self->fft_out;
	float			*inp_re = ctx->inp_i;
	float			*inp_im = ctx->inp_q;
	float			*out_re = ctx->out_i;
	float			*out_im = ctx->out_q;
	double			scale = 1.0;

	if (*(ctx->inverse))
		scale = 1.0 / ((double) size);

	for (i = 0; i < size; i++)
	{
		fft_inp[i] = inp_re[i] + I*inp_im[i];
	}
	fftw_execute(self->plan);
	for (i = 0; i < size; i++)
	{
		out_re[i] = scale * creal(fft_out[i]);
		out_im[i] = scale * cimag(fft_out[i]);
	}

	return 0;
}

int	FFTW3_cleanup(void *context)
{
	FFTW3_context_t	*ctx = (FFTW3_context_t *) context;
	FFTW3_self_t	*self = ctx->self;

	fftw_destroy_plan(self->plan);
	fftw_free(self->fft_inp);
	fftw_free(self->fft_out);

	return 0;
}

int	FFTW3_delete(void *sys_catalog)
{
// You should put your own code here
	return 0;
}

