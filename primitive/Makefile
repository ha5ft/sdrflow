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

CFLAGS  = -Wall -Ofast -funroll-all-loops -march=native
CFLAGS += -fPIC -DSDF_PLATFORM=0
LDFLAGS = -shared -s
LDFLAGS+= -lm -lpulse -lpulse-simple -lfftw3
DIRECTORIES = `find * -type d`

all:
	@echo CFLAGS: $(CFLAGS)
	@for subdir in $(DIRECTORIES); do \
	    cd $$subdir; \
	    echo -- $$subdir --; \
	    $(CC) $${subdir}.c $(CFLAGS) $(LDFLAGS) -o $${subdir}.sdf.so; \
	    cd ../$$subdir/..; \
	done

clean:
	@for subdir in $(DIRECTORIES); do \
	    cd $$subdir; \
	    echo -- $$subdir --; \
	    rm -f $$subdir.sdf.so; \
	    cd ../$$subdir/..; \
	done

install:
	for subdir in $(DIRECTORIES); do \
	    cp -p $$subdir/$$subdir.sdf.so ../$(ACTOR_DIR); \
	    cp -p $$subdir/$$subdir.sdf.ctx ../$(CONTEXT_DIR); \
	done

uninstall:
	@for subdir in $(DIRECTORIES); do rm -f ../$(ACTOR_DIR)/$$subdir.sdf.so ../$(CONTEXT_DIR)/$$subdir.sdf.ctx; done
