/*******************************************************************************
 * 							sdrflow runtime command
 * *****************************************************************************
 * 	Filename:		command.c
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


#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	"../common/objects.h"
#include	"../common/pointer.h"
#include	"../common/program.h"
#include	"command.h"
#include	"console.h"

#define	CMD_NOP		0
#define	CMD_LOAD	1
#define	CMD_RUN		2
#define	CMD_UNLOAD	3
#define	CMD_START	4
#define	CMD_KILL	5
#define	CMD_IGET	6
#define CMD_LGET	7
#define	CMD_FGET	8
#define	CMD_DGET	9
#define	CMD_SGET	10
#define	CMD_ISET	11
#define	CMD_LSET	12
#define	CMD_FSET	13
#define	CMD_DSET	14
#define	CMD_SSET	15
#define	CMD_EXIT	16
#define CMD_BENCHMARK	17

#define	CMD_NOP_STR		"nop"
#define	CMD_LOAD_STR	"load"
#define	CMD_RUN_STR		"run"
#define	CMD_UNLOAD_STR	"unload"
#define	CMD_START_STR	"start"
#define	CMD_KILL_STR	"kill"
#define	CMD_IGET_STR	"iget"
#define CMD_LGET_STR	"lget"
#define	CMD_FGET_STR	"fget"
#define	CMD_DGET_STR	"dget"
#define	CMD_SGET_STR	"sget"
#define	CMD_ISET_STR	"iset"
#define	CMD_LSET_STR	"lset"
#define	CMD_FSET_STR	"fset"
#define	CMD_DSET_STR	"dset"
#define	CMD_SSET_STR	"sset"
#define	CMD_EXIT_STR	"exit"
#define CMD_BENCHMARK_STR	"benchmark"

#define	CMD_COUNT	18

#define	STATE_TOKEN_START				0
#define	STATE_NUM_SIGN					1
#define STATE_NUM_DEC_POINT				2
#define	STATE_NUM_FRAC_DIGIT			3
#define STATE_NUM_EXP_START				4
#define	STATE_NUM_EXP_SIGN				5
#define STATE_NUM_EXP_DIGIT				6
#define STATE_NUM_INT_DIGIT				7
#define STATE_NUM_ZERO_DIGIT			8
#define	STATE_NUM_HEX_START				9
#define	STATE_NUM_HEX_DIGIT				10
#define STATE_PATH_START				11
#define STATE_PATH_ALPHA				12
#define STATE_PATH_UNDERSCORE_ALPHA		13
#define STATE_PATH_ALL					14
#define STATE_CMD_CHAR					15
#define STATE_STRING_START				16
#define STATE_STRING_CLOSE				17

#define	PARSER_STATE_CMD_NAME			0
#define	PARSER_STATE_CMD_PATH			1
#define	PARSER_STATE_CMD_VALUE			2

#define	TOKEN_NULL			0
#define	TOKEN_CMD_NAME		1
#define	TOKEN_PATH_NAME		2
#define	TOKEN_INTEGER		3
#define	TOKEN_REAL			4
#define	TOKEN_STRING		5
#define	TOKEN_HEX			6
#define	TOKEN_EOL			7

#define VALUE_TYPE_NULL		0
#define VALUE_TYPE_INT		1
#define VALUE_TYPE_LONG		2
#define VALUE_TYPE_FLOAT	3
#define VALUE_TYPE_DOUBLE	4
#define VALUE_TYPE_STRING	5

#define ERROR_COUNT			18
#define	MAX_PATH			32
#define EXEC_ERROR_COUNT	12

static	char	*error_description[ERROR_COUNT] =
{
	"No Error", // 0
	"Illegal character", // 3->1
	"'.' or '0'-'9' are expected", // 4->2
	"'0'-'9' are expected", // 5->3
	"'0'-'9','e','E',';',white space or EOL expected", // 6->4
	"'0'-'9','+' or '-' are expected", // 7->4
	"'0'-'9',';',white space or EOL expected", // 8->6
	"Can not allocate word structure", // 9->7
	"'0'-'9','e','E','.',';',white space or EOL expected", // 12->8
	"'0'-'9','e','E','.',';','x','X',white space or EOL expected", // 13->9
	"'0'-'9','a'-'f','A'-'F' are expected", // 10
	"'0'-'9','a'-'f','A'-'F', white space or EOL are expected", // 11
	"illegal character in path", // 14->12
	"letters,white space or EOL are expected", // 15->13
	"';', white space or EOL are expected", // 18->14
	"End of line before string end", // 19->15
	"Letters are expected", // 23->16
	"Unexpected line end" //17
};

static char	*exec_error[EXEC_ERROR_COUNT] =
{
	[0] = "OK",
	[1] = "Error: Can not load the dataflow",
	[2] = "Error: Can not find the dataflow",
	[3] = "Error: In unloading the dataflow",
	[4] = "Command not implemented yet",
	[5] = "Invalid command ID",
	[6] = "Cannot get parameter value",
	[7] = "Cannot set parameter value",
	[8] = "Cannot fire program",
	[9] = "Cannot start program",
	[10] = "Error in killing the program",
	[11] = "Cannot unload a fireing program"
};

struct	_command
{
	int		id;
	int		line_length;
	char	*line;
	int		line_pos;
	int		line_end;
	int		token_type;
	int		path_started;
	int		path_count;
	int		value_count;
	int		value_length;
	int		value_type;
	char	*token;
	char	*path[MAX_PATH];
	union	{
				int	i;
				long	l;
				float	f;
				double	d;
				char	ch[32];
			} values;
};

typedef	struct _command	command_t;


static int	_command_parse(command_t *cmd);
static int	_command_next_token(command_t *cmd);
static int	_command_exec(command_t * command);
//static int	_command_list(command_t * command);


static	void	*commands = NULL;
static	int		initialized = 0;

static char	*cmd_names[CMD_COUNT] =
{
	[CMD_NOP]		= CMD_NOP_STR,
	[CMD_LOAD]		= CMD_LOAD_STR,
	[CMD_RUN]		= CMD_RUN_STR,
	[CMD_UNLOAD]	= CMD_UNLOAD_STR,
	[CMD_START]		= CMD_START_STR,
	[CMD_KILL]		= CMD_KILL_STR,
	[CMD_IGET]		= CMD_IGET_STR,
	[CMD_LGET]		= CMD_LGET_STR,
	[CMD_FGET]		= CMD_FGET_STR,
	[CMD_DGET]		= CMD_DGET_STR,
	[CMD_SGET]		= CMD_SGET_STR,
	[CMD_ISET]		= CMD_ISET_STR,
	[CMD_LSET]		= CMD_LSET_STR,
	[CMD_FSET]		= CMD_FSET_STR,
	[CMD_DSET]		= CMD_DSET_STR,
	[CMD_SSET]		= CMD_SSET_STR,
	[CMD_EXIT]		= CMD_EXIT_STR,
	[CMD_BENCHMARK]		= CMD_BENCHMARK_STR
};

static int cmd_path_count[CMD_COUNT] =
{
	[CMD_NOP]		= 0,
	[CMD_LOAD]		= 2,
	[CMD_RUN]		= 1,
	[CMD_UNLOAD]	= 1,
	[CMD_START]		= 1,
	[CMD_KILL]		= 1,
	[CMD_IGET]		= -2,
	[CMD_LGET]		= -2,
	[CMD_FGET]		= -2,
	[CMD_DGET]		= -2,
	[CMD_SGET]		= -2,
	[CMD_ISET]		= -2,
	[CMD_LSET]		= -2,
	[CMD_FSET]		= -2,
	[CMD_DSET]		= -2,
	[CMD_SSET]		= -2,
	[CMD_EXIT]		= 0,
	[CMD_BENCHMARK]		= 1
};

static int	cmd_value_type[CMD_COUNT] =
{
	[CMD_NOP]		= VALUE_TYPE_NULL,
	[CMD_LOAD]		= VALUE_TYPE_NULL,
	[CMD_RUN]		= VALUE_TYPE_INT,
	[CMD_UNLOAD]	= VALUE_TYPE_NULL,
	[CMD_START]		= VALUE_TYPE_NULL,
	[CMD_KILL]		= VALUE_TYPE_NULL,
	[CMD_IGET]		= VALUE_TYPE_NULL,
	[CMD_LGET]		= VALUE_TYPE_NULL,
	[CMD_FGET]		= VALUE_TYPE_NULL,
	[CMD_DGET]		= VALUE_TYPE_NULL,
	[CMD_SGET]		= VALUE_TYPE_NULL,
	[CMD_ISET]		= VALUE_TYPE_INT,
	[CMD_LSET]		= VALUE_TYPE_LONG,
	[CMD_FSET]		= VALUE_TYPE_FLOAT,
	[CMD_DSET]		= VALUE_TYPE_DOUBLE,
	[CMD_SSET]		= VALUE_TYPE_STRING,
	[CMD_EXIT]		= VALUE_TYPE_NULL,
	[CMD_BENCHMARK]		= VALUE_TYPE_INT
};


int command_init()
{
	if (initialized) return 0;
	if ((commands = objects_new_set(OBJECTS_TYPE_COMMAND)) == NULL)
	{
		fprintf(stderr,"command_init(): Can not create command set\r\n");
		return -1;
	}
	initialized = -1;

	return 0;
}

void	*command_new()
{
	command_t	*cmd;
	int			i = 0;

//	fprintf(stderr,"command_new()\r\n");
	cmd = (command_t *) objects_new(commands, sizeof(command_t));
	if (cmd != NULL)
	{
		cmd->id = 0;
		cmd->line = NULL;
		cmd->line_length = 0;
		cmd->line_pos = 0;
		cmd->token_type = TOKEN_NULL;
		cmd->line_end = 0;
		cmd->path_started = 0;
		cmd->path_count = 0;
		cmd->value_count = 0;
		cmd->value_length = 0;
		cmd->value_type = VALUE_TYPE_NULL;
		for (i = 0; i < 32; i++)
		{
			cmd->values.ch[i] = 0;
		}
		for (i = 0; i < MAX_PATH; i++)
		{
			cmd->path[i] = NULL;
		}
	}
	else
	{
		fprintf(stderr,"command_new(): Can not create command objects\r\n");
	}
	return cmd;
}

int	command_exec_cmdline(void *command, char *line, int line_length)
{
	command_t	*cmd = (command_t *) command;
	int			i = 0;

//	fprintf(stderr,"command_exec_command_line()\r\n");

	cmd->line = line;
	cmd->line_length = line_length;
	cmd->id = CMD_NOP;
	cmd->line_pos = 0;
	cmd->token_type = TOKEN_NULL;
	cmd->line_end = 0;
	cmd->path_started = 0;
	cmd->path_count = 0;
	cmd->value_count = 0;
	cmd->value_length = 0;
	cmd->value_type = VALUE_TYPE_NULL;
	for (i = 0; i < 32; i++)
	{
		cmd->values.ch[i] = 0;
	}
	for (i = 0; i < MAX_PATH; i++)
	{
		cmd->path[i] = NULL;
	}

	if ( _command_parse(cmd) != 0)
	{
//		fprintf(stderr,"Error in parsing the command line\r\n");
		return -1;
	}
//	if (_command_list(cmd) != 0)
//	{
//		sprintf(cmd->line,"Error in listing");
//		return -1;
//	}
	return _command_exec(cmd);
}
/*
static int	_command_list(command_t * cmd)
{
//	int		pos = 0;
//	char	**path = NULL;
	int		i = 0;
//	int		n = 0;
	float	f;
//	char	*ps,pd;

	fprintf(stderr,"_command_exec()\r\n");
	fprintf(stderr,"cmd id:%d\r\n",cmd->id);
	fprintf(stderr,"path count:%d\r\n",cmd->path_count);
	fprintf(stderr,"value count:%d\r\n",cmd->value_count);
	if (cmd->path_count > 0)
	{
		fprintf(stderr,"path:");
		for (i = 0; i < cmd->path_count; i++)
		{
			fprintf(stderr, "/%s", cmd->path[i]);
		}
		fprintf(stderr,"\r\n");
	}
	if (cmd->value_count)
	{
		fprintf(stderr,"value:");
		switch(cmd->value_type)
		{
			case	VALUE_TYPE_INT:
				fprintf(stderr,"%d\r\n", cmd->values.i);
				break;
			case	VALUE_TYPE_LONG:
				fprintf(stderr,"%ld\r\n", cmd->values.l);
				break;
			case	VALUE_TYPE_FLOAT:
				fprintf(stderr,"%f\r\n", cmd->values.f);
				memcpy((void *) &f, (void *) cmd->values.ch, sizeof(float));
				fprintf(stderr,"copied:%f\r\n",f);
				break;
			case	VALUE_TYPE_DOUBLE:
				fprintf(stderr,"%lf\r\n", cmd->values.d);
				break;
			case	VALUE_TYPE_STRING:
				fprintf(stderr,"%s\r\n", cmd->values.ch);
				break;
		}
	}
	sprintf(cmd->line,"OK");
	return 0;
}
*/
static int	_command_exec(command_t * cmd)
{
	int		error = 0;
	void	*program = NULL;
	double	time_ms = 0;

	switch (cmd->id)
	{
		case CMD_NOP:
			break;
		case CMD_LOAD:
			if (program_new(cmd->path[0], cmd->path[1]) == NULL)
				error = 1;
			break;
		case CMD_RUN:
			if (program_fire(cmd->path[0], cmd->values.i) != 0)
				error = 8;
			break;
		case CMD_UNLOAD:
			program = program_find(cmd->path[0]);
			if (program != NULL)
			{
				if (program_get_state(program) != PROGRAM_STATE_FIREING)
				{
					if (program_delete(program) != 0)
						error = 3;
				}
				else
				{
					error = 11;
				}
			}
			else
			{
				error = 2;
			}

			break;
		case CMD_START:
			if (program_fire_in_thread(cmd->path[0]) != 0)
				error = 9;
			break;
		case CMD_BENCHMARK:
			if (program_benchmark(cmd->path[0],  cmd->values.i, &time_ms) != 0)
			{
				error = 9;
			}
			else
			{
				snprintf(cmd->line,(size_t) cmd->line_length,"cycle time[ms]=%lf", time_ms / ((double) cmd->values.i));
				error = -2;
			}
			break;
		case CMD_KILL:
			if (program_kill_fireing(cmd->path[0]) != 0)
				error = 10;
			break;
		case CMD_EXIT:
			error = -1;
			break;
		case CMD_IGET:
			if (program_get_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.i, sizeof(int)) == 0)
			{
				snprintf(cmd->line,(size_t) cmd->line_length,"value=%d", cmd->values.i);
				error = -2;
			}
			else
			{
				error = 6;
			}
			break;
		case CMD_LGET:
			if (program_get_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.l, sizeof(long)) == 0)
			{
				snprintf(cmd->line,(size_t) cmd->line_length,"value=%ld", cmd->values.l);
				error = -2;
			}
			else
			{
				error = 6;
			}
			break;
		case CMD_FGET:
			if (program_get_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.f, sizeof(float)) == 0)
			{
				snprintf(cmd->line,(size_t) cmd->line_length,"value=%f", cmd->values.f);
				error = -2;
			}
			else
			{
				error = 6;
			}
			break;
		case CMD_DGET:
			if (program_get_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.d, sizeof(double)) == 0)
			{
				snprintf(cmd->line,(size_t) cmd->line_length,"value=%lf", cmd->values.d);
				error = -2;
			}
			else
			{
				error = 6;
			}
			break;
		case CMD_SGET:
			if (program_get_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.ch[0], 32) == 0)
			{
				snprintf(cmd->line,(size_t) cmd->line_length,"value=%s", cmd->values.ch);
				error = -2;
			}
			else
			{
				error = 6;
			}
			break;
		case CMD_ISET:
			if (program_set_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.i, sizeof(int)) == 0)
			{
				error = 0;
			}
			else
			{
				error = 7;
			}
			break;
		case CMD_LSET:
			if (program_set_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.l, sizeof(long)) == 0)
			{
				error = 0;
			}
			else
			{
				error = 7;
			}
			break;
		case CMD_FSET:
			if (program_set_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.f, sizeof(float)) == 0)
			{
				error = 0;
			}
			else
			{
				error = 7;
			}
			break;
		case CMD_DSET:
			if (program_set_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.d, sizeof(double)) == 0)
			{
				error = 0;
			}
			else
			{
				error = 7;
			}
			break;
		case CMD_SSET:
			if (program_set_parameter(cmd->path, cmd->path_count, (void *) &cmd->values.ch[0], 32) == 0)
			{
				error = 0;
			}
			else
			{
				error = 7;
			}
			break;
		default:
			error = 5;
			break;
	}
	if (error >= 0)
		sprintf(cmd->line,"%s",exec_error[error]);
	return -error;
}

