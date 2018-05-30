/*******************************************************************************
 * 							Primitive A8
 * *****************************************************************************
 * 	Filename:		A8.c
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
#include	"A8.h"


#define	A8CTX	((A8_t *) ctx)

primitive_catalog_t	A8_catalog =
{
	.name =			"A8",
	.self_size =	sizeof(A8_self_t),
	.init =			&A8_init,
	.fire =			&A8_fire,
	.cleanup =		&A8_cleanup,
	.load =			&A8_load,
	.delete =		&A8_delete
};

int	A8_init(void *ctx)
{
	A8_self_t	*self = ((A8_t *) ctx)->self;

	self->fire_count = 0;
	fprintf(stderr, "A8: %s initialized\r\n", self->instance_name);
	return 0;
}

int	A8_fire(void *ctx)
{
#ifndef _BENCHMARK_
	A8_self_t	*self = ((A8_t *) ctx)->self;

	self->fire_count++;
	fprintf(stderr, "A8: %s fired fire_count=%d\r\n", self->instance_name, self->fire_count);
#endif
	return 0;
}

int A8_cleanup(void *ctx)
{
	A8_self_t	*self = ((A8_t *) ctx)->self;

	fprintf(stderr,"A8: %s cleaned up\r\n", self->instance_name);
	return 0;
}

int	A8_load(void *ctx)
{
	fprintf(stderr, "A8: loaded\r\n");
	return 0;
}

int	A8_delete(void *ctx)
{
	fprintf(stderr, "A8: deleted\r\n");
	return 0;
}

