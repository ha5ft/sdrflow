/*******************************************************************************
 * 							Common pointer
 * *****************************************************************************
 * 	Filename:		pointer.h
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

#ifndef	POINTER_H
#define	POINTER_H

#ifdef	_POINTER_

typedef struct	_pointer
{
	void	*ptr;
} pointer_t;

#endif

int		pointer_initialize(void);
void	*pointer_new_set(void);
void	*pointer_dup_set(void *set);
void	*pointer_new(void *set, void *ptr);
void	*pointer_dup(void *ptr, void *dup_set);
void	*pointer_get_raw(void *ptr);
void	**pointer_get_raw_pointers(void *set);
int		pointer_delete(void *set, void *ptr);
int		pointer_delete_set(void *set);
int		pointer_cleanup(void);

#endif