int command_delete(void *command)
{
//	command_t	*cmd = (command_t *) command;
	int			error = 0;

	if (initialized == 0) return -1;
	if (commands == NULL) return -1;
	if (command == NULL) return -1;
//	if (cmd->path != NULL)
//	{
//		if (pointer_delete_set(cmd->path) != 0)
//			error = -1;
//	}
	if (objects_delete(commands, command) != 0)
		error = -1;
	if (objects_delete_set(commands) != 0)
		error = -1;
	return error;
}

void command_cleanup()
{
//	command_t	*cmd = NULL;
	int			error __attribute__((unused));

	if (initialized == 0) return;
	if (commands == NULL) return;	
//	cmd = (command_t *) objects_get_first(commands);
//	while (cmd != NULL)
//	{
//		if (cmd->path != NULL)
//		{
//			error = objects_delete_set(cmd->path);
//		}
//		cmd = (command_t *) objects_get_next(commands, cmd);
//	}
	error = objects_delete_set(commands);

	return;
}

static int	_command_next_token(command_t *cmd)
{
	int		state = STATE_TOKEN_START;
	int		pos = cmd->line_pos;
	int		token_end = 0;
	int		token_start = 0;
	int		token_type = TOKEN_EOL;
	int		path_started = cmd->path_started;
	int		end = 0;
	char	ch;
	int		error = 0;

//	fprintf(stderr,"_command_next_token()\r\n");
	if (cmd->line_length == 0) return 0;
	if (path_started) state = STATE_PATH_START;
	ch = cmd->line[pos];
	while (end == 0) 
	{
		switch (state)
		{
			case	STATE_TOKEN_START:
				token_start = pos;
				if ((ch == 32) || (ch == 9)) break;

				if ((ch == '+') || (ch == '-')) {state = STATE_NUM_SIGN; break;}
				if ((ch >= '1') && (ch <= '9')) {state = STATE_NUM_INT_DIGIT; break;}
				if (ch == '0') {state = STATE_NUM_ZERO_DIGIT; break;}
				if (ch == '.') {state = STATE_NUM_DEC_POINT; break;}

				if ((ch >= 'a') && (ch <= 'z')) {state = STATE_CMD_CHAR; break;}
				if ((ch >= 'A') && (ch <= 'Z')) {state = STATE_CMD_CHAR; break;}

				if (ch == 0) {end = -1; token_type = TOKEN_EOL; break;}

				if (ch == '"') {state = STATE_STRING_START; token_start++; break;}

				if (ch == '/') {state = STATE_PATH_START; path_started = -1; break;}

				{end = -1; error = 1; break;}
				break;

// **************************** Literal number *********************************

			case	STATE_NUM_SIGN:
				if (ch == '.') {state = STATE_NUM_DEC_POINT; break;}
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_INT_DIGIT; break;}
				{end = -1; error = 2; break;}
			case	STATE_NUM_DEC_POINT:
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_FRAC_DIGIT; break;}
				{end = -1; error = 3; break;}
			case	STATE_NUM_FRAC_DIGIT:
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_FRAC_DIGIT; break;}
				if ((ch == 'e') || (ch == 'E')) {state = STATE_NUM_EXP_START; break;}
				if ((ch == 32) || (ch == 9) || (ch == 0))
					{end = -1; token_end = pos; token_type = TOKEN_REAL; break;}
				{end = -1; error = 4; break;}
			case	STATE_NUM_EXP_START:
				if ((ch == '+') || (ch == '-')) {state = STATE_NUM_EXP_SIGN; break;}
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_EXP_DIGIT; break;}
				{end = -1; error = 5; break;}
			case	STATE_NUM_EXP_SIGN:
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_EXP_DIGIT; break;}
				{end = -1; error = 3; break;}
			case	STATE_NUM_EXP_DIGIT:
				if ((ch >= '0') && (ch <= '9')) {state = 6; break;}
				if ((ch == 32) || (ch == 9) || (ch == 0))
					{end = -1; token_end = pos; token_type = TOKEN_REAL; break;}
				{end = -1; error = 6; break;}
			case	STATE_NUM_INT_DIGIT:
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_INT_DIGIT; break;}
				if ((ch == 'e') || (ch == 'E')) {state = STATE_NUM_EXP_START; break;}
				if (ch == '.') {state = STATE_NUM_DEC_POINT; break;}
				if ((ch == 32) || (ch == 9) || (ch == 0))
					{end = -1; token_end = pos; token_type = TOKEN_INTEGER; break;}
				{end = -1; error = 8; break;}
			case	STATE_NUM_ZERO_DIGIT:
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_INT_DIGIT; break;}
				if (ch == '.') {state = STATE_NUM_DEC_POINT; break;}
				if ((ch == 'e') || (ch == 'E')) {state = STATE_NUM_EXP_START; break;}
				if ((ch == 32) || (ch == 9) || (ch == 0))
					{end = -1; token_end = pos; token_type = TOKEN_INTEGER; break;}
				if ((ch == 'x') || (ch == 'X')) {state = STATE_NUM_HEX_START; break;}
				{end = -1; error = 9; break;}
			case	STATE_NUM_HEX_START:
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_HEX_DIGIT; break;}
				if ((ch >= 'a') && (ch <= 'f')) {state = STATE_NUM_HEX_DIGIT; break;}
				if ((ch >= 'A') && (ch <= 'F')) {state = STATE_NUM_HEX_DIGIT; break;}
				{end = -1; error = 10; break;}
			case	STATE_NUM_HEX_DIGIT:
				if ((ch >= '0') && (ch <= '9')) {state = STATE_NUM_HEX_DIGIT; break;}
				if ((ch >= 'a') && (ch <= 'f')) {state = STATE_NUM_HEX_DIGIT; break;}
				if ((ch >= 'A') && (ch <= 'F')) {state = STATE_NUM_HEX_DIGIT; break;}
				if ((ch == 32) || (ch == 9) || (ch == 0))
					{end = -1; token_end = pos; token_type = TOKEN_HEX; break;}
				{end = -1; error = 11; break;}

