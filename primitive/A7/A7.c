/*******************************************************************************
 * 							Primitive A7
 * *****************************************************************************
 * 	Filename:		A7.c
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
#include	"A7.h"


#define	A7CTX	((A7_t *) ctx)

primitive_catalog_t	A7_catalog =
{
	.name =			"A7",
	.self_size =	sizeof(A7_self_t),
	.init =			&A7_init,
	.fire =			&A7_fire,
	.cleanup =		&A7_cleanup,
	.load =			&A7_load,
	.delete =		&A7_delete
};

int	A7_init(void *ctx)
{
	A7_self_t	*self = ((A7_t *) ctx)->self;

	self->fire_count = 0;
	fprintf(stderr, "A7: %s initialized\r\n", self->instance_name);
	return 0;
}

int	A7_fire(void *ctx)
{
#ifndef _BENCHMARK_
	A7_self_t	*self = ((A7_t *) ctx)->self;

	self->fire_count++;
	fprintf(stderr, "A7: %s fired fire_count=%d\r\n", self->instance_name, self->fire_count);
#endif
	return 0;
}

int A7_cleanup(void *ctx)
{
	A7_self_t	*self = ((A7_t *) ctx)->self;

	fprintf(stderr,"A7: %s cleaned up\r\n", self->instance_name);
	return 0;
}

int	A7_load(void *ctx)
{
	fprintf(stderr, "A7: loaded\r\n");
	return 0;
}

int	A7_delete(void *ctx)
{
	fprintf(stderr, "A7: deleted\r\n");
	return 0;
}

