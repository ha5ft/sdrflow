/*******************************************************************************
 * 							Common sentence
 * *****************************************************************************
 * 	Filename:		sentence.h
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

#ifndef	SENTENCE_H
#define	SENTENCE_H

void	sentence_init(void);
void	*sentence_new_set(void);
void	*sentence_new(void *set, int lineno);
int		sentence_add_word(void *sent, void *word);
void	*sentence_get_first_word(void *set, void **iter);
void	*sentence_get_next_word(void *set, void **iter);
int		sentence_set_size(void *sent, int size);
int		sentence_get_lineno(void *sent);
int		sentence_get_size(void *sent);
int 	sentence_delete(void *set, void *sent);
int		sentence_delete_set(void *set);
int 	sentence_cleanup(void);

#endif
