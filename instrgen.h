#ifndef VMACHINE_INSTR_GEN
#	define VMACHINE_INSTR_GEN

#include "vmachine.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline size_t vmachine_instr_gen(
	uint32_t instrbuf[const restrict],
	enum VMachineInstrSet op,
	...
) {
	if( op >= MaxOps || op < halt ) {
		return 0;
	}
	
	size_t cells = sizeof(InstrFunc*) / sizeof *instrbuf;
	if( instrbuf != NULL ) {
		union VMachineVal faddr = { .uintptr = ( uintptr_t )(opcode_func[op]) };
		instrbuf[0] = faddr.uint32a[0];
		instrbuf[1] = faddr.uint32a[1];
	}
	
	va_list ap; va_start(ap, op);
	switch( op ) {
		/// 0 operands:
		case halt: case nop: case ret: case savelink: case remitlink: {
			break;
		}
		
		/// 2 unsigned bytes + 1~2 byte.
		case add:  case addi:
		case sub:  case subi:
		case mul:  case muli:
		case _div: case divi:
		case mod:  case modi:
		case _and: case _andi:
		case _or:  case _ori:
		case _xor: case _xori:
		case sll:  case slli:
		case srl:  case srli:
		case sra:  case srai:
		case slt:  case slti:
		case sltu: case sltui:
		case jeq:  case jne:
		case jlt:  case jltu:
		case jge:  case jgeu:
		{
			uint32_t oper1 = 0;
			const int a = va_arg(ap, int);
			const int b = va_arg(ap, int);
			const int c = va_arg(ap, int);
			oper1 = (a & 0xff) | ((b & 0xff) << 8) | ((c & 0xffFF) << 16);
			if( instrbuf != NULL ) {
				instrbuf[2] = oper1;
			}
			cells += sizeof oper1 / sizeof *instrbuf;
			break;
		}
		
		/// single signed 4-byte operand.
		case alloc: case jmp: case jal: case enter:
		{
			const uint32_t oper1 = va_arg(ap, uint32_t);
			if( instrbuf != NULL ) {
				instrbuf[2] = oper1;
			}
			cells += sizeof oper1 / sizeof *instrbuf;
			break;
		}
		default:
	}
	return cells;
}

#ifdef __cplusplus
}
#endif

#endif /** VMACHINE_INSTR_GEN */
