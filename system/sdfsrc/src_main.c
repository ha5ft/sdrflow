/*******************************************************************************
 * 							Sdf compiler src_main
 * *****************************************************************************
 * 	Filename:		src_main.h
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

#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	"../common/common.h"
#include	"../common/objects.h"
#include	"../common/io.h"
#include 	"../common/words.h"
#include	"../common/pointer.h"
#include 	"../common/hash_table.h"
#include 	"../common/src_lang.h"
#include	"../common/scanner.h"
#include 	"../common/sdferr.h"
#include	"../common/data.h"
#include	"../common/context.h"
#include	"../common/src_code.h"

#define	OPTION_COUNT		5
#define OPTION_INPUT_NAME	0
#define OPTION_INPUT_DIR	1
#define OPTION_BUILD_DIR	2
#define OPTION_CONTEXT_DIR	3
#define OPTION_BUFFER_TYPE	4

#define BUFFER_TYPE_STATIC_STR	"static"
#define BUFFER_TYPE_DYNAMIC_STR	"dynamic"

typedef struct _options
{
	char	*input_name;
	char	*input_dir;
	char	*build_dir;
	char	*context_dir;
	int		buffer_type;
} options_t;

static int	initialize();
static void	cleanup();
static int	parse_options(int argc, char **argv, options_t *options);
static void	print_usage();

int main(int argc, char **argv)
{
	void	*inp;
	void	*out;
	void	*src_code;
//	char	input_name[IO_BASE_NAME_LENGTH + 1];	
	options_t options =
	{
		.input_name = NULL,
		.input_dir = "./",
		.build_dir = "./",
		.context_dir = "./",
		.buffer_type = COMMON_BUFFER_TYPE_STATIC
	};

	fprintf(stderr,"SDF Compiler version 0.1\n");

//
// ********************* Initializaton ****************************************
//

	if (initialize() != 0)
	{
		cleanup();
		exit(1);
	}

//
// ************************ Parsing command line options **********************
//
//	fprintf(stderr, "src_main() : Starts parsing options\n");
	if (parse_options(argc, argv, &options) != 0)
	{
		fprintf(stderr,"Can not parse options\n");
		print_usage();
		cleanup();
		exit(1);
	}
	if (options.input_name == NULL)
	{
		fprintf(stderr,"Missing input name\n");
		print_usage();
		cleanup();
		exit(1);
	}
	if (strlen(options.input_name) > IO_BASE_NAME_LENGTH)
	{
		fprintf(stderr,"Too long input name\n");
		exit(1);
	}
//	fprintf(stderr, "src_main() : Finished parsing options\n");

//
// ************************ Parsing src module ********************************
//

	fprintf(stderr, "Starts compiling composite %s\n", options.input_name);
	if (context_add_location(IO_CHAN_TYPE_FILE, options.context_dir) != 0)
	{
		fprintf(stderr,"Can not add context dir to locations\n");
		cleanup();
		exit(1);
	}
	if ((inp = io_new_channel(IO_CHAN_TYPE_FILE,options.input_dir,options.input_name,IO_ITEM_TYPE_SRC_CODE)) == NULL)
	{
		fprintf(stderr, "Can not create input channel\n");
		cleanup();
		exit(1);
	}
	if (io_open_for_read(inp) != 0)
	{
		fprintf(stderr, "Can not open input channel\n");
		cleanup();
		exit(1);		
	}
	if ((src_code = src_code_new(options.input_name, inp, options.buffer_type)) == NULL)
	{
		fprintf(stderr, "Can not create src_code object\n");
		cleanup();
		exit(1);
	}
	if (src_code_parse(src_code) != 0)
	{
		fprintf(stderr, "Can not parse src_code\n");
		cleanup();
		exit(1);
	}
	io_close(inp);
	io_delete_channel(inp);

//
// ************************ Output source listing *****************************
//
/*
	if ((out = io_new_channel(IO_CHAN_TYPE_FILE,options.build_dir,options.input_name,IO_ITEM_TYPE_SRC_LST)) == NULL)
	{
		fprintf(stderr, "Can not create list channel\n");
		cleanup();
		exit(1);
	}
	if (io_open_for_write(out) != 0)
	{
		fprintf(stderr, "Can not open list channel\n");
		cleanup();
		exit(1);		
	}
	if (src_code_print(src_code, out) != 0)
	{
		fprintf(stderr, "Can not generate source list\n");
		io_close(out);
		io_delete_channel(out);
		cleanup();
		exit(1);
	}
	io_close(out);
	io_delete_channel(out);
*/
//
// ************************ Output asm code ***********************************
//

	if ((out = io_new_channel(IO_CHAN_TYPE_FILE,options.build_dir,options.input_name,IO_ITEM_TYPE_ASM_CODE)) == NULL)
	{
		fprintf(stderr, "Can not create asm channel\n");
		cleanup();
		exit(1);
	}
	if (io_open_for_write(out) != 0)
	{
		fprintf(stderr, "Can not open asm channe\n");
		cleanup();
		exit(1);		
	}
	if (src_code_generate_asm(src_code, out) != 0)
	{
		fprintf(stderr, "Can not generate asm code\n");
		io_close(out);
		io_delete_channel(out);
		cleanup();
		exit(1);		
	}
	io_close(out);
	io_delete_channel(out);

