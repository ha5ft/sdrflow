/*******************************************************************************
 * 							Primitive Printer2
 * *****************************************************************************
 * 	Filename:		Printer2.c
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
#include	"Printer2.h"

primitive_catalog_t	Printer2_catalog =
{
	.name =			"Printer2",
	.self_size =	sizeof(printer2_self_t),
	.init =			&printer2_init,
	.fire =			&printer2_fire,
	.cleanup =		&printer2_cleanup,
	.load =			&printer2_load,
	.delete =		&printer2_delete
};

int	printer2_load(void *context)
{
	return 0;
}

int	printer2_init(void *context)
{
	return 0;
}

int	printer2_fire(void *context)
{
#ifndef _BENCHMARK_
	int	i;
	float	*inp_re = ((printer2_context_t *) context)->inp_re;
	float	*inp_im = ((printer2_context_t *) context)->inp_im;

	fprintf(stderr,"\r\n\r\n");
	for (i = 0; i < VECTOR_SIZE; i++)
	{
		fprintf(stderr,"%d\t%2.9f\t%2.9f\r\n", i, inp_re[i], inp_im[i]);
	} 
	fprintf(stderr,"\n\n");
#endif
	return 0;
}

int	printer2_cleanup(void *context)
{
	return 0;
}

int	printer2_delete(void *context)
{
	return 0;
}

