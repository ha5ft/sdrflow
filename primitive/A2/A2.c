/*******************************************************************************
 * 							Primitive A2
 * *****************************************************************************
 * 	Filename:		A2.c
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
#include	"../../system/common/obj_format.h"
#include	"../../include/primitive_interface.h"
#include	"A2.h"


#define	A2CTX	((A2_t *) ctx)

primitive_catalog_t	A2_catalog =
{
	.name =			"A2",
	.self_size =	sizeof(A2_self_t),
	.init =			&A2_init,
	.fire =			&A2_fire,
	.cleanup =		&A2_cleanup,
	.load =			&A2_load,
	.delete =		&A2_delete
};

int	A2_init(void *ctx)
{
	A2_self_t	*self = ((A2_t *) ctx)->self;

	self->fire_count = 0;
	fprintf(stderr, "A2: %s initialized\r\n", self->instance_name);
	return 0;
}

int	A2_fire(void *ctx)
{
#ifndef _BENCHMARK_
	A2_self_t	*self = ((A2_t *) ctx)->self;

	self->fire_count++;
	fprintf(stderr, "A2: %s fired fire_count=%d\r\n", self->instance_name, self->fire_count);
#endif
	return 0;
}

int A2_cleanup(void *ctx)
{
	A2_self_t	*self = ((A2_t *) ctx)->self;

	fprintf(stderr,"A2: %s cleaned up\r\n", self->instance_name);
	return 0;
}

int	A2_load(void *ctx)
{
	fprintf(stderr, "A2: loaded\r\n");
	return 0;
}

int	A2_delete(void *ctx)
{
	fprintf(stderr, "A2: deleted\r\n");
	return 0;
}


