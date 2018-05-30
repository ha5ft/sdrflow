/*******************************************************************************
 * 							Common segments
 * *****************************************************************************
 * 	Filename:		segments.h
 * 	Platform:		ubuntu 16.04 64 bit
 * 	Author:			Copyright (C) Selmeczi János, original version
 *******************************************************************************
 *							Licensing
 *******************************************************************************
 *  This file is part of sdrflow.
 *
 *  Sdrflow is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Sdrflow is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************
 *							Revision history
 *******************************************************************************
 *	Author				Date		Comment
 *******************************************************************************
 *	Selmeczi János		23-04-2018	Original version
 *
 ******************************************************************************/

#ifndef SEGMENTS_H
#define SEGMENTS_H

#define SEGDEF_COUNT		5
#define SEGMENT_COUNT		(SEGDEF_COUNT - 1)

#define META_SEG			0
#define CODE_SEG			1
#define	DATA_SEG			2
#define	CONTEXT_SEG			3
#define	END_SEG				4
#define UNDEF_SEG			(-1)

#define META_SEG_STR		".meta"
#define CODE_SEG_STR		".code"
#define	DATA_SEG_STR		".data"
#define	CONTEXT_SEG_STR		".context"
#define	END_SEG_STR			".endseg"

#endif

