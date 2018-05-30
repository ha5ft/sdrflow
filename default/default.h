/*******************************************************************************
 * 							Primitive ?actor?
 * *****************************************************************************
 * 	Filename:		Blackhole.h
 * 	Platform:		ubuntu 16.04 64 bit
 * 	Author:			
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
 *  along with sdrflow.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 *							Revision history
 *******************************************************************************
 *	Author				Date		Comment
 *******************************************************************************
 *	
 *
 ******************************************************************************/

#ifndef	__PRIMITIVE__H
#define	__PRIMITIVE__H

#include	"../../include/primitive_interface.h"


struct	_?actor?_self
{
	char	*instance_name;
// You should put your definitions here
};

typedef	struct _?actor?_self	?actor?_self_t;

struct	_?actor?_context
{
	?actor?_self_t	*const self;
// You should put your definitions here
}__attribute__((packed));

typedef struct _?actor?_context	?actor?_context_t;

int	?actor?_load(void *context);
int	?actor?_init(void *context);
int	?actor?_fire(void *context);
int	?actor?_cleanup(void *context);
int	?actor?_delete(void *context);

#endif


