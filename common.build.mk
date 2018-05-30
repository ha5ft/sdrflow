#*******************************************************************************
#* 							Makefile for sdrflow syslibs
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

CFLAGS=-c -Wall -O2 -DSDF_PLATFORM=0
LIBFLAGS=rcs
LINKFLAGS=-Wall

BUILDDIR=../../build/system/common
BINDIR=../../bin
LIBDIR=../../lib
SYSLIBDIR=../../syslib

#
# Object files to be generated
#

OBJFILES= \
	${BUILDDIR}/asm_code.o \
	${BUILDDIR}/asm_lang.o \
	${BUILDDIR}/composite.o \
	${BUILDDIR}/context.o \
	${BUILDDIR}/data.o \
	${BUILDDIR}/hash_table.o \
	${BUILDDIR}/io.o \
	${BUILDDIR}/location.o \
	${BUILDDIR}/obj_code.o \
	${BUILDDIR}/objects.o \
	${BUILDDIR}/pointer.o \
	${BUILDDIR}/primitive.o \
	${BUILDDIR}/program.o \
	${BUILDDIR}/scanner.o \
	${BUILDDIR}/sentence.o \
	${BUILDDIR}/src_code.o \
	${BUILDDIR}/src_lang.o \
	${BUILDDIR}/words.o

LIBASMOBJ= \
	${BUILDDIR}/asm_code.o \
	${BUILDDIR}/asm_lang.o \
	${BUILDDIR}/hash_table.o \
	${BUILDDIR}/io.o \
	${BUILDDIR}/obj_code.o \
	${BUILDDIR}/objects.o \
	${BUILDDIR}/pointer.o \
	${BUILDDIR}/scanner.o \
	${BUILDDIR}/sentence.o \
	${BUILDDIR}/words.o

LIBSRCOBJ= \
	${BUILDDIR}/context.o \
	${BUILDDIR}/data.o \
	${BUILDDIR}/hash_table.o \
	${BUILDDIR}/io.o \
	${BUILDDIR}/objects.o \
	${BUILDDIR}/pointer.o \
	${BUILDDIR}/scanner.o \
	${BUILDDIR}/src_code.o \
	${BUILDDIR}/src_lang.o \
	${BUILDDIR}/location.o \
	${BUILDDIR}/words.o

LIBRUNOBJ= \
	${BUILDDIR}/composite.o \
	${BUILDDIR}/io.o \
	${BUILDDIR}/location.o \
	${BUILDDIR}/objects.o \
	${BUILDDIR}/pointer.o \
	${BUILDDIR}/primitive.o \
	${BUILDDIR}/program.o

LIBRARYFILES= \
	${SYSLIBDIR}/libasm.a \
	${SYSLIBDIR}/libsrc.a \
	${SYSLIBDIR}/librun.a

#
# Header files dependencies
#

ASM_CODE_DEP= \
	asm_code.h \
	config.h \
	common.h \
	sentence.h \
	scanner.h \
	hash_table.h \
	value.h \
	allocators.h \
	instructions.h \
	labels.h \
	segments.h \
	obj_format.h \
	asm_lang.h \
	objects.h \
	words.h \
	sdferr.h \
	io.h

ASM_LANG_DEP= \
	asm_lang.h \
	config.h \
	common.h \
	value.h \
	allocators.h \
	instructions.h \
	labels.h \
	segments.h \
	obj_format.h \
	hash_table.h \
	words.h \
	sentence.h

COMPOSIT_DEP= \
	composite.h \
	config.h \
	common.h \
	objects.h \
	obj_format.h \
	io.h \
	location.h \
	primitive.h \
	pointer.h

CONTEXT_DEP= \
	context.h \
	config.h \
	common.h \
	objects.h \
	pointer.h \
	words.h \
	hash_table.h \
	scanner.h \
	io.h \
	data.h \
	src_lang.h \
	location.h

DATA_DEP=\
	data.h \
	config.h \
	common.h \
	objects.h \
	words.h \
	hash_table.h \
	scanner.h \
	io.h \
	src_lang.h \
	value.h \
	allocators.h \
	pointer.h \
	context.h

HASH_TABLE_DEP= \
	hash_table.h \
	config.h \
	common.h \
	objects.h \
	words.h

IO_DEP= \
	io.h \
	config.h \
	common.h \
	objects.h

LOCATION_DEP=\
	location.h \
	config.h \
	common.h \
	io.h

OBJ_CODE_DEP=\
	obj_code.h \
	config.h \
	common.h \
	obj_format.h \
	asm_lang.h \
	io.h

OBJECTS_DEP= \
	objects.h \
	config.h \
	common.h \

POINTER_DEP=\
	pointer.h \
	config.h \
	common.h \
	objects.h

PRIMITIVE_DEP=\
	primitive.h \
	config.h \
	common.h \
	obj_format.h \
	objects.h \
	io.h \
	location.h

PROGRAM_DEP= \
	program.h \
	config.h \
	common.h \
	instructions.h \
	obj_format.h \
	composite.h \
	objects.h

SCANNER_DEP=\
	scanner.h \
	config.h \
	common.h \
	objects.h \
	hash_table.h \
	words.h \
	io.h