//
// ************************ Output context file *******************************
//

	if ((out = io_new_channel(IO_CHAN_TYPE_FILE,options.build_dir,options.input_name,IO_ITEM_TYPE_CONTEXT)) == NULL)
	{
		fprintf(stderr, "Can not create context channel\n");
		cleanup();
		exit(1);
	}
	if (io_open_for_write(out) != 0)
	{
		fprintf(stderr, "Can not open context channe\n");
		cleanup();
		exit(1);		
	}
	if (src_code_generate_context(src_code, out) != 0)
	{
		fprintf(stderr, "Can not generate context\n");
		io_close(out);
		io_delete_channel(out);
		cleanup();
		exit(1);		
	}
	io_close(out);
	io_delete_channel(out);

	fprintf(stderr, "Composite %s has been succesfully compiled\n", options.input_name);

//
// ************************ Cleaup and exit ***********************************
//

	cleanup();
	exit(0);
}

//
// ******************************** Private functions *************************

static int	initialize()
{
	if (objects_init() != 0)
	{
		fprintf(stderr, "Can not initialize the objects class\n");
		return -1;
	}
	if (pointer_initialize() != 0)
	{
		fprintf(stderr, "Can not initialize the pointer class\n");
		return -1;
	}
	if (words_init() != 0)
	{
		fprintf(stderr, "Can not initialize the words class\n");
		return -1;
	}
	if (io_init() != 0)
	{
		fprintf(stderr, "Can not initialize the io class\n");
		return -1;
	}
	if (hash_table_init() != 0)
	{
		fprintf(stderr, "Can not initialize the hash table class\n");
		return -1;
	}
	if (scanner_init() != 0)
	{
		fprintf(stderr, "Can not initialize the scanner class\n");
		return -1;
	}
	if (data_initialize() != 0)
	{
		fprintf(stderr, "Can not initialize the data class\n");
		return -1;
	}
	if (context_initialize() != 0)
	{
		fprintf(stderr, "Can not initialize the context class\n");
		return -1;
	}
	if (src_code_initialize() != 0)
	{
		fprintf(stderr, "Can not initialize the src_code class\n");
		return -1;
	}
	return 0;
}

static int	parse_options(int argc, char **argv, options_t *options)
{
	int n, i;

	char *option_names[OPTION_COUNT] =
	{
		[OPTION_INPUT_NAME] = "-i",
		[OPTION_INPUT_DIR] = "-I",
		[OPTION_BUILD_DIR] = "-B",
		[OPTION_CONTEXT_DIR] = "-C",
		[OPTION_BUFFER_TYPE] = "-b"
	};

//	fprintf(stderr, "parse_options() : argc = %d\n", argc);
//	fprintf(stderr, "parse_options() : argv1 = %s\n", *(argv + 1));
//	fprintf(stderr, "parse_options() : argv2 = %s\n", *(argv + 2));
//	fprintf(stderr, "parse_options() : argv3 = %s\n", *(argv + 3));
//	fprintf(stderr, "parse_options() : argv4 = %s\n", *(argv + 4));
	if (argc < 2)
	{
		return -1;
	}

	n = 1;
	while (n < argc)
	{
		i = 0;
		while (i < OPTION_COUNT)
		{
			if (strcmp(*(argv + n), option_names[i]) == 0)
			{
				n++;
				if (n > argc) return -1;
				switch (i)
				{
					case OPTION_INPUT_NAME:
						options->input_name = *(argv + n++);
						break;
					case OPTION_INPUT_DIR:
						options->input_dir = *(argv + n++);
						break;
					case OPTION_BUILD_DIR:
						options->build_dir = *(argv + n++);
						break;
					case OPTION_CONTEXT_DIR:
						options->context_dir = *(argv + n++);
						break;
					case OPTION_BUFFER_TYPE:
						if (strcmp(*(argv + n), BUFFER_TYPE_STATIC_STR) == 0)
						{
							options->buffer_type = COMMON_BUFFER_TYPE_STATIC;
							n++;
							break;
						}
						if (strcmp(*(argv +n), BUFFER_TYPE_DYNAMIC_STR) == 0)
						{
							options->buffer_type = COMMON_BUFFER_TYPE_DYNAMIC;
							n++;
							break;
						}
						return -1;
					default:
						return -1;
				}
				break;
			}
			i++;
		}
		if (i >= OPTION_COUNT) return -1;
	}
	return 0;
}

static void	print_usage()
{
	fprintf(stderr,"Usage: sdfsrc options -i composit_name\n");
	fprintf(stderr,"options:\n");
	fprintf(stderr,"\t-B builddir\n");
	fprintf(stderr,"\t-C contextdir\n");
	fprintf(stderr,"\t-b static | dynamic\n");
}

static void	cleanup()
{
//	fprintf(stderr, "Starting the cleanup\n");
	src_code_cleanup();
//	fprintf(stderr, "src_code cleaned up\n");
	context_cleanup();
//	fprintf(stderr, "context cleaned up\n");
	data_cleanup();
//	fprintf(stderr, "data cleaned up\n");
	scanner_cleanup();
//	fprintf(stderr, "scanner cleaned up\n");
	hash_table_cleanup();
//	fprintf(stderr, "hash_table cleaned up\n");
	io_cleanup();
//	fprintf(stderr, "io cleaned up\n");
	words_cleanup();
//	fprintf(stderr, "words cleaned up\n");
	pointer_cleanup();
//	fprintf(stderr, "pointer cleaned up\n");
	objects_cleanup();
//	fprintf(stderr, "objects cleaned up\n");
//	fprintf(stderr, "Finished the cleanup\n");	
}


