#*******************************************************************************
#* 							Makefile for sdrflow runtime
#* *****************************************************************************
#* 	Filename:		makefile
#* 	Platform:		ubuntu 16.04 64 bit
#* 	Author:			Copyright (C) Selmeczi János, original version
#*******************************************************************************
#*							Licensing
#*******************************************************************************
#*  This file is part of sdrflow.
#*
#*  Sdrflow is free software: you can redistribute it and/or modify
#*  it under the terms of the GNU General Public License as published by
#*  the Free Software Foundation, either version 3 of the License, or
#*  (at your option) any later version.
#*
#*  Sdrflow is distributed in the hope that it will be useful,
#*  but WITHOUT ANY WARRANTY; without even the implied warranty of
#*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#*  GNU General Public License for more details.
#*
#*  You should have received a copy of the GNU General Public License
#*  along with sdrflow.  If not, see <http://www.gnu.org/licenses/>.
#*******************************************************************************
#*							Revision history
#*******************************************************************************
#*	Author				Date		Comment
#*******************************************************************************
#*	Selmeczi János		23-04-2018	Original version
#*
#******************************************************************************/


CP=cp
RM=rm -f
CC=gcc
AR=ar
LINK=gcc
MAKE=make

BUILDDIR=../../build/system/sdfrun
BINDIR=../../bin
LIBDIR=../../lib
SYSLIBDIR=../../syslib
COMMONDIR=../common

CFLAGS=-c -Wall -O2 -DSDF_PLATFORM=0
LIBFLAGS=rcs
LINKFLAGS=-Wall
MKFLAGS=-C ${COMMONDIR}

#
# Object files to be generated
#

OBJECTFILES=\
	${BUILDDIR}/run_main.o \
	${BUILDDIR}/console.o \
	${BUILDDIR}/command.o

LIBRARYFILES=\
	${SYSLIBDIR}/librun.a \
	-ldl \
	-lpthread

EXEFILE=\
	${BUILDDIR}/sdfrun

INSTALLEDFILE=\
	${BINDIR}/sdfrun

#
# Header files dependencies
#

RUN_MAIN_DEP=\
	${COMMONDIR}/config.h \
	${COMMONDIR}/common.h \
	${COMMONDIR}/sdferr.h \
	${COMMONDIR}/objects.h \
	${COMMONDIR}/instructions.h \
	${COMMONDIR}/obj_format.h \
	${COMMONDIR}/primitive.h \
	${COMMONDIR}/location.h \
	${COMMONDIR}/composite.h \
	${COMMONDIR}/program.h \
	${COMMONDIR}/pointer.h \
	${COMMONDIR}/io.h \
	console.h \
	command

CONSOLE_DEP=\
	console.h \
	command.h

COMMAND_DEP=\
	${COMMONDIR}/objects.h \
	${COMMONDIR}/program.h \
	${COMMONDIR}/pointer.h \
	command.h

# Phony targets

PHONY_TARGETS=\
	all \
	sdfrun \
	run_main \
	console \
	command \
	librun \
	clean

.PHONY: ${PHONY_TARGETS}

# Targets to be passed to make in the command line

all: ${EXEFILE}

sdfrun: ${EXEFILE}

run_main: ${BUILDDIR}/run_main.o

testprim: ${BUILDDIR}/testprim.o

console: ${BUILDDIR}/console.o

command: ${BUILDDIR}/command.o

clean:
	@${RM} ${OBJECTFILES}
	@${RM} ${EXEFILE}
	@${RM} ${INSTALLEDFILE}

# Internally used target for building the executable file

${BUILDDIR}/sdfrun: ${OBJECTFILES} librun
	@${RM} ${EXEFILE}
	@${RM} ${INSTALLEDFILE}
	@${LINK} ${LINKFLAGS} -o ${EXEFILE} ${OBJECTFILES} ${LIBRARYFILES}
	@${CP} ${EXEFILE} ${BINDIR}

# Internally used phony target to build the library file if necessary

librun:
	@${MAKE} ${MKFLAGS} -f ../../common.build.mk librun

# Internally used targets for compiling source files into object files

${BUILDDIR}/run_main.o: run_main.c ${RUN_MAIN_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/run_main.o run_main.c

${BUILDDIR}/console.o: console.c ${CONSOLE_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/console.o console.c

${BUILDDIR}/command.o: command.c ${COMMAND_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/command.o command.c



