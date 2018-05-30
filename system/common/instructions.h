/*******************************************************************************
 * 							Common instaructions
 * *****************************************************************************
 * 	Filename:		instructions.h
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

#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#define INSTR_COUNT			25

// The instruction code and all the operans are integers on every platforms
// instruction size = INSTR_CODE_SIZE_nn + (INSTR_OP_SIZE_nn * xxx_OP_COUNT)

#define	INSTR_CODE_SIZE_16	2	//in bytes
#define	INSTR_CODE_SIZE_32	4
#define	INSTR_CODE_SIZE_64	4

#define	INSTR_OP_SIZE_16	2	//in bytes
#define	INSTR_OP_SIZE_32	4
#define	INSTR_OP_SIZE_64	4

#define	EXIT_INSTR			0
#define	PAUSE_INSTR			1
#define END_INSTR			2
#define	ENDCYCLE_INSTR		3
#define	BREAK_INSTR			4
#define	GOTO_INSTR			5
#define	DO_INSTR			6
#define	LOOP_INSTR			7
#define	FIRE_INSTR			8
#define	INIT_INSTR			9
#define	CLEANUP_INSTR		10
#define	RET_INSTR			11
#define	PRIM_FIRE_INSTR		12
#define	PRIM_INIT_INSTR		13
#define	PRIM_CLEANUP_INSTR	14
#define	CPPTR_INSTR			15
#define	INCPTR_INSTR		16
#define	CPCTXPTR_INSTR		17
#define	META_EXIT_INSTR			18
#define META_LDCOMP_INSTR		19
#define META_LDPRIM_INSTR		20
#define META_MKBUF_INSTR		21
#define META_MKCOMPINST_INSTR	22
#define META_MKPRIMINST_INSTR	23
#define META_DELBUF_INSTR		24

#define	EXIT_STR			"exit"
#define	PAUSE_STR			"pause"
#define	END_STR				"end"
#define	ENDCYCLE_STR		"end.cycle"
#define	BREAK_STR			"break"
#define	GOTO_STR			"goto"
#define	DO_STR				"do"
#define	LOOP_STR			"loop"
#define	FIRE_STR			"fire.comp"
#define	INIT_STR			"init.comp"
#define	CLEANUP_STR			"cleanup.comp"
#define	RET_STR				"ret"
#define	PRIM_FIRE_STR		"fire.prim"
#define	PRIM_INIT_STR		"init.prim"
#define	PRIM_CLEANUP_STR	"cleanup.prim"
#define	CPPTR_STR			"cp.ptr"
#define	INCPTR_STR			"inc.ptr"
#define	CPCTXPTR_STR		"cp.ctx.ptr"
#define	META_EXIT_STR		"meta.exit"
#define	META_LDCOMP_STR		"ld.comp"
#define	META_LDPRIM_STR		"ld.prim"
#define	META_MKBUF_STR		"mk.buffer"
#define	META_MKCOMPINST_STR	"mk.comp.inst"
#define	META_MKPRIMINST_STR	"mk.prim.inst"
#define	META_DELBUF_STR		"del.buf"

#define	EXIT_OP_COUNT			0
#define	PAUSE_OP_COUNT			0
#define	END_OP_COUNT			0
#define	ENDCYCLE_OP_COUNT		0
#define	BREAK_OP_COUNT			0
#define	GOTO_OP_COUNT			1
#define	DO_OP_COUNT				1
#define	LOOP_OP_COUNT			1
#define	FIRE_OP_COUNT			1
#define	INIT_OP_COUNT			1
#define	CLEANUP_OP_COUNT		1
#define	RET_OP_COUNT			0
#define	PRIM_FIRE_OP_COUNT		1
#define	PRIM_INIT_OP_COUNT		1
#define	PRIM_CLEANUP_OP_COUNT	1
#define	CPPTR_OP_COUNT			2
#define	INCPTR_OP_COUNT		2
#define	CPCTXPTR_OP_COUNT		2
#define META_EXIT_OP_COUNT		0
#define META_LDCOMP_OP_COUNT	1//	2
#define META_LDPRIM_OP_COUNT	1//2
#define META_MKBUF_OP_COUNT		3
#define META_MKCOMPINST_OP_COUNT	3
#define META_MKPRIMINST_OP_COUNT	3
#define META_DELBUF_OP_COUNT	1

#endif

