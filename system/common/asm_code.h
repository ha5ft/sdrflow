/*******************************************************************************
 * 							Common asmcode
 * *****************************************************************************
 * 	Filename:		asm_code.h
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

#ifndef	ASM_CODE_H
#define	ASM_CODE_H

#include "sdferr.h"

int		asm_code_init(void);
void	*asm_code_new(void);
//void 	*asm_code_parse(void *asm_code, void *inp);
sdferr_t	*asm_code_get_error(void *asm_code);
int		asm_code_delete(void *asm_code);
int		asm_code_cleanup(void);
void 	asm_code_print(void *asm_code, void * out);
int		asm_code_analyse(void *asm_code, void *inp);
void 	asm_code_list(void *asm_code, void *out);
void 	*asm_code_generate(void *asm_code);
int		asm_code_get_composite_size(void *asm_code);
int 	asm_code_set_sizes(void *asm_code);
int		asm_code_build_prolog(void *asm_code);

#endif
