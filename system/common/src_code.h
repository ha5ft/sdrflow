/*******************************************************************************
 * 							Common src_code
 * *****************************************************************************
 * 	Filename:		src_code.h
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

#ifndef	SRC_CODE_H
#define	SRC_CODE_H

#include	"words.h"

#ifdef _SRC_CODE_

typedef struct	_src_code
{
	char	name[WORD_NAME_LENGTH + 1];
	void	*components;
	void	*implementation;
	void	*context;
	void	*streams;
	void	*variables;
	void	*constants;
	void	*actors;
	void	*schedule;
	void	*inp;
	int		subgraph_count;
	int		isolated_count;
	int		schedule_depth;
	int		initialized_for_fireing;
	int		state;
	int		depth;
	int		buffer_type;
} src_code_t;

#endif

int		src_code_initialize(void);
void	*src_code_new(char *name, void *inp, int buffer_type);
int		src_code_parse(void *src);
int		src_code_delete(void *src);
int		src_code_cleanup(void);
//int		src_code_generate_schedule(void *src, int sub);
int		src_code_generate_asm(void *src_code, void *out);
int		src_code_generate_context(void *src_code, void *out);
int		src_code_print(void *src, void *out);
#endif