// ****************************** Command ***********************************

			case	STATE_CMD_CHAR:
				if ((ch >= 'a') && (ch <= 'z')) {state = STATE_CMD_CHAR; break;}
				if ((ch >= 'A') && (ch <= 'Z')) {state = STATE_CMD_CHAR; break;}
				if ((ch == 32) || (ch == 9) || (ch == 0))
					{end = -1; token_end = pos; token_type = TOKEN_CMD_NAME; break;}
				{end = -1; error = 13; break;}

// ****************************** Path ***********************************

			case	STATE_PATH_START:
				token_start = pos;
				if (ch == '.') {state = STATE_PATH_ALPHA; break;}
				if (ch == '_') {state = STATE_PATH_UNDERSCORE_ALPHA; break;}
				if ((ch >= 'a') && (ch <= 'z')) {state = STATE_PATH_ALL; break;}
				if ((ch >= 'A') && (ch <= 'Z')) {state = STATE_PATH_ALL; break;}
				{end = -1; error = 12; break;}
			case	STATE_PATH_ALPHA:
				if ((ch >= 'a') && (ch <= 'z')) {state = STATE_PATH_ALL; break;}
				if ((ch >= 'A') && (ch <= 'Z')) {state = STATE_PATH_ALL; break;}
				{end = -1; error = 12; break;}
			case	STATE_PATH_UNDERSCORE_ALPHA:
				if (ch == '_') {state = STATE_PATH_UNDERSCORE_ALPHA; break;}
				if ((ch >= 'a') && (ch <= 'z')) {state = STATE_PATH_ALL; break;}
				if ((ch >= 'A') && (ch <= 'Z')) {state = STATE_PATH_ALL; break;}
				{end = -1; error = 12; break;}
			case	STATE_PATH_ALL:
				if (ch == '.') {state = STATE_PATH_ALL; break;}
				if (ch == '_') {state = STATE_PATH_ALL; break;}
				if ((ch >= 'a') && (ch <= 'z')) {state = STATE_PATH_ALL; break;}
				if ((ch >= 'A') && (ch <= 'Z')) {state = STATE_PATH_ALL; break;}
				if ((ch >= '0') && (ch <= '9')) {state = STATE_PATH_ALL; break;}
				if (ch == '/') {end = -1; token_end = pos; token_type = TOKEN_PATH_NAME; break;}
				if ((ch == 32) || (ch == 9) || (ch == 0))
					{end = -1; token_end = pos; token_type = TOKEN_PATH_NAME; path_started = 0; break;}

