/*******************************************************************************
 * 							Common obj_format
 * *****************************************************************************
 * 	Filename:		obj_format.h
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

#include	<stddef.h>
#include	<stdint.h>
#include	<stdlib.h>

#ifndef OBJ_FORMAT_H
#define OBJ_FORMAT_H

//#define PLATFORM_COUNT		3
#define	ENTRY_COUNT			3
#define	META_ENTRY_COUNT	3
#define CATALOG_SIG_LENGTH	3
#define	CATALOG_SIZE		32

#define	SIGNATURE_CHAR_1	's'
#define	SIGNATURE_CHAR_2	'd'
#define	SIGNATURE_CHAR_3	'f'

//#define	PLATFORM_UNDEF		(-1)
//#define	PLATFORM_16			0
//#define	PLATFORM_32			1
//#define	PLATFORM_64			2

#define ENTRY_INIT			1
#define	ENTRY_FIRE			0
#define	ENTRY_CLEANUP		2
#define ENTRY_UNDEF			(-1)

#define	META_ENTRY_LOAD		0
#define	META_ENTRY_MAKE		1
#define	META_ENTRY_DELETE	2
#define	META_ENTRY_UNDEF	(-1)
#define MAX_NAME_LENGTH     63

// ============================================================================
// Catalog is the header structure of the object code.
// It has the same format for all the platforms.
// if the object code is in hex format, then the first record contains the
// catalog. In this case the catalog is described by a single record.
// The code_offset and data_offset are relative to the beginning of the object
// code, including the catalog. These offsets point to the beginning of the
// corresponding segment header.
// The meta segment begins with the catalog structure.
// The name_offset and version_offset are relative to the beginning of the
// meta segment (which is equivalent to the beginning of the object code.
// The offsets of the segments are the following:
//		meta segment : 0
//		code segment : code_offset field of the catalog
//		data segment : data_offset field of the catalog
// The size of the segments:
//		meta segment : code_offset
//		code segment : data_offset - code_offset
//		data segment : size - data_offset
// The name_offset field points to a null terminated string. This string is
//	the name of the code.
// The version offset points to an uint32 value.
// ============================================================================

struct _catalog
{
	int8_t		signature[CATALOG_SIG_LENGTH];
	int8_t		platform;
	int32_t		name_offset;
	int32_t		version_offset;
	int32_t		size;
	int32_t		code_offset;
	int32_t		data_offset;
	int32_t		context_size;
	int32_t		reserved_2;
}__attribute__((packed));

typedef struct _catalog	catalog_t;

#define META_SEG_OFFSET	sizeof(struct _catalog);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  +	The following structure definitions should be used only in the runtime
  +	systems, becouse the actual sizes of some integer types in the structures
  + are different for different platforms. So an assembler or compiler, which
  + should generate code for every platforms can not use this structures,
  + becouse they will use the sizes of the platform on which the assembler or
  + compiler is running and not the sizes valid on the target platform.
  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

// ============================================================================
// The following structures describe the segment headers.
// The meta header contains the offsets of the beginnings of the 3 meta scripts.
// The code header contains the offsets of the beginnings of the code for the
//	3 executable functions of the composite.
// The data header contains the pointers to the beginnings of the code for
// the 3 executable functions of the composite. In the object file produced by
// the assembler they have NULL value. The runtime system assigns value to
// them using the starting address of the object code in the memory and
// the offsets in the code header. For example:
//		fire = (starting address) + fire_offset
// Storing these pointers in the data segment in the virtual machine we do not
// have to have a pointer to the code segment. We do not have to save this
// pointer when executing a subcomposite function. And finally we do not have to
// compute the new instruction pointer value at every subcomposit function call.
// The offsets are relative to the beginning of the corresponding segments.
// ============================================================================

struct _meta_header
{
	int		load_offset;
	int		make_offset;
	int		delete_offset;
}__attribute__((packed));

struct _code_header
{
	int		fire_offset;
	int		init_offset;
	int		cleanup_offset;
}__attribute__((packed));

struct _data_header
{
//	int32_t		*fire;
//	int32_t		*init;
//	int32_t		*cleanup;
	void		*composite;
    catalog_t   *catalog;
	char		*name;
    int32_t     flag;
    int32_t     reserved;
}__attribute__((packed));

// ============================================================================
// entry_t is the typedef of a pointer to the  entry functions of a primitive 
// actor. A primitive actor has 3 entry points. These are:
//		void	initialize(void *context);
//		void	fire(void *context);
//		void	cleanup(void *context);
// The formal parameter 'context' points to a structure, which has the following
// general form:
//		struct context
//		 {
//			void	*input_1;
//			....
//			void	*input_n;
//			void	*output_1;
//			....
//			void	*output_m;
//			void	*parameter_1;
//			....
//			void	*parameter_k;
//		}
// The virtual machine of the runtime system calls these entry functions
// using function pointers.
// ============================================================================

//typedef	void (*entry_t)(void *context);

// ============================================================================
// The followings are the major data structures in the data segment
// struct _buf_instance contains a pointer to the given buffer, which has been
// allocated in the data segment.
// In struct _prim_instance
//		'table' field is a pointer to a table of the entry pointers of the
//			primitive actor.
// This table has the folloving structure:
//		void		primitive_fire(void *context);
//		void		primitive_initialize(void *context);
//		void		primitive_cleanup(void *context);
//		entry_t		entry_table[3] =
//					{
//						[0] = primitive_fire,
//						[0] = primitive_initialize,
//						[0] = primitive_cleanup
//					};
//		struct _prim_instance primitive_instance =
//				{
//					.table = &entry_table[0];
//				};
// The 'table' field is followed by an array of void pointers. These pointers
// represet the context structure for the primitive.
// We call the primitive entries in the following way:
//		void **ptr = (void **)&primitive_instance;
//		(**(*((entry_t **)ptr) + ENTRY_xxx))((void *)(ptr + 1));
// In struct _comp_instance
//		the 'data' field points to the beginning of the composite's data segment.
// The 'data' field is followed by an array of void pointers. These pointers
// represet the context structure for the composite actor.
// ============================================================================

struct _buf_instance
{
	void	*buf;			// points to the memory allocated in the data
							// segment for the buffer.
}__attribute__((packed));

struct	_prim_instance
{
	void	**table;			// pointers to a table containing the pointers to
							// the primitive's externally callable functions.
	void	*self;
	void	*context[];		// an array containing the pointers to the primitive's
							// input, output and parameter buffers.
}__attribute__((packed));

struct	_comp_instance
{
	int32_t	**table;			// points to the entry table containing the pointers
							// to the entry points in the code segment
	void	*data;			// points to the beginning of the data segment of
							// the instance
	void	*context[];		// an array containing the pointers to the context's
							// input, output and parameter buffers.
}__attribute__((packed));

// ============================================================================
// In the following we specify the structure of the instructions used in the
// meta segment.
// Each instruction consists of an array of integer values. On every platform
// we use the platform's int type for the values. So for 64 and 32 bit platforms
// the instructions consist of 32 bit values. For the 16 bit platforms the
// instructions consist of 16 bit values.
// The first value in the array allways the instruction code.
// Following the instruction code we have the operands of the instructions.
// Currently we support up to 4 operands.
// The operands could be:
//		literals of int type
//		offsets into the meta, code or data segments, represented by an int type
// ============================================================================
 
struct	_meta_ldprim
{
	int	instruction; 	// instruction code
	int	name;			// offset of the primitive's name string (meta seg rel.)
/*	int	inst_name;*/		// offset of the instance's name string (meta seg rel.)
}__attribute__((packed));

