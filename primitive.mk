#*******************************************************************************
#* 							Makefile for sdrflow
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
AR=ar
LINK=gcc
ifdef _BENCHMARK_
MAKE=make _BENCHMARK_=1
CC=gcc -D_BENCHMARK_
else
MAKE=make --no-print-directory
CC=gcc
endif


BINDIR=./bin
LIBDIR=./lib
ACTORDIR=./actor
CONTEXTDIR=./context
COMPOSITEROOT=./composite
PRIMITIVEROOT=./primitive
SYSLIBDIR=./syslib
COMMONDIR=./system/common
SDFASMDIR=./system/sdfasm
SDFSRCDIR=./system/sdfsrc
SDFRUNDIR=./system/sdfrun
BUILDROOTDIR=./build

%:
	@if [ -d primitive/$@ ]; \
	then \
	{ \
		echo "Building $@" && \
		if [ ! -d "${BUILDROOTDIR}/actor/$@" ]; then mkdir -p ${BUILDROOTDIR}/actor/$@; fi && \
		if [ ! -d "${ACTORDIR}" ]; then mkdir -p ${ACTORDIR}; fi && \
		if [ ! -d "${CONTEXTDIR}" ]; then mkdir -p ${CONTEXTDIR}; fi && \
		${MAKE} -C primitive/$@ -f ../../primitive.build.mk ../../actor/$@.sdf.so && \
		${MAKE} -C primitive/$@ -f ../../primitive.build.mk ../../context/$@.sdf.ctx; \
	} || { echo "Building $@ failed" ; exit 1; };\
	else \
	{ \
		echo "Primitive $@ do not exist" ; \
		exit 1; \
	}; \
	fi;

