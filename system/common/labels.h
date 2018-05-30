/*******************************************************************************
 * 							Common labels
 * *****************************************************************************
 * 	Filename:		labels.h
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

#ifndef LABELS_H
#define LABELS_H

#define	LABEL_COUNT			12

#define	LABEL_CODE_OFFSET	0
#define	LABEL_DATA_OFFSET	1
#define	LABEL_COMP_SIZE		2
#define	LABEL_COMP_NAME		3
#define	LABEL_COMP_VERS		4
#define	LABEL_CODE_INIT		5
#define	LABEL_CODE_FIRE		6
#define	LABEL_CODE_CLEANUP	7
#define	LABEL_META_LOAD		8
#define	LABEL_META_MAKE		9
#define	LABEL_META_DELETE	10
#define LABEL_CONTEXT_SIZE	11

#define	LABEL_CODE_OFFS_STR		".code.offset"
#define	LABEL_DATA_OFFS_STR		".data.offset"
#define	LABEL_COMP_SIZE_STR		".size"
#define	LABEL_COMP_NAME_STR		".name"
#define	LABEL_COMP_VERS_STR		".version"
#define	LABEL_CODE_INIT_STR		".init"
#define	LABEL_CODE_FIRE_STR		".fire"
#define	LABEL_CODE_CLEANUP_STR	".clean"
#define	LABEL_META_LOAD_STR		".load"
#define	LABEL_META_MAKE_STR		".make"
#define	LABEL_META_DELETE_STR	".delete"
#define LABEL_CONTEXT_SIZE_STR	".ctx.size"

#endif

