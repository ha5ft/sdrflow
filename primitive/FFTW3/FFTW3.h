/*******************************************************************************
 * 							Primitive FFTW3
 * *****************************************************************************
 * 	Filename:		Blackhole.h
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

#ifndef	__PRIMITIVE__H
#define	__PRIMITIVE__H

#include	"../../include/primitive_interface.h"
#include	<complex.h>
#include	<fftw3.h>


struct	_FFTW3_self
{
	char			*instance_name;
	double complex	*fft_inp;
	double complex	*fft_out;
	fftw_plan		plan;
};

typedef	struct _FFTW3_self	FFTW3_self_t;

struct	_FFTW3_context
{
	FFTW3_self_t	*const self;
    float   		*inp_i;
    float   		*inp_q;
    float   		*out_i;
    float   		*out_q;
    int     		*log2point;
	int				*inverse;
}__attribute__((packed));

typedef struct _FFTW3_context	FFTW3_context_t;

int	FFTW3_load(void *context);
int	FFTW3_init(void *context);
int	FFTW3_fire(void *context);
int	FFTW3_cleanup(void *context);
int	FFTW3_delete(void *context);

#endif


