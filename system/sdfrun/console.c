/*******************************************************************************
 * 							sdrflow runtime console
 * *****************************************************************************
 * 	Filename:		console.c
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


#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <termios.h>
#include <sys/signal.h>
#include <sys/select.h>
// #include <stropts.h>
#include <sys/ioctl.h>

#include "command.h"
#include "console.h"

#define	COMMAND_LINE_LENGTH	256

static void _console_putc(char c);
static char _console_getc();

static bool initialized = false;
static bool batchmode = false;
static int	batch_fd = -1;
static int	ignore_cr = 0;
static int	ignore_lf = 0;

static struct termios	oldtio, newtio;

static char	command_line[COMMAND_LINE_LENGTH];

//************************* Main interface functions **************************


void console_init()
{
    if (initialized) return;
    initialized = true;
    
    tcgetattr(0,&oldtio);
    memcpy(&newtio, &oldtio, sizeof(newtio));
	cfmakeraw(&newtio);
    newtio.c_lflag &= ~ICANON;
    newtio.c_lflag &= ~ECHO;
    newtio.c_oflag &= ~OPOST;
    tcsetattr(0,TCSANOW,&newtio);
	setbuf(stdin, NULL);
//	console_putstr("Hello\r\n");

	ignore_cr = 0;
	ignore_lf = 0;

    return;    
}

int console_set_batchmode(char *cmdfile_name)
{
	int		ret = 0;
	
	batch_fd = open(cmdfile_name, O_RDONLY);
	if (batch_fd >= 0)
	{
		batchmode = true;
		ret = 0;
	}
	else
	{
		ret = 1;
	}
	return ret;
}

int console_run()
{
	void 	*command;
	int		ret = 0;
	int		end = 0;

	if ((command = command_new()) == NULL)
	{
		fprintf(stderr, "console_run(): Can not create command line\n");
	}
	else
	{
		while (end == 0)
		{
			ret = console_read_command_line(&command_line[0], COMMAND_LINE_LENGTH);
			if (ret == 0)
			{
				if (command_line[0] != 0)
				{
					ret = command_exec_cmdline(command, &command_line[0], COMMAND_LINE_LENGTH);
//					fprintf(stderr,"command_exec_cmdline() returned\r\n");
					if (ret == 1)
					{
//						fprintf(stderr,"Quit requested\r\n");
						end = -1;
					}
					else
					{
						console_write_line(command_line);
					}
				}
			}
			else
			{
				if (ret == 1)
				{
//					fprintf(stderr,"Exit requested\r\n");
					end = -1;
				}
				else
				{				
					fprintf(stderr, "Console read error\r\n");
				}
			}
		}
	}
	return ret;
}

void console_cleanup()
{
//    console_putstr("\r\nGoodby\r\n");
    if (initialized)
	{
		tcsetattr(0,TCSANOW,&oldtio);
		if (batchmode)
		{
			close(batch_fd);
		}
	}
    initialized = false;
    return;
}


//****************************** Utility functions ****************************

int console_read_command_line(char *line, int length)
{
	int		i=0;
	int		j=0;
	char	ch;
	int		ret = 0;
	int		state = 1;

	console_putstr("sdrflow>");
	do
	{
		ch = _console_getc();
		switch (ch)
		{
			case '\r':
				if (ignore_cr == 1)
				{
					ignore_cr = 0;
					break;
				}
				else
				{
					ignore_lf = 1;
					line[i] = 0;
					state = 0;
					break;
				}
				break;
			case '\n':
				if (ignore_lf == 1)
				{
					ignore_cr = 0;
					break;
				}
				else
				{
					ignore_cr = 1;
					line[i] = 0;
					state = 0;
					break;
				}
				break;
			default:
				ignore_cr = ignore_lf = 0;
				if ((ch == 'x') && (i == 0))
				{
					ret = 1;
					state = 0;
					_console_putc('x');
				}
				else
				{
					if (ch == 127)
					{
						if (j > 0)
						{
							if (j <= (length -1))
							{
								line[--i] = 0;
							}
							j--;
							_console_putc('\b');
							_console_putc(' ');
							_console_putc('\b');
						}
					}
					else
					{
						if(i < (length -1))
						{
							line[i++] = ch;
						}
						j++;
						_console_putc(ch);
					}
				}
				break;
		}
	}
	while (state);
	_console_putc('\r');
	_console_putc('\n');
	return ret;
}

void console_write_line(char *buff)
{
	int i = 0;
	while ((buff[i] != 0) && (i < COMMAND_LINE_LENGTH))
	{
		_console_putc(buff[i++]);
	}
	_console_putc('\r');
	_console_putc('\n');
    return;
}


int	console_get_available()
{
	int	ret = 0;
	
	if (batchmode)
	{
		ret = 1;
	}
	else
	{
		if (console_kbhit())
		{
			ret = 1;
		}
	}
	return ret;
}

int console_kbhit()
{
	int	bytes_waiting;
	
//	struct timeval	tv = {0L, 0L};
//	fd_set			fds;
//	FD_ZERO(&fds);
//	FD_SET(0, &fds);
//	return select(1, &fds, NULL, NULL, &tv);
	ioctl(0, FIONREAD, &bytes_waiting);
	return bytes_waiting;
}

int	console_getc(char *ch)
{
	*ch = _console_getc();
	if (*ch == 0)
	{
		return 0;
	}
	return 1;
}

void	console_putc(char ch)
{

	_console_putc(ch);

	return;
}


void console_putstr(char *str)
{

	while(*str)
	{
		_console_putc(*str++);
	}
	return;
}

void console_debug_putc(char ch)
{
	int	err __attribute__((unused));

	err = write(2,&ch,1);

	return;
}

// ************************** Internal functions ******************************

static void _console_putc(char c)
{
	int	err __attribute__((unused));

	err = write(1,&c,1);

	return;
}

static char _console_getc()
{
	char	ch;
	int		end = 0;

	while (end == 0)
	{
		if (batchmode)
		{
			if (read(batch_fd, &ch, 1) != 1)
				ch = 0;
		}
		else
		{
			if (read(0,&ch,1) != 1)
				ch = 0;
		}
		if (	((ch >= '0') && (ch <= '9')) ||
				((ch >= 'A') && (ch <= 'Z')) ||
				((ch >= 'a') && (ch <= 'z')) ||
				(ch == '.') ||
				(ch == '+') ||
				(ch == '-') ||
				(ch == '/') ||
				(ch == 127) ||
				(ch == '\r') ||
				(ch == '\n') ||
				(ch == ' ') ||
				(ch == '"')
			) end = -1;

	}
	return ch;
}