// ************************** String literal ***********************************

			case	STATE_STRING_START:
				if (ch == '"') {state = STATE_STRING_CLOSE; break;}
				if (ch == 0) {end = -1; error = 15; break;}
				{state = STATE_STRING_START; break;}
			case	STATE_STRING_CLOSE:
				if ((ch == 32) || (ch == 9) || (ch == 0))
					{end = -1;; token_end = pos - 1; token_type = TOKEN_STRING; break;}
				{end = -1; error = 14; break;}
		}
		if (error == 0)
		{
			if (end == 0)
			{
				if (pos++ < cmd->line_length)
				{
					ch = cmd->line[pos];
				}
				else
				{
					error = 18;
					end = -1;
					break;
				}
			}
			else
			{
				if (ch == 0) cmd->line_end = -1;
				cmd->line[token_end] = 0;
				cmd->token = &cmd->line[token_start];
				if (cmd->line_end == 0)
					cmd->line_pos = token_end + 1;
				else
					cmd->line_pos = token_end;
//				if (cmd->line[cmd->line_pos] == 0) cmd->line_end = -1;
				cmd->token_type = token_type;
				cmd->path_started = path_started;
				break;
			}
		}
	}

	if (error)
	{
		snprintf
				(
					cmd->line,
					(size_t) cmd->line_length,
					"Error in tokenizing command line: in position %d : %s",
					pos,
					error_description[error]
				);
	}

	return error;
}

