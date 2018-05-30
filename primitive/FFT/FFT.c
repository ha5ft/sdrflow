/*******************************************************************************
 * 							Primitive FFT
 * *****************************************************************************
 * 	Filename:		FFT.c
 * 	Platform:		ubuntu 16.04 64 bit
 * 	Author:			Copyright (C) Krüpl Zsolt hg2ecz, original version
 *					Copyright (C) Selmeczi János, modifications
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
 *	Krüpl Zsolt hg5ecz	13-02-2018	Original
 *	Selmeczi János		23-04-2018	Modification for sdrflow
 *
 ******************************************************************************/

#include	<math.h>
#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	"../../include/primitive_interface.h"
#include	"FFT.h"

struct _sample {
    float i;
    float q;
};

primitive_catalog_t	FFT_catalog =
{
	.name =			"FFT",
	.self_size =	sizeof(fft_self_t),
	.init =			&fft_init,
	.fire =			&fft_fire,
	.cleanup =		&fft_cleanup,
	.load =			&fft_load,
	.delete =		&fft_delete
};

static int 		phasevec_exist = 0;
static struct _sample phasevec[32];


int	fft_load(void *context)
{
    if (!phasevec_exist)
	{
		for (int i=0; i<32; i++)
		{
	    	int point = 2<<i;
	    	phasevec[i].i = cos(-2*M_PI/point);
	    	phasevec[i].q = sin(-2*M_PI/point);
		}
		phasevec_exist = 1;
    }

	return 0;
}

int	fft_init(void *context)
{
	return 0;
}

int	fft_fire(void *context)
{
#ifndef _BENCHMARK_
	fft_context_t	*ctx = (fft_context_t *) context;
//	fft_self_t		*self = ctx->self;
	float			*inp_re = ctx->inp_i;
	float			*inp_im = ctx->inp_q;
	float			*out_re = ctx->out_i;
	float			*out_im = ctx->out_q;
	int				log2point = 11;//*(ctx->log2point);
	int				inverse = *(ctx->inverse);

	struct _sample	xy_in[2048];
	struct _sample	xy_out[2048];

//	fprintf(stderr,"FFT\n");

	for (int i = 0; i < 2048; i++)
	{
		xy_in[i].i = inp_re[i];
		xy_in[i].q = inp_im[i];
	}	

    if (!phasevec_exist)
	{
		for (int i=0; i<32; i++)
		{
	    	int point = 2<<i;
	    	phasevec[i].i = cos(-2*M_PI/point);
	    	phasevec[i].q = sin(-2*M_PI/point);
		}
		phasevec_exist = 1;
    }

    for (int i=0; i < (1<<log2point); i++) {
	unsigned int brev = i;
	brev = ((brev & 0xaaaaaaaa) >> 1) | ((brev & 0x55555555) << 1);
	brev = ((brev & 0xcccccccc) >> 2) | ((brev & 0x33333333) << 2);
	brev = ((brev & 0xf0f0f0f0) >> 4) | ((brev & 0x0f0f0f0f) << 4);
	brev = ((brev & 0xff00ff00) >> 8) | ((brev & 0x00ff00ff) << 8);
	brev = (brev >> 16) | (brev << 16);

	brev >>= 32-log2point;
	xy_out[brev] = xy_in[i];
    }

	if (inverse)
	{
    	for (int i=0; i < (1<<log2point); i++)
		{
			xy_out[i].q *= -1.0;
		}
	}
    // here begins the Danielson-Lanczos section
    int n = 1<<log2point;
    int l2pt=0;
    int mmax=1;
#ifdef MOD_SPEED
    while (l2pt < log2point) {
	int istep = 2<<l2pt;
#else
    while (n>mmax) {
	int istep = mmax<<1;
#endif
//	int theta = -2*M_PI/istep;
//	double complex wphase_XY = cos(theta) + sin(theta)*I;
	struct _sample wphase_XY = phasevec[l2pt];
	l2pt++;

	struct _sample w_XY = { 1., 0. };
	for (int m=0; m < mmax; m++) {
	    for (int i=m; i < n; i += istep) {
		struct _sample tempXY;
		tempXY.i = w_XY.i * xy_out[i+mmax].i - w_XY.q * xy_out[i+mmax].q;
		tempXY.q = w_XY.i * xy_out[i+mmax].q + w_XY.q * xy_out[i+mmax].i;
		xy_out[i+mmax].i  = xy_out[i].i - tempXY.i;
		xy_out[i+mmax].q  = xy_out[i].q - tempXY.q;
		xy_out[i     ].i += tempXY.i;
		xy_out[i     ].q += tempXY.q;
	    }
	    float w_tmp = w_XY.i * wphase_XY.i - w_XY.q * wphase_XY.q;
	    w_XY.q      = w_XY.i * wphase_XY.q + w_XY.q * wphase_XY.i;
	    w_XY.i      = w_tmp;
	}
	mmax=istep;
    }
	if (inverse)
	{
		float	N = 1.0 / (float) n;
		float	MN  = -N;
    	for (int i=0; i < (1<<log2point); i++)
		{
			xy_out[i].i *= N;
			xy_out[i].q *= MN;
		}
	}
	for (int i = 0; i < 2048; i++)
	{
		out_re[i] = xy_out[i].i;
		out_im[i] = xy_out[i].q;
	}
#endif
	return 0;
}

int	fft_cleanup(void *context)
{
	return 0;
}

int	fft_delete(void *context)
{
	return 0;
}


