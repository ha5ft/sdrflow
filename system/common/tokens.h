/*******************************************************************************
 * 							Common tokens
 * *****************************************************************************
 * 	Filename:		tokens.h
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

#ifndef TOKENS_H
#define TOKENS_H

#define	TOKEN_COUNT			22

#define	TOKEN_USE			0
#define	TOKEN_PRIMITIVE		1
#define	TOKEN_COMPOSITE		2
#define	TOKEN_IMPL			3
#define	TOKEN_STREAM		4
#define	TOKEN_VAR			5
#define	TOKEN_CONST			6
#define	TOKEN_INPUT			7
#define	TOKEN_OUTPUT		8
#define	TOKEN_PARAM			9
#define	TOKEN_STATE			10
#define	TOKEN_TEMP			11
#define	TOKEN_ACTORS		12
#define	TOKEN_SCHEDULE		13
#define	TOKEN_DO			14
#define	TOKEN_LOOP			15
#define	TOKEN_END			16
#define	TOKEN_SIGNALS		17
#define	TOKEN_TOPOLOGY		18
#define	TOKEN_CONTEXT		19
#define	TOKEN_MANUAL		20
#define	TOKEN_AUTO			21

#define	TOKEN_USE_STR		"use"
#define	TOKEN_PRIMITIVE_STR	"primitive"
#define	TOKEN_COMPOSITE_STR	"composite"
#define	TOKEN_IMPL_STR		"implementation"
#define	TOKEN_STREAM_STR	"stream"
#define	TOKEN_VAR_STR		"var"
#define	TOKEN_CONST_STR		"const"
#define	TOKEN_INPUT_STR		"input"
#define	TOKEN_OUTPUT_STR	"output"
#define	TOKEN_PARAM_STR		"parameter"
#define	TOKEN_STATE_STR		"state"
#define	TOKEN_TEMP_STR		"temp"
#define	TOKEN_ACTORS_STR	"actors"
#define	TOKEN_SCHEDULE_STR	"schedule"
#define	TOKEN_DO_STR		"do"
#define	TOKEN_LOOP_STR		"loop"
#define	TOKEN_END_STR		"end"
#define	TOKEN_SIGNALS_STR	"signals"
#define	TOKEN_TOPOLOGY_STR	"topology"
#define	TOKEN_CONTEXT_STR	"context"
#define	TOKEN_MANUAL_STR	"manual"
#define	TOKEN_AUTO_STR		"auto"

#endif

