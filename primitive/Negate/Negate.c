/*******************************************************************************
 * 							Primitive MultiplierExt
 * *****************************************************************************
 * 	Filename:		MultiplierExt.c
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
#include	"../../include/primitive_interface.h"
#include	"Negate.h"

primitive_catalog_t	Negate_catalog =
{
	.name =			"Negate",
	.self_size =	sizeof(negate_self_t),
	.init =			&negate_init,
	.fire =			&negate_fire,
	.cleanup =		&negate_cleanup,
	.load =			&negate_load,
	.delete =		&negate_delete
};

int	negate_load(void *context)
{
	return 0;
}

int	negate_init(void *context)
{
	return 0;
}

int	negate_fire(void *context)
{
#ifndef _BENCHMARK_
	*(((negate_context_t *)context)->out) = - (*(((negate_context_t *)context)->inp));
#endif
	return 0;
}

int	negate_cleanup(void *context)
{
	return 0;
}

int	negate_delete(void *context)
{
	return 0;
}

