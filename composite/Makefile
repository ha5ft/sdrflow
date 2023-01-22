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

DIRECTORIES = A4 FreqResp M1 M2 Mixer OverlapSave CwTx
# Second step because of dependencies
DIRECTORIES += Filter CwRx
# Third step because of dependencies
DIRECTORIES += Channel Demo

all:

clean:
	for subdir in $(DIRECTORIES); do rm -f $$subdir/$$subdir.sdf.ctx $$subdir/$$subdir.sdf.asm $$subdir/$$subdir.sdf.bin; done

install:
	for subdir in $(DIRECTORIES); do \
	    ../$(BIN_DIR)/sdfsrc -C ../$(CONTEXT_DIR)/ -i $${subdir}/$${subdir} || exit; \
	    cp -p $$subdir/$$subdir.sdf.ctx ../$(CONTEXT_DIR); \
	    ../$(BIN_DIR)/sdfasm -i $${subdir}/$${subdir} || exit; \
	    cp -p $$subdir/$$subdir.sdf.bin ../$(ACTOR_DIR); \
	done

uninstall:
	@for subdir in $(DIRECTORIES); do rm -f ../$(CONTEXT_DIR)/$$subdir.sdf.ctx ../$(ACTOR_DIR)/$$subdir.sdf.bin; done
