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

# Targets to be passed to make in the command line

%:
	@case $@ in \
		all) \
			for s in system/*; \
			do \
				if [ -d "$$s" ]; \
				then \
					{ \
						echo "Building $$s" && \
						if [ ! -d "${BUILDROOTDIR}/$$s" ]; then mkdir -p ${BUILDROOTDIR}/$$s; fi &&\
						if [ ! -d "${BINDIR}" ]; then mkdir -p ${BINDIR}; fi && \
						if [ ! -d "${SYSLIBDIR}" ]; then mkdir -p ${SYSLIBDIR}; fi && \
						${MAKE} -C $$s -f ../../$${s##*/}.build.mk all; \
					} || { echo "Building $$s failed" ; exit 1; };\
				fi; \
			done; \
			for p in primitive/*;\
			do \
				if [ -d "$$p" ]; \
				then \
					{ \
						${MAKE} -f primitive.mk $${p##*/}; \
					} || { exit 1; }\
				fi; \
			done; \
			for c in composite/*; \
			do \
				if [ -d "$$c" ]; \
				then \
					{ \
						${MAKE} -f composite.mk $${c##*/}; \
					} || { exit 1; }\
				fi; \
			done;; \
		executables) \
			for s in system/*; \
			do \
				if [ -d "$$s" ]; \
				then \
					{ \
						echo "Building $$s" && \
						if [ ! -d "${BUILDROOTDIR}/$$s" ]; then mkdir -p ${BUILDROOTDIR}/$$s; fi && \
						if [ ! -d "${BINDIR}" ]; then mkdir -p ${BINDIR}; fi && \
						if [ ! -d "${SYSLIBDIR}" ]; then mkdir -p ${SYSLIBDIR}; fi && \
						${MAKE} -C $$s -f ../../$${s##*/}.build.mk all; \
					} || { echo "Building $$s failed" ; exit 1; };\
				fi; \
			done;; \
		primitives) \
			for p in primitive/*; \
			do \
				if [ -d "$$p" ]; \
				then \
					{ \
						${MAKE} -f primitive.mk $${p##*/}; \
					} || { exit 1; }\
				fi; \
			done;; \
		composites) \
			for c in composite/*; \
			do \
				if [ -d "$$c" ]; \
				then \
					{ \
						${MAKE} -f composite.mk $${p##*/}; \
					} || { exit 1; }\
				fi; \
			done;; \
		sdfsrc|sdfasm|sdfrun|common) \
			{ \
				echo "Building $@" && \
				if [ ! -d "${BUILDROOTDIR}/system/$@" ]; then mkdir -p ${BUILDROOTDIR}/system/$@; fi && \
				if [ ! -d "${BINDIR}" ]; then mkdir -p ${BINDIR}; fi && \
				if [ ! -d "${SYSLIBDIR}" ]; then mkdir -p ${SYSLIBDIR}; fi && \
				${MAKE} -C system/$@ -f ../../$@.build.mk all; \
			} || { echo "Building $@ failed" ; exit 1; } ;;\
		clean) \
			for s in system/*; \
			do \
				rm -f ${BUILDROOTDIR}/$$s/*; \
			done; \
			${RM} ${BINDIR}/*; \
			${RM} ${SYSLIBDIR}/*; \
			for c in composite/*; \
			do \
				if [ -d "$$c" ]; \
				then \
					${RM} ${BUILDROOTDIR}/actor/$${c##*/}/*; \
				fi; \
			done; \
			for p in primitive/*; \
			do \
				if [ -d "$$p" ]; \
				then \
					${RM} ${BUILDROOTDIR}/actor/$${p##*/}/*; \
				fi; \
			done; \
			${RM} ${ACTORDIR}/*; \
			${RM} ${CONTEXTDIR}/*;; \
		clean_primitives) \
			for p in primitive/*; \
			do \
				if [ -d "$$p" ]; \
				then \
					${RM} ${BUILDROOTDIR}/actor/$${p##*/}/*; \
					${RM} ${ACTORDIR}/$${p##*/}.sdf.so; \
					${RM} ${CONTEXTDIR}/$${p##*/}.sdf.ctx; \
				fi; \
			done;; \
		clean_composites) \
			for c in composite/*; \
			do \
				if [ -d "$$c" ]; \
				then \
					${RM} ${BUILDROOTDIR}/actor/$${c##*/}/*; \
					${RM} ${ACTORDIR}/$${c##*/}.sdf.bin; \
					${RM} ${CONTEXTDIR}/$${c##*/}.sdf.ctx; \
				fi; \
			done;; \
		*) \
			if [ -d primitive/$@ ]; \
			then \
				{ ${MAKE} -f primitive.mk $@ ; } || { exit 1; }; \
			else \
				if [ -d composite/$@ ]; \
				then \
					{ ${MAKE} -f composite.mk $@ ; } || { exit 1; }; \
				else \
					{ echo "Component $@ do not exist"; exit 1; }; \
				fi; \
			fi;; \
	esac



