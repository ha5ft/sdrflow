/*******************************************************************************
 * 							Primitive VFFT
 * *****************************************************************************
 * 	Filename:		VFFT.c
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
#include	"VFFT.h"

#if defined (__i386) || defined (__x86_64)
# include <immintrin.h>
#elif defined(__arm__) || defined (__aarch64__)
# include <arm_neon.h>
#endif

#if defined(__ARM_NEON)
typedef float32x4_t VECTORTYPE; // 4 pcs parallel
#elif defined(__SSE__)
typedef __m128 VECTORTYPE; // 4 pcs parallel
#endif

#if (defined(__ARM_NEON) || defined(__SSE__)) // hardware vectorization

primitive_catalog_t	VFFT_catalog =
{
	.name =			"VFFT",
	.self_size =	sizeof(vfft_self_t),
	.init =			&vfft_init,
	.fire =			&vfft_fire,
	.cleanup =		&vfft_cleanup,
	.load =			&vfft_load,
	.delete =		&vfft_delete
};

static float	phasevec[32][2];
static int 		phasevec_exist = 0;


int	vfft_load(void *context)
{
    if (!phasevec_exist)
	{
		for (int i=0; i<32; i++)
		{
	   		int point = 2<<i;
	    	phasevec[i][0] = cos(-2*M_PI/point);
	    	phasevec[i][1] = sin(-2*M_PI/point);
		}
		phasevec_exist = 1;
    }

	return 0;
}

int	vfft_init(void *context)
{
	return 0;
}

int	vfft_fire(void *context)
{
#ifndef _BENCHMARK_
	vfft_context_t	*ctx = (vfft_context_t *) context;
//	fft_self_t		*self = ctx->self;
	float			*i_re = ctx->inp_i;
	float			*i_im = ctx->inp_q;
	float			*o_re = ctx->out_i;
	float			*o_im = ctx->out_q;
	int				log2point = 11;//*(ctx->log2point);
	int				inverse = *(ctx->inverse);

	float	inp_re[2048];
	float	inp_im[2048];
	float	out_re[2048];
	float	out_im[2048];

//	fprintf(stderr,"vfft\n");
    for (int i=0; i < (1<<log2point); i++)
	{
		inp_re[i] = i_re[i];
		inp_im[i] = i_im[i];
	}
    if (!phasevec_exist)
	{
		for (int i=0; i<32; i++)
		{
	   		int point = 2<<i;
	    	phasevec[i][0] = cos(-2*M_PI/point);
	    	phasevec[i][1] = sin(-2*M_PI/point);
		}
		phasevec_exist = 1;
    }

    for (int i=0; i < (1<<log2point); i+=2)
	{
		unsigned int brev = i;
		brev = ((brev & 0xaaaaaaaa) >> 1) | ((brev & 0x55555555) << 1);
		brev = ((brev & 0xcccccccc) >> 2) | ((brev & 0x33333333) << 2);
		brev = ((brev & 0xf0f0f0f0) >> 4) | ((brev & 0x0f0f0f0f) << 4);
		brev = ((brev & 0xff00ff00) >> 8) | ((brev & 0x00ff00ff) << 8);
		brev = (brev >> 16) | (brev << 16);

		brev >>= 32-log2point;
		out_re[brev] = inp_re[i];
		out_im[brev] = inp_im[i];

		unsigned int brev2 = brev | (1<<(log2point-1));
		out_re[brev2] = inp_re[i+1];
		out_im[brev2] = inp_im[i+1];
    }

	if (inverse)
	{
    	for (int i=0; i < (1<<log2point); i++)
		{
			out_im[i] *= -1.0;
		}
	}
//	fprintf(stderr,"FFT 1\n");

    // here begins the Danielson-Lanczos section

    int n = 1<<log2point;
    int l2pt=0;
    int mmax=1;

    l2pt++;
    for (int i=0; i < n; i += 2)
	{
		float tempX = out_re[i+mmax];
		float tempY = out_im[i+mmax];
		out_re[i+mmax]  = out_re[i] - tempX;
		out_im[i+mmax]  = out_im[i] - tempY;
		out_re[i     ] += tempX;
		out_im[i     ] += tempY;
    }
//	fprintf(stderr,"FFT 2\n");

    mmax<<=1;

    float w_X2 = phasevec[l2pt][0];
    float w_Y2 = phasevec[l2pt][1];
	l2pt++;

    for (int i=0; i < n; i += 4)
	{
		float tempX = out_re[i+mmax];
		float tempY = out_im[i+mmax];
		out_re[i+mmax]  = out_re[i] - tempX;
		out_im[i+mmax]  = out_im[i] - tempY;
		out_re[i     ] += tempX;
		out_im[i     ] += tempY;

		float tempX2 = (float)w_X2 * out_re[i+1+mmax] - (float)w_Y2 * out_im[i+1+mmax];
		float tempY2 = (float)w_X2 * out_im[i+1+mmax] + (float)w_Y2 * out_re[i+1+mmax];
		out_re[i+1+mmax]  = out_re[i+1] - tempX2;
		out_im[i+1+mmax]  = out_im[i+1] - tempY2;
		out_re[i+1     ] += tempX2;
		out_im[i+1     ] += tempY2;
    }
//	fprintf(stderr,"FFT 3\n");

    mmax<<=1;

    while (n>mmax)
	{
		int istep = mmax<<1;
		float wphase_X = phasevec[l2pt][0];
		float wphase_Y = phasevec[l2pt][1];

//		fprintf(stderr,"mmax=%d\n",mmax);

		VECTORTYPE wphase_Xvec, wphase_Yvec;
		wphase_Xvec[0] = wphase_Xvec[1] = wphase_Xvec[2] = wphase_Xvec[3]= phasevec[l2pt-2][0];
		wphase_Yvec[0] = wphase_Yvec[1] = wphase_Yvec[2] = wphase_Yvec[3]= phasevec[l2pt-2][1];
		l2pt++;

		VECTORTYPE w_Xvec, w_Yvec;
		w_Xvec[0] = 1.;
		w_Yvec[0] = 0.;

		w_Xvec[1] = w_Xvec[0] * wphase_X; // - w_Yvec[0] * wphase_Y;
		w_Yvec[1] = w_Xvec[0] * wphase_Y; // + w_Yvec[0] * wphase_X;

		w_Xvec[2] = w_Xvec[1] * wphase_X - w_Yvec[1] * wphase_Y;
		w_Yvec[2] = w_Xvec[1] * wphase_Y + w_Yvec[1] * wphase_X;

		w_Xvec[3] = w_Xvec[2] * wphase_X - w_Yvec[2] * wphase_Y;
		w_Yvec[3] = w_Xvec[2] * wphase_Y + w_Yvec[2] * wphase_X;

		for (int m=0; m < mmax; m+=4)
		{ // optimization: tempXY and tempXY2
//			fprintf(stderr,"m=%d\n",m);
	    	for (int i=m; i < n; i += istep)
			{
//				if ((m == 0) && (mmax == 4)) fprintf(stderr,"i=%d\n",i);
				VECTORTYPE *reg1_reptr = (VECTORTYPE *)&out_re[i+mmax]; // 4 lanes reg
				VECTORTYPE *reg1_imptr = (VECTORTYPE *)&out_im[i+mmax]; // 4 lanes reg
				VECTORTYPE reg1_re = *reg1_reptr;
//				if ((m == 0) && (mmax == 4)) fprintf(stderr,"FFT 3.1\n");
				VECTORTYPE reg1_im = *reg1_imptr;

//				if ((m == 0) && (mmax == 4)) fprintf(stderr,"FFT 3.2\n");

				VECTORTYPE temp_re = w_Xvec * reg1_re - w_Yvec * reg1_im; // 4 lanes mul
				VECTORTYPE temp_im = w_Xvec * reg1_im + w_Yvec * reg1_re; // 4 lanes mul

//				if ((m == 0) && (mmax == 4)) fprintf(stderr,"FFT 3.3\n");

				VECTORTYPE *reg2_reptr = (VECTORTYPE *)&out_re[i]; // 4 lanes reg
				VECTORTYPE *reg2_imptr = (VECTORTYPE *)&out_im[i]; // 4 lanes reg
				VECTORTYPE reg2_re = *reg2_reptr;
				VECTORTYPE reg2_im = *reg2_imptr;

//				if ((m == 0) && (mmax == 4)) fprintf(stderr,"FFT 3.4\n");

				*reg1_reptr = reg2_re - temp_re; // 4 lanes sub&store
				*reg1_imptr = reg2_im - temp_im; // 4 lanes sub&store 
				*reg2_reptr = reg2_re + temp_re; // 4 lanes add&store
				*reg2_imptr = reg2_im + temp_im; // 4 lanes add&store

//				if ((m == 0) && (mmax == 4)) fprintf(stderr,"FFT 3.4\n");
	    	}
	   	 	VECTORTYPE w_Xtmp;
	   	 	w_Xtmp = w_Xvec * wphase_Xvec - w_Yvec * wphase_Yvec; // 4 lanes rotate
	   	 	w_Yvec = w_Xvec * wphase_Yvec + w_Yvec * wphase_Xvec; // 4 lanes rotate
	   	 	w_Xvec = w_Xtmp;
		}
		mmax=istep;
    }
//	fprintf(stderr,"FFT 4\n");

	if (inverse)
	{
		float	N = 1.0 / (float) n;
		float	MN  = -N;
    	for (int i=0; i < (1<<log2point); i++)
		{
			out_re[i] *= N;
			out_im[i] *= MN;
		}
	}
//	fprintf(stderr,"FFT 5\n");
    for (int i=0; i < (1<<log2point); i++)
	{
		o_re[i] = out_re[i];
		o_im[i] = out_im[i];
	}
#endif
	return 0;
}

int	vfft_cleanup(void *context)
{
	return 0;
}

int	vfft_delete(void *context)
{
	return 0;
}


#else // no hardware vectorization

primitive_catalog_t	VFFT_catalog =
{
	.name =			"VFFT",
	.self_size =	sizeof(vfft_self_t),
	.init =			&vfft_init,
	.fire =			&vfft_fire,
	.cleanup =		&vfft_cleanup,
	.load =			&vfft_load,
	.delete =		&vfft_delete
};

struct _sample {
    float i;
    float q;
};

static struct _sample phasevec[32];
static int 		phasevec_exist = 0;


int	vfft_load(void *context)
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

int	vfft_init(void *context)
{
	return 0;
}

int	vfft_fire(void *context)
{
#ifndef _BENCHMARK_
	vfft_context_t	*ctx = (vfft_context_t *) context;
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
#endif // _BENCHMARK
	return 0;
}

int	vfft_cleanup(void *context)
{
	return 0;
}

int	vfft_delete(void *context)
{
	return 0;
}

#endif // no hardvare vectorization


