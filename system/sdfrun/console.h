/*******************************************************************************
 * 							sdrflow runtime consoée
 * *****************************************************************************
 * 	Filename:		console.h
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


#ifndef _CONSOLE_H
#define	_CONSOLE_H

	
void	console_init();
int		console_get_available();
int 	console_set_batchmode(char *cmdfile_name);
int 	console_kbhit();
int		console_getc(char *ch);
int 	console_read_command_line(char *line, int length);
void	console_putc(char ch);
void	console_putstr(char *str);
void	console_write_line(char *buff);
void	console_cleanup();
int		console_run();

#endif	/* _CONSOLE_H */

