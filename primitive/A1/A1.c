/*******************************************************************************
 * 							Primitive A1
 * *****************************************************************************
 * 	Filename:		A1.c
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
#include	"A1.h"


#define	A1CTX	((A1_t *) ctx)

primitive_catalog_t	A1_catalog =
{
	.name =			"A1",
	.self_size =	sizeof(A1_self_t),
	.init =			&A1_init,
	.fire =			&A1_fire,
	.cleanup =		&A1_cleanup,
	.load =			&A1_load,
	.delete =		&A1_delete
};

int	A1_init(void *ctx)
{
	A1_self_t	*self = ((A1_t *) ctx)->self;

	self->fire_count = 0;
	fprintf(stderr, "A1: %s initialized\r\n", self->instance_name);
	return 0;
}

int	A1_fire(void *ctx)
{
#ifndef _BENCHMARK_
	A1_self_t	*self = ((A1_t *) ctx)->self;
	
	self->fire_count++;
	fprintf(stderr, "A1: %s fired fire_count=%d\r\n", self->instance_name, self->fire_count);
#endif
	return 0;
}

int A1_cleanup(void *ctx)
{
	A1_self_t	*self = ((A1_t *) ctx)->self;
	
	fprintf(stderr,"A1: %s Cleaned up\r\n", self->instance_name);
	return 0;
}

int	A1_load(void *ctx)
{
	fprintf(stderr, "A1: loaded\r\n");
	return 0;
}

int	A1_delete(void *ctx)
{
	fprintf(stderr, "A1: deleted\r\n");
	return 0;
}