static int	_command_parse(command_t *cmd)
{
	int	state = PARSER_STATE_CMD_NAME;
	int	ret = 0;
	int	error = 0;
	int	end = 0;
	int	no_new_token = 0;
	int	pc = 0;
	int	i = 0;

//	fprintf(stderr,"_command_parse()\r\n");
	while ((end == 0) && (error == 0))
	{
		if (no_new_token == 0)
		{
			ret = _command_next_token(cmd);
		}
		else
		{
			ret = 0;
			no_new_token = 0;
		}
		if (ret == 0)
		{
//			fprintf(stderr,"token:%s type:%d\r\n",cmd->token,cmd->token_type);
			switch (state)
			{
				case PARSER_STATE_CMD_NAME:
					if (cmd->token_type == TOKEN_CMD_NAME)
					{
						i = 0;						
						while (i < CMD_COUNT)
						{
							if (strcmp(cmd->token, cmd_names[i]) == 0)
							{
								cmd->id = i;
								break;
							}
							else
							{
								i++;
							}
						}
						if (i >= CMD_COUNT)
						{
							error = -1;
						}
						else
						{
							if (cmd_path_count[cmd->id] != 0)
							{
								state = PARSER_STATE_CMD_PATH;
								cmd->path_count = 0;
							}
							else
							{
								if (cmd_value_type[cmd->id] != VALUE_TYPE_NULL)
								{
									state = PARSER_STATE_CMD_VALUE;
								}
								else
								{
									cmd->value_type = VALUE_TYPE_NULL;
									cmd->value_count = 0;
									cmd->value_length = 0;
									end = -1;
								}
							}
						}
					}
					else
					{
						error = -1;
					}
					if (error)
					{
						if (cmd->token_type == TOKEN_EOL)
							snprintf(cmd->line, (size_t) cmd->line_length, "Error: Unexpected EOL");
						else
							snprintf(cmd->line, (size_t) cmd->line_length, "Error: Wrong command token");
					}
					break;
				case	PARSER_STATE_CMD_PATH:
					switch (cmd->token_type)
					{
						case TOKEN_PATH_NAME:
							if (cmd->path_count < MAX_PATH)
							{
								cmd->path[cmd->path_count] = cmd->token;
								cmd->path_count++;
								if (cmd_path_count[cmd->id] == cmd->path_count)
								{
									if (cmd_value_type[cmd->id] != VALUE_TYPE_NULL)
									{
										state = PARSER_STATE_CMD_VALUE;
									}
									else
									{
										cmd->value_type = VALUE_TYPE_NULL;
										cmd->value_count = 0;
										cmd->value_length = 0;
										end = -1;
									}
								}
							}
							else
							{
								error = -1;
								snprintf(cmd->line, (size_t) cmd->line_length, "Error: Too long path");
							}
							break;
						case TOKEN_INTEGER:
						case TOKEN_REAL:
						case TOKEN_STRING:
						case TOKEN_HEX:
							pc = (cmd_path_count[cmd->id] >= 0) ? cmd_path_count[cmd->id] : -cmd_path_count[cmd->id];
							if (cmd->path_count >= pc)
							{
								if (cmd_value_type[cmd->id] != VALUE_TYPE_NULL)
								{
									no_new_token = -1;
									state = PARSER_STATE_CMD_VALUE;
								}
								else
								{
									cmd->value_type = VALUE_TYPE_NULL;
									cmd->value_count = 0;
									cmd->value_length = 0;
									end = -1;
								}
							}
							else
							{
								error = -1;
								if (cmd->path_count == 0)
									snprintf(cmd->line, (size_t) cmd->line_length, "Error: Missing path");
								else
									snprintf(cmd->line, (size_t) cmd->line_length, "Error: The path too short");
							}
							break;
						case TOKEN_EOL:
							pc = (cmd_path_count[cmd->id] >= 0) ? cmd_path_count[cmd->id] : -cmd_path_count[cmd->id];
							if ((cmd->path_count >= pc) && (cmd_value_type[cmd->id] == VALUE_TYPE_NULL))
							{
								cmd->value_type = VALUE_TYPE_NULL;
								cmd->value_count = 0;
								cmd->value_length = 0;
								end = -1;
							}
							else
							{
								error = -1;
								snprintf(cmd->line, (size_t) cmd->line_length, "Error: Unexpected EOL");
							}
							break;
						default:
							error = -1;
							snprintf(cmd->line, (size_t) cmd->line_length, "Error: Unexpected token");
							break;
					}
					break;
				case	PARSER_STATE_CMD_VALUE:
//					fprintf(stderr,"Parsing value\r\n");
					switch (cmd_value_type[cmd->id])
					{
						case VALUE_TYPE_INT:
							switch (cmd->token_type)
							{
								case TOKEN_INTEGER:
//									fprintf(stderr,"Parsing integer\r\n");
									ret =sscanf(cmd->token,"%d", &cmd->values.i);
//									fprintf(stderr,"ret:%d\r\n",ret);
//									if (ret == 1) fprintf(stderr,"value:%d\r\n",cmd->values.i);
									if (ret != 1) error = -1;
									break;
								case TOKEN_HEX:
									ret =sscanf(cmd->token,"%x", &cmd->values.i);
									if (ret != 1) error = -1;
									break;
								default:
									error=-2;
									break;
							}
							cmd->value_length = (int) sizeof(int);
							cmd->value_type = VALUE_TYPE_INT;
							cmd->value_count = 1;
							break;
						case VALUE_TYPE_LONG:
							switch (cmd->token_type)
							{
								case TOKEN_INTEGER:
									ret =sscanf(cmd->token,"%ld", &cmd->values.l);
									if (ret != 1) error = -1;
									break;
								case TOKEN_HEX:
									ret =sscanf(cmd->token,"%lx", &cmd->values.l);
									if (ret != 1) error = -1;
									break;
								default:
									error=-2;
									break;
							}
							cmd->value_length = (int) sizeof(long);
							cmd->value_type = VALUE_TYPE_LONG;
							cmd->value_count = 1;
							break;
						case VALUE_TYPE_FLOAT:
							switch (cmd->token_type)
							{
								case TOKEN_REAL:
								case TOKEN_INTEGER:
									ret =sscanf(cmd->token,"%f", &cmd->values.f);
									if (ret != 1) error = -1;
									break;
								default:
									error=-2;
									break;
							}
							cmd->value_length = (int) sizeof(float);
							cmd->value_type = VALUE_TYPE_FLOAT;
							cmd->value_count = 1;
							break;
						case VALUE_TYPE_DOUBLE:
							switch (cmd->token_type)
							{
								case TOKEN_REAL:
								case TOKEN_INTEGER:
									ret =sscanf(cmd->token,"%lf", &cmd->values.d);
									if (ret != 1) error = -1;
									break;
								default:
									error=-2;
									break;
							}
							cmd->value_length = (int) sizeof(double);
							cmd->value_type = VALUE_TYPE_DOUBLE;
							cmd->value_count = 1;
							break;
						case VALUE_TYPE_STRING:
							switch (cmd->token_type)
							{
								case TOKEN_STRING:
									if (strlen(cmd->token) < 32)
									{
										strcpy(cmd->values.ch, cmd->token);
									}
									else
									{
										error = -3;
									}
									break;
								default:
									error=-2;
									break;
							}
							cmd->value_length = (int) (strlen(cmd->token) + 1);
							cmd->value_type = VALUE_TYPE_STRING;
							cmd->value_count = 1;
							break;
						default:
							error = -4;
							break;
					}
					if (error == 0) cmd->value_count = 1;
					if (error == -1)
					{
						snprintf(cmd->line, (size_t) cmd->line_length, "Error: Number conversion failed");
					}
					if (error == -2)
					{
						if (cmd->token_type == TOKEN_EOL)
							snprintf(cmd->line, (size_t) cmd->line_length, "Error: Unexpected EOL");
						else
							snprintf(cmd->line, (size_t) cmd->line_length, "Error: Wrong argument token type");
					}
					if (error == -3)
					{
						snprintf(cmd->line, (size_t) cmd->line_length, "Error: Too long string argument");
					}
					if (error == -4)
					{
						snprintf(cmd->line, (size_t) cmd->line_length, "Error: Unexpected VALUE_TYPE");
					}
					end = -1;
					break;
				
			} // end switch(state)
		}
		else
		{
			error = -1; // _command_nex_toke() already has written the error text
		} //end if(ret == 0)
	} // end while()
//	if (error)
//		console_putstr("Parser exited with error\r\n");
	return error;
}

