/*******************************************************************************
 * 							Common scanner
 * *****************************************************************************
 * 	Filename:		scanner.h
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

#ifndef	SCANNER_H
#define	SCANNER_H

#include	"common.h"
#include	"words.h"
#include	"sdferr.h"

#define SCANNER_ERROR_NO_ERROR	0
#define	SCANNER_ERROR_EOL		1
#define	SCANNER_ERROR_EOF		2
#define SCANNER_ERROR_READ		25
#define	SCANNER_LINE_LENGTH		COMMON_LINE_LENGTH

int		scanner_init(void);
void	*scanner_new(void);
sdferr_t	*scanner_error(void *scan);
word_t	*scanner_scan(void *scan, void *inp, void *hash);
int		scanner_get_line(void *scan, char *line, int len);
void	scanner_push_back_word(void *scan, word_t *w);

//word_t	*scanner_dup_word(void *scan, word_t *word);
//word_t	*scanner_get_label(void *scan, unsigned int label);
//word_t	*scanner_get_alloc(void *scan, unsigned int alloc);
//word_t	*scanner_get_segdef(void *scan, unsigned int segdef);
//word_t	*scanner_get_new_word(void * scan);
//word_t	*scanner_lookup(void *scan, char *name);
int		scanner_lineno(void *scan);
int		scanner_delete(void *scan);
int		scanner_cleanup(void);
void	scanner_print_hash(void *scan);

#endif
