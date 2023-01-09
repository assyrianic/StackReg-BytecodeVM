#include "vmachine.h"
#include <time.h>

//#define VMACHINE_DBG

#ifdef VMACHINE_DBG
static uint8_t limit=0;
enum{ LIMIT = 0xff };
static FILE *vm_output;
static void prep_vm_output() {
	vm_output = fopen("vmachine_output.txt", "w+");
}
static void close_vm_output() {
	fclose(vm_output);
	vm_output = NULL;
}
#	define VMACHINE_OPCODE    do { if( limit++ < LIMIT ) { fprintf(vm_output, "\n%s\n", __func__); sleep(1); } } while( 0 );
#	define VMACHINE_3OPU	  do { if( limit < LIMIT ) { fprintf(vm_output, "a: '%u' | b: '%u' | c: '%u'\n", a, b, c); } } while( 0 );
#	define VMACHINE_3OPI	  do { if( limit < LIMIT ) { fprintf(vm_output, "a: '%u' | b: '%u' | c: '%i'\n", a, b, c); } } while( 0 );
#	define VMACHINE_OFFS	  do { if( limit < LIMIT ) { fprintf(vm_output, "offset: '%" PRIdFAST32 "'\n\n", offset); } } while( 0 );
#	define VMACHINE_3REG	  do { if( limit < LIMIT ) { fprintf(vm_output, "rsp[a]: '%" PRIu64 "' | rsp[b]: '%" PRIu64 "' | rsp[c]: '%" PRIu64 "'\n", rsp[a].uint64, rsp[b].uint64, rsp[c].uint64); } } while( 0 );
#	define VMACHINE_2REG	  do { if( limit < LIMIT ) { fprintf(vm_output, "rsp[a]: '%" PRIu64 "' | rsp[b]: '%" PRIu64 "'\n", rsp[a].uint64, rsp[b].uint64); } } while( 0 );
/**
#	define VMACHINE_OPCODE	do { if( limit++ < LIMIT ) { printf("\n%s\n", __func__); sleep(1); } } while( 0 );
#	define VMACHINE_3OPU	  do { if( limit < LIMIT ) { printf("a: '%u' | b: '%u' | c: '%u'\n", a, b, c); } } while( 0 );
#	define VMACHINE_3OPI	  do { if( limit < LIMIT ) { printf("a: '%u' | b: '%u' | c: '%i'\n", a, b, c); } } while( 0 );
#	define VMACHINE_OFFS	  do { if( limit < LIMIT ) { printf("offset: '%" PRIdFAST32 "'\n\n", offset); } } while( 0 );
#	define VMACHINE_3REG	  do { if( limit < LIMIT ) { printf("rsp[a]: '%" PRIu64 "' | rsp[b]: '%" PRIu64 "' | rsp[c]: '%" PRIu64 "'\n", rsp[a].uint64, rsp[b].uint64, rsp[c].uint64); } } while( 0 );
#	define VMACHINE_2REG	  do { if( limit < LIMIT ) { printf("rsp[a]: '%" PRIu64 "' | rsp[b]: '%" PRIu64 "'\n", rsp[a].uint64, rsp[b].uint64); } } while( 0 );
 */
#else
#	define VMACHINE_OPCODE
#	define VMACHINE_3OPU
#	define VMACHINE_3OPI
#	define VMACHINE_OFFS
#	define VMACHINE_3REG
#	define VMACHINE_2REG
#endif

void vmachine_run(
	const size_t          stksize,
	union VMachineVal     stkmem[],
	const size_t          callsize,
	uintptr_t             callmem[],
	const uint32_t        ip[],
	struct VMachineState *state
) {
#ifdef VMACHINE_DBG
	prep_vm_output();
#endif
	state->opstk        = ( uintptr_t )(stkmem);
	state->callstk      = ( uintptr_t )(callmem);
	state->opstk_size   = stksize;
	state->callstk_size = callsize;
	uintptr_t sp = ( uintptr_t )(&stkmem[stksize - 1]);
	uintptr_t cp = ( uintptr_t )(&callmem[callsize - 1]);
	uintptr_t lr = ( uintptr_t )(NULL);
	union VMachinePtr pc = { .uint32 = &ip[0] };
	//printf("stksize: '%zu' | stkmem: '%p' | callmem: '%p' | ip: '%p' | state: '%p'\n", stksize, &stkmem[0], &callmem[0], &ip[0], state);
	//printf("sp: '%zu' | cp: '%zu' | lr: '%zu'\n", (size_t)sp, (size_t)cp, (size_t)lr);
	VMACHINE_OPCODE
	//const clock_t start = clock();
	vmachine_exec(INSTR_ARGS);
	//const clock_t end = clock();
	//const double elapsed = ( double )(end - start) / ( double )(CLOCKS_PER_SEC);
	//printf("Done Executing | elapsed: '%f' milliseconds\n", elapsed * 1000.0);
	
#ifdef VMACHINE_DBG
	close_vm_output();
#endif
}
/*
void vmachine_printstack(const struct VMachineState *const state, const uintptr_t sp) {
	size_t i = 0;
	const union VMachineVal *const top = ( const union VMachineVal* )(state->opstk) + state->opstk_size;
	for(
		const union VMachineVal *head = ( const union VMachineVal* )(sp);
		head < top; head++
	) {
		printf("opstack(size: %zu)[%zu] = %" PRIu64 "\n", state->opstk_size, i++, head->uint64);
	}
}
*/