struct	_meta_ldcomp
{
	int	instruction;	// instruction code
	int	name;			// offset of the composit's name string (meta seg rel.)
/*	int	inst_name;*/		// offset of the instance's name string (meta seg rel.)
}__attribute__((packed));

struct _meta_mkbuf
{
	int	instruction;	// instruction code
//	int	type;			// value type defined by the VALUE_TYPE_xxx constants
//	int	count;			// max count of the values in the buffer
						// The buffer size in bytes = (size of the type) * count
	int	size;			// Size of the buffer in bytes
	int	name;			// offset of the buffer's name string (meta seg rel.)
	int	instance;		// offset of the buffers's instance (data seg rel.)
}__attribute__((packed));

struct _meta_mkcomp
{
	int	instruction;	// instruction code
	int comp_name;		// offset of the composite's name string (meta seg rel.)
	int inst_name;		// offset of the instance's name string (meta seg rel.)
	int	instance;		// offset of the instance's structure (data seg rel.)
}__attribute__((packed));

struct _meta_mkprim
{
	int	instruction;	// instruction code
	int prim_name;		// offset of the primitive's name string (meta seg rel.)
	int inst_name;		// offset of the instance's name string (meta seg rel.)
	int	instance;		// offset of the instance's structure (data seg rel.)
}__attribute__((packed));

struct	_meta_delbuf
{
	int	instruction;	// instruction code
	int	instance;	// offset of the instance's structure (data seg rel.)
}__attribute__((packed));

struct _meta_exit
{
	int	instuction;		// instruction code
}__attribute__((packed));

