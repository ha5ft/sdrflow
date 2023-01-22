#*******************************************************************************
#* 							Makefile for sdrflow
#* *****************************************************************************
#* 	Filename:		Makefile
#* 	Platform:		ubuntu 16.04 64 bit
#* 	Author:			Copyright (C) Krüpl Zsolt & Selmeczi János, original version
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
#*      Krüpl Zsolt		23-01-2023	Rewriting, cleaning
#*
#******************************************************************************/

export CC = gcc
export CFLAGS = -Wall -O2
export SDRFLOW_LIB = ../libsdrflow.a

SYSTEM_MODULES = common sdfasm sdfsrc sdfrun

export BIN_DIR = bin
export ACTOR_DIR = actor
export CONTEXT_DIR = context

all:
	@for mod in ${SYSTEM_MODULES}; do make -C system/$$mod; done
	@echo
	@make -C primitive

clean:
	@for mod in ${SYSTEM_MODULES}; do make -C system/$$mod clean; done
	@make -C primitive clean
	@make -C composite clean

install:
	mkdir -p $(BIN_DIR)
	@for mod in ${SYSTEM_MODULES}; do make -C system/$$mod install; done

	mkdir -p $(ACTOR_DIR)
	mkdir -p $(CONTEXT_DIR)
	@make -C primitive install
	@make -C composite install

uninstall:
	@for mod in ${SYSTEM_MODULES}; do make -C system/$$mod uninstall; done
	rmdir $(BIN_DIR) || true

	@make -C primitive uninstall
	@make -C composite uninstall
	rmdir -p $(ACTOR_DIR) $(CONTEXT_DIR) || true

distclean:
	make clean
	make uninstall
