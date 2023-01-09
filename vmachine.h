#ifndef VMACHINE_INCLUDED
#	define VMACHINE_INCLUDED

#include "harbol_common_defines.h"
#include "harbol_common_includes.h"

#ifdef __cplusplus
extern "C" {
#endif


union VMachineVal {
	uint64_t       uint64;
	int64_t        int64;
	float64_t      float64;
	
	/// for general pointer data, use (u)intptr. 
	intptr_t       intptr;
	uintptr_t      uintptr;
	
	size_t         size;
	ssize_t        ssize; /// defined in harbol_common_includes.h
	
	uint32_t       uint32,  uint32a[sizeof(uint64_t) / sizeof(uint32_t)];
	int32_t        int32,   int32a[sizeof(uint64_t) / sizeof(int32_t)];
	float32_t      float32, float32a[sizeof(uint64_t) / sizeof(float32_t)];
	
	uint16_t       uint16,  uint16a[sizeof(uint64_t) / sizeof(uint16_t)];
	int16_t        int16,   int16a[sizeof(uint64_t) / sizeof(int16_t)];
	
	uint8_t        uint8,   uint8a[sizeof(uint64_t) / sizeof(uint8_t)];
	int8_t         int8,    int8a[sizeof(uint64_t) / sizeof(int8_t)];
	bool           b00l,    boola[sizeof(uint64_t) / sizeof(bool)];
	
	uint_fast64_t  ufast64; int_fast64_t fast64;
	uint_fast32_t  ufast32; int_fast32_t fast32;
	uint_fast16_t  ufast16; int_fast16_t fast16;
	uint_fast8_t   ufast8;  int_fast8_t  fast8;
};

union VMachinePtr {
	const uint64_t          *uint64;
	const uint32_t          *uint32;
	const uint16_t          *uint16;
	const uint8_t           *uint8;
	
	const int64_t           *int64;
	const int32_t           *int32;
	const int16_t           *int16;
	const int8_t            *int8;
	
	const float32_t         *float32;
	const float64_t         *float64;

	const uintptr_t         *uintptr;
	const intptr_t          *intptr;
	
	const union VMachineVal *val;
};


/**
 * Bytecode Design.
 * Things to take into consideration:
 * 1. Operations/assembly per opcode - make better use of CPU instruction cache.
 * 2. Smaller amount of opcodes will require more instructions generated.
 * 3. For archs where params are passed by regs, remember to keep hot VM state vars in those registers.
 */
#define VMACHINE_INSTR_SET \
	X(halt) X(nop) X(alloc) \
    X(add) X(addi) X(sub) X(subi) \
    X(mul) X(muli) X(_div) X(divi) X(mod) X(modi) \
	X(_and) X(_andi) X(_or) X(_ori) X(_xor) X(_xori) \
	X(sll) X(slli) X(srl) X(srli) X(sra) X(srai) \
	X(slt) X(slti) X(sltu) X(sltui) \
	X(jmp) X(jeq) X(jne) X(jlt) X(jltu) X(jge) X(jgeu) \
	X(jal) X(ret) X(savelink) X(remitlink) X(enter)

#define X(x) x,
enum VMachineInstrSet { VMACHINE_INSTR_SET MaxOps };
#undef X

struct VMachineState;

#define INSTR_PARAMS    union VMachinePtr pc, uintptr_t sp, uintptr_t cp, uintptr_t lr, struct VMachineState *state
#define INSTR_ARGS      pc, sp, cp, lr, state
typedef void InstrFunc(INSTR_PARAMS);
typedef void VMachineCFunc(INSTR_PARAMS);

#define X(x)    InstrFunc func_##x ;
	VMACHINE_INSTR_SET
#undef X

#define X(x) #x ,
	/// for debugging purposes.
	static const char *const op_to_cstr[] = { VMACHINE_INSTR_SET };
#undef X

#define X(x) &func_##x ,
	static InstrFunc *const opcode_func[] = { VMACHINE_INSTR_SET };
#undef X
#undef VMACHINE_INSTR_SET


struct VMachineNative {
	const char    *name;
	VMachineCFunc *cfunc;
};


enum {
	VMACHINE_FLAG_NATIVE = 1,  /// if is a native C/JIT compiled function.
	VMACHINE_FLAG_LINKED = 2,  /// ptr has been linked.
};

struct VMachineItem {
	uintptr_t
		item, /// data, as uint32_t*, cast as needed.
		owner /// Add an owner so we can do dynamic linking & loading.
	;
	uint_fast32_t flags;
}; /// 12 (32-bit) ~ 24 (64-bit) bytes

enum { SYMBOL_BUCKETS = 32 };
struct VMachineSymTable {
	const char         **keys;   /// array of string names of each item.
	struct VMachineItem *table;  /// VMachineItem array.
	size_t
		len,                     /// table's len.
		buckets[SYMBOL_BUCKETS], /// hash index bucket for each index. SIZE_MAX if invalid.
	   *hashes,                  /// hash value for each item index.
	   *chain,                   /// index chain to resolve collisions. SIZE_MAX if invalid.
	   *bytes                    /// byte of item data.
	;
}; /// 148 (32-bit) ~ 296 (64-bit) bytes

struct VMachineState {
	const struct VMachineSymTable *functbl, *vartbl;
	union VMachineVal              saved_ra;
	uintptr_t                      opstk, callstk;
	size_t                         opstk_size;
	size_t                         callstk_size;
};


void vmachine_run(size_t stksize, union VMachineVal stkmem[], size_t callsize, uintptr_t callmem[], const uint32_t instrs[], struct VMachineState *vmstate);
void vmachine_exec(INSTR_PARAMS);

#ifdef __cplusplus
}
#endif

#endif /** VMACHINE_INCLUDED */
