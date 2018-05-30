/*******************************************************************************
 * 							Common asm_lang
 * *****************************************************************************
 * 	Filename:		asm_lang.h
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
#ifndef ASM_LANG_H
#define ASM_LANG_H

/************************** Public functions ********************************/

int asm_lang_init_hash_table(void *hash);

int	asm_lang_compile_sentence(void *buf, int len, void *sent);
int	asm_lang_compile_word(void *buf, int len, void *word);

int	asm_lang_instr_size(unsigned int instr);
int asm_lang_instr_optype(unsigned int instr, unsigned int opnr);
int asm_lang_instr_opcount(unsigned int instr);
int asm_lang_alloc_optype(unsigned int alloc);
char *asm_lang_alloc_name(unsigned int alloc);

char *asm_lang_label_name(unsigned int label);

char *asm_lang_segdef_name(unsigned int segdef);

int asm_lang_datasize(unsigned int data_type);
int asm_lang_datamin(unsigned int data_type);
int asm_lang_datamax(unsigned int data_type);

int	asm_lang_is_compatible(unsigned int type, unsigned int value_type);

#endif
