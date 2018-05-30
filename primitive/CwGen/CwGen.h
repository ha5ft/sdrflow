/*******************************************************************************
 * 							Primitive CwGen
 * *****************************************************************************
 * 	Filename:		CwGen.h
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
 *	Selmeczi János		23-04-2018	Original version
 *
 ******************************************************************************/

#ifndef	CWGEN_PRIMITIVE_H
#define	CWGEN_PRIMITIVE_H

#include	"../../include/primitive_interface.h"

#define	VECTOR_SIZE	1024

struct _cwgen_self
{
	char	*instance_name;
	int		text_idx;
	int		state_idx;
	int 	sample_idx;
	int		up_down_length;
	int		dot_length;
	int		dash_length;
	int		space_length;
	int		char_space_length;
	int		word_space_length;
	int		current_code;
	int		state;
	double	phase;
	double	phase_increment;
};

typedef struct _cwgen_self	cwgen_self_t;

struct	_cwgen_context
{
	cwgen_self_t	*self;
	float			*out_re;
	float			*out_im;
	char			*text;
};

typedef	struct	_cwgen_context	cwgen_context_t;


int	cwgen_load(void *context);
int	cwgen_init(void *context);
int	cwgen_fire(void *context);
int	cwgen_cleanup(void *context);
int	cwgen_delete(void *context);

#endif


