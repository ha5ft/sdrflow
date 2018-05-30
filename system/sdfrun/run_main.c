/*******************************************************************************
 * 							sdrflow runtime run_main
 * *****************************************************************************
 * 	Filename:		run_main.c
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
#include	"../common/instructions.h"
#include	"../common/obj_format.h"
#include	"../common/primitive.h"
#include	"../common/location.h"
#include	"../common/composite.h"
#include	"../common/program.h"
#include	"../common/pointer.h"
#include	"../common/io.h"
#include	"console.h"
#include	"command.h"


int main(int argc, char **argv)
{
	int			ret __attribute__((unused));

	fprintf(stderr,"sdrflow runtime version 0.1\r\n");

//************************* Initialization ************************************

	if (objects_init() != 0)
	{
		fprintf(stderr, "main: Can not init objects class\r\n");
		exit(1);
	}
//	fprintf(stderr,"main: Objects component initialized\r\n");

	console_init();
//	fprintf(stderr,"main: Console initialized\r\r\n");

	if (pointer_initialize() != 0)
	{
		fprintf(stderr, "main: Can not init pointer class\r\r\n");
		console_cleanup();
		objects_cleanup();
		exit(1);
	}
//	fprintf(stderr,"main: Pointer class initialized\r\r\n");

	if (command_init() != 0)
	{
		fprintf(stderr, "main: Can not init command class\r\r\n");
		pointer_cleanup();
		console_cleanup();
		objects_cleanup();
		exit(1);
	}
//	fprintf(stderr,"main: Command class initialized\r\r\n");

	if (location_init() != 0)
	{
		fprintf(stderr,"main: Can not initialize location vomponent\r\r\n");
		command_cleanup();
		pointer_cleanup();
		console_cleanup();
		objects_cleanup();
		exit(1);
	}
//	fprintf(stderr,"main: Location component initialized\r\n");

	if (io_init() != 0)
	{
		fprintf(stderr,"main: Can not initialize IO component\r\n");
		location_cleanup();
		command_cleanup();
		pointer_cleanup();
		console_cleanup();
		objects_cleanup();
		exit(1);
	}
//	fprintf(stderr,"main: IO component initialized\r\n");

	if (primitive_init() != 0)
	{
		fprintf(stderr, "main: Can not init primitive class\r\n");
		io_cleanup();
		location_cleanup();
		command_cleanup();
		pointer_cleanup();
		console_cleanup();
		objects_cleanup();
		exit(1);
	}

	if (composite_init() != 0)
	{
		fprintf(stderr,"main: Can not initialize composite class\r\n");
		primitive_cleanup();
		io_cleanup();
		location_cleanup();
		command_cleanup();
		pointer_cleanup();
		console_cleanup();
		objects_cleanup();
		exit(1);		
	}
//	fprintf(stderr,"main: composite class initialized\r\n");

	if (program_init() != 0)
	{
		fprintf(stderr,"main: Can not initialize program component\r\n");
		composite_cleanup();
		primitive_cleanup();
		io_cleanup();
		location_cleanup();
		command_cleanup();
		pointer_cleanup();
		console_cleanup();
		objects_cleanup();
		exit(1);				
	}
//	fprintf(stderr,"main: Program component initialized\r\n");

//************************* End of initialization *****************************

//************************ Main programming loop ****************************

	console_run();

//******************* End of main programming loop **************************

//************************ Cleanup ********************************************

//	fprintf(stderr,"main: Starting the runtime cleanup\r\n");
	program_cleanup();
//	fprintf(stderr,"main: program component cleandup\r\n");
	composite_cleanup();
//	fprintf(stderr,"main: composite class cleandup\r\n");
	primitive_cleanup();
//	fprintf(stderr,"main: location class cleandup\r\n");
	io_cleanup();
//	fprintf(stderr,"main: primitive class cleandup\r\n");
	location_cleanup();
//	fprintf(stderr,"main: io class cleandup\r\n");
	command_cleanup();
//	fprintf(stderr,"main: command class cleandup\r\n");
	pointer_cleanup();
//	fprintf(stderr,"main: pointer class cleandup\r\n");
	console_cleanup();
//	fprintf(stderr,"main: console class cleandup\r\n");
	objects_cleanup();
//	fprintf(stderr,"main: objects class cleandup\r\n");
//	fprintf(stderr,"sdf runtime finished\r\n");
	
	return 0;
}
