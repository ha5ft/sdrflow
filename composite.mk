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
	@if [ -d composite/$@ ]; \
	then \
	{ \
		echo "Building composite $@" && \
		deps="" && \
		for d in $(shell cat composite/$@/$@.sdf.src | grep ^use | awk '{print $$2}'); \
		do \
		{ \
			deps="$${deps} ../../context/$${d}.sdf.ctx" && \
			if [ -d primitive/$$d ]; \
			then \
			{ \
				${MAKE} -f primitive.mk $$d ; \
			} \
			else \
			{ \
				if [ -d composite/$$d ]; \
				then \
					${MAKE} -f composite.mk $$d ; \
				else \
					{ echo "Dependency $@ do not exist"; exit 1; } \
				fi; \
			} \
			fi; \
		} || { echo "Building dependency $d failed" ; exit 1; }\
		done && \
		if [ ! -d "${BUILDROOTDIR}/actor/$@" ]; then mkdir -p ${BUILDROOTDIR}/actor/$@; fi && \
		if [ ! -d "${ACTORDIR}" ]; then mkdir -p ${ACTORDIR}; fi && \
		if [ ! -d "${CONTEXTDIR}" ]; then mkdir -p ${CONTEXTDIR}; fi && \
		${MAKE} -C composite/$@ -f ../../composite.build.mk ../../actor/$@.sdf.bin DEPS="$${deps}" && \
		${MAKE} -C composite/$@ -f ../../composite.build.mk ../../context/$@.sdf.ctx; \
	} || { echo "Building $@ failed" ; exit 1; };\
	else \
	{ \
		echo "Composite $@ do not exist" ; \
		exit 1; \
	}; \
	fi;

