/*******************************************************************************
 * 							Sdf assembler asm_main
 * *****************************************************************************
 * 	Filename:		asm_main.c
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
#include	"../common/config.h"
#include	"../common/objects.h"
#include	"../common/io.h"
#include 	"../common/words.h"
#include	"../common/pointer.h"
#include	"../common/sentence.h"
#include 	"../common/hash_table.h"
#include	"../common/scanner.h"
#include 	"../common/sdferr.h"
#include	"../common/asm_code.h"
#include	"../common/asm_lang.h"
#include 	"../common/obj_code.h"

#define	OPTION_COUNT		3
#define OPTION_INPUT_NAME	0
#define OPTION_INPUT_DIR	1
#define OPTION_BUILD_DIR	2

typedef struct _options
{
	char	*input_name;
	char	*input_dir;
	char	*build_dir;
} options_t;

static int	initialize();
static void	cleanup();
static int	parse_options(int argc, char **argv, options_t *options);
static void	print_usage();

int main(int argc, char **argv)
{
	void		*inp = NULL;
	void		*out = NULL;
	void		*asm_code = NULL;
	void		*obj_code = NULL;
	options_t 	options =
				{
					.input_name = NULL,
					.input_dir = "./",
					.build_dir = "./"
				};

	fprintf(stderr,"SDF Assembler version 0.1\n");

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

	fprintf(stderr, "Starts assembling composite %s\n", options.input_name);
	if ((inp = io_new_channel(IO_CHAN_TYPE_FILE,options.input_dir,options.input_name,IO_ITEM_TYPE_ASM_CODE)) == NULL)
	{
		fprintf(stderr, "error: main(): Can not create input channel\n");
		cleanup();
		exit(1);
	}
	if (io_open_for_read(inp) != 0)
	{
		fprintf(stderr, "error: main(): Can not open input channel\n");
		cleanup();
		exit(1);		
	}
	if ((asm_code = asm_code_new()) == NULL)
	{
		fprintf(stderr, "error: main(): Can not create new asm_code object\n");
		cleanup();
		exit(1);
	}
	if (asm_code_build_prolog(asm_code) == -1)
	{
		fprintf(stderr, "error: main(): Error in building prolog\n");
		cleanup();
		exit(1);
	}
	if (asm_code_analyse(asm_code, inp) != 0)
	{
		fprintf(stderr, "error: main(): Can not parse asm_code\n");
		cleanup();
		exit(1);
	}
	if (asm_code_set_sizes(asm_code) == -1)
	{
		fprintf(stderr, "error: main(): Parser can not set segment sizes\n");
		cleanup();
		exit(1);
	}
	if ((obj_code = asm_code_generate(asm_code)) == NULL)
	{
		fprintf(stderr, "error: main(): Can not generate object code\n");
		cleanup();
		exit(1);
	}
	io_close(inp);
	io_delete_channel(inp);
/*
	if ((out = io_new_channel(IO_CHAN_TYPE_FILE,options.build_dir,options.input_name,IO_ITEM_TYPE_PRN)) == NULL)
	{
		fprintf(stderr, "error: main(): Can not create prn channel\n");
		cleanup();
		exit(1);
	}
	if (io_open_for_write(out) != 0)
	{
		fprintf(stderr, "error: main(): Can not open prn channel\n");
		cleanup();
		exit(1);		
	}
	asm_code_print(asm_code, out);
	if ((out = io_new_channel(IO_CHAN_TYPE_FILE,options.build_dir,options.input_name,IO_ITEM_TYPE_LST)) == NULL)
	{
		fprintf(stderr, "error: main(): Can not create lst channel\n");
		cleanup();
		exit(1);
	}
	if (io_open_for_write(out) != 0)
	{
		fprintf(stderr, "error: main(): Can not open lst channel\n");
		cleanup();
		exit(1);		
	}
	asm_code_list(asm_code, out);
	io_close(out);
	io_delete_channel(out);
*/
	asm_code_delete(asm_code);
	asm_code = NULL;
	fprintf(stderr, "Finished assembling composite %s\n", options.input_name);

//
// ************************ Output object code ***********************************
//

	if ((out = io_new_channel(IO_CHAN_TYPE_FILE,options.build_dir,options.input_name,IO_ITEM_TYPE_BIN_OBJ_CODE)) == NULL)
	{
		fprintf(stderr, "error: main(): Can not create bin channel\n");
		cleanup();
		exit(1);
	}
	if (io_open_for_write(out) != 0)
	{
		fprintf(stderr, "error: main(): Can not open bin channel\n");
		cleanup();
		exit(1);		
	}
	if (obj_code_save_bin(obj_code, out) != 0)
	{
		fprintf(stderr, "error: main(): Can not save obj code\n");
		io_close(out);
		io_delete_channel(out);
		cleanup();
		exit(1);		
	}
	io_close(out);
	io_delete_channel(out);
	free(obj_code);
	obj_code = NULL;
	fprintf(stderr, "Object code has been written for composite %s\n", options.input_name);

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
	sentence_init();
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
	if (asm_code_init() != 0)
	{
		fprintf(stderr, "Can not initialize the asm_code class\n");
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
		[OPTION_BUILD_DIR] = "-B"
	};

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
	asm_code_cleanup();
	scanner_cleanup();
	hash_table_cleanup();
	io_cleanup();
	sentence_cleanup();
	pointer_cleanup();
	words_cleanup();
	objects_cleanup();
}


