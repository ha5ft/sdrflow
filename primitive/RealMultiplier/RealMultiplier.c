/*******************************************************************************
 * 							Primitive RealMultiplier
 * *****************************************************************************
 * 	Filename:		RealMultiplier.c
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
#include	"RealMultiplier.h"

primitive_catalog_t	RealMultiplier_catalog =
{
	.name =			"RealMultiplier",
	.self_size =	sizeof(realmultiplier_self_t),
	.init =			&realmultiplier_init,
	.fire =			&realmultiplier_fire,
	.cleanup =		&realmultiplier_cleanup,
	.load =			&realmultiplier_load,
	.delete =		&realmultiplier_delete
};

int	realmultiplier_load(void *context)
{
	return 0;
}

int	realmultiplier_init(void *context)
{
	return 0;
}

int	realmultiplier_fire(void *context)
{
#ifndef _BENCHMARK_
	float	*a = ((realmultiplier_context_t *)context)->a;
	float	*b = ((realmultiplier_context_t *)context)->b;
	float	*axb = ((realmultiplier_context_t *)context)->axb;
	int		i = 0;

	for (i = 0; i < VECTOR_SIZE; i++)
	{
		axb[i] = a[i] * b[i];
	}
#endif
	return 0;
}

int	realmultiplier_cleanup(void *context)
{
	return 0;
}

int	realmultiplier_delete(void *context)
{
	return 0;
}

