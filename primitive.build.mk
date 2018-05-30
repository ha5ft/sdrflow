#*******************************************************************************
#* 							Makefile for sdrflow primitives
#* *****************************************************************************
#* 	Filename:		primitive.build.mk
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
MAKE=make
CC=gcc
endif

BUILDROOT=../../build
ACTORDIR=../../actor
INCLUDEDIR=../../include
CONTEXTDIR=../../context

CFLAGS=-c -Wall -Ofast -funroll-all-loops -fPIC -march=native -DSDF_PLATFORM=0
LIBFLAGS=rcs
LINKFLAGS=-Wall -shared
LIBFLAGS=-lm -lpulse -lpulse-simple -lfftw3


%.actor: $(ACTORDIR)/%.sdf.so

%.context: $(CONTEXTDIR)/%.sdf.ctx

$(ACTORDIR)/%.sdf.so: %.c %.h ${INCLUDEDIR}/primitive_interface.h
	@${RM} ${BUILDROOT}/actor/$(basename $(basename $(notdir $@)))/*
	@${CC} ${CFLAGS} -o ${BUILDROOT}/actor/$(basename $(basename $(notdir $@)))/$(basename $(basename $(notdir $@))).o $<
	@${LINK} ${LINKFLAGS} -o $@ ${BUILDROOT}/actor/$(basename $(basename $(notdir $@)))/$(basename $(basename $(notdir $@))).o ${LIBFLAGS}
	@${CP} ../../primitive/$(basename $(basename $(notdir $@)))/$(basename $(basename $(notdir $@))).sdf.ctx ${BUILDROOT}/actor/$(basename $(basename $(notdir $@)))
	@${CP} ${BUILDROOT}/actor/$(basename $(basename $(notdir $@)))/$(basename $(basename $(notdir $@))).sdf.ctx ${CONTEXTDIR}

${CONTEXTDIR}/%.sdf.ctx:
	@${CP} ${BUILDROOT}/actor/$(basename $(basename $(notdir $@)))/$(basename $(basename $(notdir $@))).sdf.ctx ${CONTEXTDIR}