void vmachine_exec(INSTR_PARAMS) {
	VMACHINE_OPCODE
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

void func_halt(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const union VMachineVal *const restrict rsp = ( const union VMachineVal* )(sp);
	state->saved_ra = rsp[0];
}
void func_nop(INSTR_PARAMS) {
	VMACHINE_OPCODE
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

void func_alloc(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const int_fast32_t offset = *pc.int32++;
	sp -= offset * sizeof(union VMachineVal);
	VMACHINE_OFFS
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

/// arithmetic ops.
void func_add(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].int64 = rsp[b].int64 + rsp[c].int64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	//(*opcode_func[*pc.uint8++])(INSTR_ARGS);
}
void func_addi(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].int64 = rsp[b].int64 + c;
	VMACHINE_2REG
	VMACHINE_3OPI
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sub(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].int64 = rsp[b].int64 - rsp[c].int64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_subi(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].int64 = rsp[b].int64 - c;
	VMACHINE_2REG
	VMACHINE_3OPI
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_mul(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].int64 = rsp[b].int64 * rsp[c].int64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_muli(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].int64 = rsp[b].int64 * c;
	VMACHINE_2REG
	VMACHINE_3OPI
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__div(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 / rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_divi(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 / c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_mod(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 % rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_modi(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 % c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

void func__and(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 & rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__andi(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 & c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__or(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 | rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__ori(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 | c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__xor(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 ^ rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func__xori(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 ^ c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sll(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 << rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_slli(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 << c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_srl(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 >> rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_srli(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 >> c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sra(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].int64 >> rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_srai(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].int64 >> c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

void func_slt(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].int64 < rsp[c].int64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_slti(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].int64 = rsp[b].int64 < c;
	VMACHINE_2REG
	VMACHINE_3OPI
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sltu(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c        = (ops & 0xff0000) >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 < rsp[c].uint64;
	VMACHINE_3REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_sltui(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const uint32_t c		= ops >> 16;
	union VMachineVal *const restrict rsp = ( union VMachineVal* )(sp);
	rsp[a].uint64 = rsp[b].uint64 < c;
	VMACHINE_2REG
	VMACHINE_3OPU
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}

/// Jumps
void func_jmp(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const int_fast32_t offset = *pc.int32++;
	pc.uint32 += offset;
	VMACHINE_OFFS
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_jeq(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	const union VMachineVal *const restrict rsp = ( const union VMachineVal* )(sp);
	VMACHINE_2REG
	VMACHINE_3OPI
	if( rsp[a].uint64==rsp[b].uint64 ) {
		pc.uint32 += c;
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	} else {
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	}
}
void func_jne(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	const union VMachineVal *const restrict rsp = ( const union VMachineVal* )(sp);
	VMACHINE_2REG
	VMACHINE_3OPI
	if( rsp[a].uint64 != rsp[b].uint64 ) {
		pc.uint32 += c;
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	} else {
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	}
}
void func_jlt(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	const union VMachineVal *const restrict rsp = ( const union VMachineVal* )(sp);
	VMACHINE_2REG
	VMACHINE_3OPI
	if( rsp[a].int64 < rsp[b].int64 ) {
		pc.uint32 += c;
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	} else {
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	}
}
void func_jltu(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	const union VMachineVal *const restrict rsp = ( const union VMachineVal* )(sp);
	VMACHINE_2REG
	VMACHINE_3OPI
	if( rsp[a].uint64 < rsp[b].uint64 ) {
		pc.uint32 += c;
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	} else {
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	}
}
void func_jge(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	const union VMachineVal *const restrict rsp = ( const union VMachineVal* )(sp);
	VMACHINE_2REG
	VMACHINE_3OPI
	if( rsp[a].int64 >= rsp[b].int64 ) {
		pc.uint32 += c;
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	} else {
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	}
}
void func_jgeu(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uint_fast32_t ops = *pc.uint32++;
	const uint32_t a        = ops & 0xff;
	const uint32_t b        = (ops & 0xff00) >> 8;
	const int32_t  c        = ( int16_t )(ops >> 16);
	const union VMachineVal *const restrict rsp = ( const union VMachineVal* )(sp);
	VMACHINE_2REG
	VMACHINE_3OPI
	if( rsp[a].uint64 >= rsp[b].uint64 ) {
		pc.uint32 += c;
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	} else {
		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	}
}


/// function call ops
void func_jal(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const int_fast32_t offset = *pc.int32++;
	lr = ( uintptr_t )(pc.uint8);
	pc.uint32 += offset;
	VMACHINE_OFFS
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_ret(INSTR_PARAMS) {
	VMACHINE_OPCODE
	pc.uint8 = ( const uint8_t* )(lr);
	if( pc.uint8==NULL ) {
		func_halt(INSTR_ARGS);
	} else {
  		( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
	}
}
void func_savelink(INSTR_PARAMS) {
	VMACHINE_OPCODE
	cp -= sizeof lr;
	uintptr_t *const restrict call_stack = ( uintptr_t* )(cp);
	*call_stack = lr;
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_remitlink(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uintptr_t *const restrict call_stack = ( const uintptr_t* )(cp);
	cp += sizeof lr;
	lr = *call_stack;
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
void func_enter(INSTR_PARAMS) {
	VMACHINE_OPCODE
	const uintptr_t *const restrict call_stack = ( const uintptr_t* )(cp);
	cp += sizeof lr;
	lr = *call_stack;
	const int_fast32_t offset = *pc.int32++;
	sp -= offset * sizeof(union VMachineVal);
	( *( InstrFunc* )(*pc.uint64++) )(INSTR_ARGS);
}
