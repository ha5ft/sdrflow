#*******************************************************************************
#* 							Makefile for sdrflow compiler
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

BUILDDIR=../../build/system/sdfsrc
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
	${BUILDDIR}/src_main.o

LIBRARYFILES=\
	${SYSLIBDIR}/libsrc.a

EXEFILE=\
	${BUILDDIR}/sdfsrc

INSTALLEDFILE=\
	${BINDIR}/sdfsrc

#
# Header files dependencies
#

SRC_MAIN_DEP=\
	${COMMONDIR}/config.h \
	${COMMONDIR}/common.h \
	${COMMONDIR}/sdferr.h \
	${COMMONDIR}/io.h \
	${COMMONDIR}/words.h \
	${COMMONDIR}/pointer.h \
	${COMMONDIR}/hash_table.h \
	${COMMONDIR}/src_lang.h \
	${COMMONDIR}/scanner.h \
	${COMMONDIR}/src_code.h \
	${COMMONDIR}/context.h \
	${COMMONDIR}/data.h \
	${COMMONDIR}/location.h

# Phony targets

PHONY_TARGETS=\
	all \
	sdfsrc \
	src_main \
	libsrc \
	clean \
	test

.PHONY: ${PHONY_TARGETS}

# Targets to be passed to make in the command line

all: ${EXEFILE}

sdfsrc: ${EXEFILE}

src_main: ${BUILDDIR}/src_main.o

clean:
	@${RM} ${OBJECTFILES}
	@${RM} ${EXEFILE}
	@${RM} ${INSTALLEDFILE}

# Internally used target for building the executable file

${EXEFILE}: ${OBJECTFILES} libsrc
	@${RM} ${EXEFILE}
	@${RM} ${INSTALLEDFILE}
	@${LINK} ${LINKFLAGS} -o ${EXEFILE} ${OBJECTFILES} ${LIBRARYFILES}
	@${CP} ${EXEFILE} ${BINDIR}

# Internally used phony target to build the library file if necessary

libsrc:
	@${MAKE} ${MKFLAGS} -f ../../common.build.mk libsrc

test:
	@echo "${MKFLAGS}"

# Internally used targets for compiling source files into object files

${BUILDDIR}/src_main.o: src_main.c ${SRC_MAIN_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/src_main.o src_main.c




