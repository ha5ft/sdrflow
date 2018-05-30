/*******************************************************************************
 * 							Primitive A5
 * *****************************************************************************
 * 	Filename:		A5.c
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
#include	"A5.h"


#define	A5CTX	((A5_t *) ctx)

primitive_catalog_t	A5_catalog =
{
	.name =			"A5",
	.self_size =	sizeof(A5_self_t),
	.init =			&A5_init,
	.fire =			&A5_fire,
	.cleanup =		&A5_cleanup,
	.load =			&A5_load,
	.delete =		&A5_delete
};

int	A5_init(void *ctx)
{
	A5_self_t	*self = ((A5_t *) ctx)->self;

	self->fire_count = 0;
	fprintf(stderr, "A5: %s initialized\r\n", self->instance_name);
	return 0;
}

int	A5_fire(void *ctx)
{
#ifndef _BENCHMARK_
	A5_self_t	*self = ((A5_t *) ctx)->self;

	self->fire_count++;
	fprintf(stderr, "A5: %s fired fire_count=%d\r\n", self->instance_name, self->fire_count);
#endif
	return 0;
}

int A5_cleanup(void *ctx)
{
	A5_self_t	*self = ((A5_t *) ctx)->self;

	fprintf(stderr,"A5: %s cleaned up\r\n", self->instance_name);
	return 0;
}

int	A5_load(void *ctx)
{
	fprintf(stderr, "A5: loaded\r\n");
	return 0;
}

int	A5_delete(void *ctx)
{
	fprintf(stderr, "A5: deleted\r\n");
	return 0;
}


