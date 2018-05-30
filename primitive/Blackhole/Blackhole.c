/*******************************************************************************
 * 							Primitive blackhole
 * *****************************************************************************
 * 	Filename:		Blackhole.c
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
#include	"Blackhole.h"

primitive_catalog_t	Blackhole_catalog =
{
	.name =			"Blackhole",
	.self_size =	sizeof(blackhole_self_t),
	.init =			&blackhole_init,
	.fire =			&blackhole_fire,
	.cleanup =		&blackhole_cleanup,
	.load =			&blackhole_load,
	.delete =		&blackhole_delete
};

int	blackhole_load(void *ctx)
{
	return 0;
}

int	blackhole_init(void *ctx)
{
	return 0;
}

int	blackhole_fire(void *context)
{
#ifndef _BENCHMARK_
#endif
	return 0;
}

int	blackhole_cleanup(void *context)
{
	return 0;
}

int	blackhole_delete(void *context)
{
	return 0;
}

