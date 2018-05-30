/*******************************************************************************
 * 							Primitive VFFT
 * *****************************************************************************
 * 	Filename:		VFFT.h
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

#ifndef	VFFT_PRIMITIVE_H
#define	VFFT_PRIMITIVE_H

#include	"../../include/primitive_interface.h"

struct _vfft_self
{
	char	*instance_name;
}__attribute__((packed));

typedef struct _vfft_self	vfft_self_t;

struct _vfft_context
{
	vfft_self_t	*self;
    float   	*inp_i;
    float   	*inp_q;
    float   	*out_i;
    float   	*out_q;
    int     	*log2point;
	int			*inverse;
}__attribute__((packed));

typedef struct _vfft_context	vfft_context_t;

int	vfft_init(void *context);
int	vfft_fire(void *context);
int	vfft_cleanup(void *context);
int	vfft_load(void *context);
int	vfft_delete(void *context);

#endif