// ============================================================================
// In the following we specify the structure of the instructions used in the
// code segment.
// Each instruction consists of an array of integer values. On every platform
// we use the platform's int type for the values. So for 64 and 32 bit platforms
// the instructions consist of 32 bit values. For the 16 bit platforms the
// instructions consist of 16 bit values.
// The first value in the array allways the instruction code.
// Following the instruction code we have the operands of the instructions.
// Currently we support up to 4 operands.
// The operands could be:
//		literals of int type
//		offsets into the meta, code or data segments, represented by an int type
// ============================================================================
 
struct _exit_instr
{
	int	instruction;	// instruction code
}__attribute__((packed));

struct _pause_instr
{
	int	instruction;	// instruction code
}__attribute__((packed));

struct _end_instr
{
	int	instruction;	// instruction code
}__attribute__((packed));

struct _endcycle_instr
{
	int	instruction;	// instruction code
}__attribute__((packed));

struct _break_instr
{
	int	instruction;	// instruction code
}__attribute__((packed));

struct	_goto_instr
{
	int	instruction;	// instruction code
	int	addr;			// relative offset from here to the target instruction
						// of the loop.
}__attribute__((packed));

struct _do_instr
{
	int	instruction;	// instruction code
	int	count;			// loop count
}__attribute__((packed));

struct	_loop_instr
{
	int	instruction;	// instruction code
	int	addr;			// relative offset from here to the first instruction
						// of the loop.
}__attribute__((packed));

struct _call_instr		// common structure for init, fire and cleanup 
						// instructions
{
	int	instruction;	// instruction code
	int	instance;		// offset of the instance structure in the data segment
}__attribute__((packed));

struct _ret_instr
{
	int	instruction;	// instruction code
}__attribute__((packed));

struct _call_prim_instr	// common structure for prim_init, prim_fire and 
						// prim_cleanup instructions
{
	int	instruction;	// instruction code
	int	instance;		// offset of the instance structure in the data segment
}__attribute__((packed));

struct _cp_var_instr	// common structure for all copy instructions
						// The type of the source and destination is
						// instruction specific. It copies a single variable.
{
	int	instruction;	// instruction code
	int	dest;			// offset of the destination in the data segment
	int	source;			// offset of the destination in the instruction
						// specific segment
}__attribute__((packed));

struct	_add_lint_instr	// commont structure for all add literal integer
						// instructions. The type of the destination is
						// instruction specific.
{
	int	instruction;	// instruction code
	int	dest;			// offset of the destination in the data segment
	int	literal;		// the literal, which should be added to the destination
}__attribute__((packed));


struct	_entry_table
{
	int	fire_offset;
	int	init_offset;
	int	cleanup_offset;
}__attribute__((packed));

struct	_meta_entry_table
{
	int	load_offset;
	int	make_offset;
	int	delete_offset;
}__attribute__((packed));

struct	_data_entry_table
{
	int32_t	*fire;
	int32_t	*init;
    int32_t *cleanup;
}__attribute__((packed));


typedef struct 	_buf_instance 		buf_instance_t;
typedef struct	_prim_instance		prim_instance_t;
typedef struct	_comp_instance		comp_instance_t;
typedef struct	_meta_ldprim		meta_ldprim_t;
typedef struct	_meta_ldcomp		meta_ldcomp_t;
typedef struct 	_meta_mkbuf			meta_mkbuf_t;
typedef struct 	_meta_mkcomp		meta_mkcomp_t;
typedef struct 	_meta_mkprim		meta_mkprim_t;
typedef struct 	_meta_exit			meta_exit_t;
typedef struct 	_exit_instr			exit_instr_t;
typedef struct 	_pause_instr		pause_instr_t;
typedef struct 	_end_instr			end_instr_t;
typedef struct 	_endcycle_instr		endcycle_instr_t;
typedef struct 	_break_instr		break_instr_t;
typedef struct	_goto_instr			goto_instr_t;
typedef struct 	_do_instr			do_instr_t;
typedef struct	_loop_instr			loop_instr_t;
typedef struct	_call_instr			call_instr_t;
typedef struct 	_ret_instr			ret_instr_t;
typedef struct 	_call_prim_instr	call_prim_instr_t;
typedef struct 	_cp_var_instr		cp_var_instr_t;
typedef struct	_add_lint_instr		add_lint_instr_t;
typedef struct	_entry_table		entry_table_t;
typedef struct	_meta_entry_table	meta_entry_table_t;
typedef	struct	_data_entry_table	data_entry_table_t;
typedef	struct	_meta_delbuf		meta_delbuf_t;

//typedef struct _catalog	catalog_t;
typedef struct _meta_header	meta_header_t;
typedef struct _code_header	code_header_t;
typedef struct _data_header	data_header_t;

#endif

