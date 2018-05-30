/*******************************************************************************
 * 							Primitive RealMultiplier
 * *****************************************************************************
 * 	Filename:		RealMultiplier.h
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
 *	Selmeczi János		23-04-2018	Original version
 *
 ******************************************************************************/

#ifndef	REALMULTIPLIER_PRIMITIVE_H
#define	REALMULTIPLIER_PRIMITIVE_H

#include	"../../include/primitive_interface.h"

#define	VECTOR_SIZE	1024

struct	_realmultiplier_self
{
	char	*instance_name;
};

typedef	struct _realmultiplier_self	realmultiplier_self_t;

struct	_realmultiplier_context
{
	realmultiplier_self_t	*self;
	float					*a;
	float					*b;
	float					*axb;
}__attribute__((packed));

typedef struct _realmultiplier_context	realmultiplier_context_t;

int	realmultiplier_load(void *context);
int	realmultiplier_init(void *context);
int	realmultiplier_fire(void *context);
int	realmultiplier_cleanup(void *context);
int	realmultiplier_delete(void *context);

#endif


