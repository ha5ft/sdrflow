/*******************************************************************************
 * 							Primitive ?actor?
 * *****************************************************************************
 * 	Filename:		?actor?.c
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

#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	"../../include/primitive_interface.h"
#include	"?actor?.h"

primitive_catalog_t	?actor?_catalog =
{
	.name =			"?actor?",
	.self_size =	sizeof(?actor?_self_t),
	.init =			&?actor?_init,
	.fire =			&?actor?_fire,
	.cleanup =		&?actor?_cleanup,
	.load =			&?actor?_load,
	.delete =		&?actor?_delete
};

int	?actor?_load(void *sys_catalog)
{
// You should put your own code here
	return 0;
}

int	?actor?_init(void *context)
{
// You should put your own code here
	return 0;
}

int	?actor?_fire(void *context)
{
// You should put your own code here
	return 0;
}

int	?actor?_cleanup(void *context)
{
// You should put your own code here
	return 0;
}

int	?actor?_delete(void *sys_catalog)
{
// You should put your own code here
	return 0;
}

