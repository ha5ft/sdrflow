/*******************************************************************************
 * 							Primitive A10
 * *****************************************************************************
 * 	Filename:		A10.c
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
#include	"A10.h"


#define	A10CTX	((A10_t *) ctx)

primitive_catalog_t	A10_catalog =
{
	.name =			"A10",
	.self_size =	sizeof(A10_self_t),
	.init =			&A10_init,
	.fire =			&A10_fire,
	.cleanup =		&A10_cleanup,
	.load =			&A10_load,
	.delete =		&A10_delete
};

int	A10_init(void *ctx)
{
	A10_self_t	*self = ((A10_t *) ctx)->self;

	self->fire_count = 0;
	fprintf(stderr, "A10: %s initialized\r\n", self->instance_name);
	return 0;
}

int	A10_fire(void *ctx)
{
#ifndef _BENCHMARK_
	A10_self_t	*self = ((A10_t *) ctx)->self;

	self->fire_count++;
	fprintf(stderr, "A10: %s fired fire_count=%d\r\n", self->instance_name, self->fire_count);
#endif
	return 0;
}

int A10_cleanup(void *ctx)
{
	A10_self_t	*self = ((A10_t *) ctx)->self;

	fprintf(stderr,"A10: %s cleaned up\r\n", self->instance_name);
	return 0;
}

int	A10_load(void *ctx)
{
	fprintf(stderr, "A10: loaded\r\n");
	return 0;
}

int	A10_delete(void *ctx)
{
	fprintf(stderr, "A10: deleted\r\n");
	return 0;
}