SENTENCE_DEP= \
	sentence.h \
	config.h \
	common.h \
	objects.h \
	pointer.h

SRC_CODE_DEP= \
	src_code.h \
	config.h \
	common.h \
	objects.h \
	words.h \
	hash_table.h \
	scanner.h \
	io.h \
	context.h \
	data.h \
	pointer.h \
	src_lang.h

WORDS_DEP=\
	words.h \
	config.h \
	common.h \
	objects.h

# Targets for the makefile

PHONY_TARGETS=\
	asm_code \
	asm_lang \
	composite \
	context \
	data \
	hash_table \
	io \
	location \
	obj_code \
	objects \
	pointer \
	primitive \
	program \
	scanner \
	sentence \
	src_code \
	src_lang \
	words \
	all \
	libasm \
	libsrc \
	librun \
	clean

.PHONY: ${PHONY_TARGETS}

# Targets to be passed to make in the command line

all: ${LIBRARYFILES}

libasm: ${SYSLIBDIR}//libasm.a

libsrc: ${SYSLIBDIR}//libsrc.a

librun: ${SYSLIBDIR}//librun.a

asm_code: ${BUILDDIR}/asm_code.o

asm_lang: ${BUILDDIR}/asm_lang.o

composite: ${BUILDDIR}/composite.o

context: ${BUILDDIR}/context.o

data: ${BUILDDIR}/data.o

hash_table: ${BUILDDIR}/hash_table.o

io: ${BUILDDIR}/io.o

location: ${BUILDDIR}/location.o

obj_code: ${BUILDDIR}/obj_code.o

objects: ${BUILDDIR}/objects.o 

pointer: ${BUILDDIR}/pointer.o

primitive: ${BUILDDIR}/primitive.o

program: ${BUILDDIR}/program.o

scanner: ${BUILDDIR}/scanner.o

sentence: ${BUILDDIR}/sentence.o

src_code: ${BUILDDIR}/src_code.o

src_lang: ${BUILDDIR}/src_lang.o

words: ${BUILDDIR}/words.o

clean:
	@${RM} ${BUILDDIR}/*.o
	@${RM} ${BUILDDIR}/*.a

# Internally used target for building static libraries

${SYSLIBDIR}/libasm.a: ${LIBASMOBJ}
	@${RM} ${BUILDDIR}/libasm.a
	@${AR} ${LIBFLAGS} ${BUILDDIR}/libasm.a ${LIBASMOBJ}
	@${CP} ${BUILDDIR}/libasm.a ${SYSLIBDIR}

${SYSLIBDIR}/libsrc.a: ${LIBSRCOBJ}
	@${RM} ${BUILDDIR}/libsrc.a
	@${AR} ${LIBFLAGS} ${BUILDDIR}/libsrc.a ${LIBSRCOBJ}
	@${CP} ${BUILDDIR}/libsrc.a ${SYSLIBDIR}

${SYSLIBDIR}/librun.a: ${LIBRUNOBJ}
	@${RM} ${BUILDDIR}/linrun.a
	@${AR} ${LIBFLAGS} ${BUILDDIR}/librun.a ${LIBRUNOBJ}
	@${CP} ${BUILDDIR}/librun.a ${SYSLIBDIR}

# Internally used targets for compiling source files into object files

${BUILDDIR}/asm_code.o: asm_code.c ${ASM_CODE_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/asm_code.o asm_code.c

${BUILDDIR}/asm_lang.o: asm_lang.c ${ASM_LANG_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/asm_lang.o asm_lang.c

${BUILDDIR}/composite.o: composite.c ${COMPOSITE_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/composite.o composite.c

${BUILDDIR}/context.o: context.c ${CONTEXT_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/context.o context.c

${BUILDDIR}/data.o: data.c ${DATA_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/data.o data.c

${BUILDDIR}/hash_table.o: hash_table.c ${HASH_TABLE_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/hash_table.o hash_table.c

${BUILDDIR}/io.o: io.c ${IO_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/io.o io.c

${BUILDDIR}/location.o: location.c ${LOCATION_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/location.o location.c

${BUILDDIR}/obj_code.o: obj_code.c ${OBJ_CODE_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/obj_code.o obj_code.c

${BUILDDIR}/objects.o: objects.c ${OBJECTS_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/objects.o objects.c

${BUILDDIR}/pointer.o: pointer.c ${POINTER_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/pointer.o pointer.c

${BUILDDIR}/primitive.o: primitive.c ${PRIMITIVE_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/primitive.o primitive.c

${BUILDDIR}/program.o: program.c ${PROGRAM_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/program.o program.c

${BUILDDIR}/scanner.o: scanner.c ${SCANNER_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/scanner.o scanner.c

${BUILDDIR}/sentence.o: sentence.c ${SENTENCE_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/sentence.o sentence.c

${BUILDDIR}/src_code.o: src_code.c ${SRC_CODE_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/src_code.o src_code.c

${BUILDDIR}/src_lang.o: src_lang.c ${SRC_LANG_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/src_lang.o src_lang.c

${BUILDDIR}/words.o: words.c ${WORDS_DEP}
	@${CC} ${CFLAGS} -o ${BUILDDIR}/words.o words.c


