#include <stdio.h>
#include <time.h>

#include "vmachine.h"
#include "instrgen.h"

int main(void) {
	struct {
		uint32_t *buf;
		size_t    len;
	} instrs = {0};
	instrs.buf = calloc(1000, sizeof *instrs.buf);
	
	/// Program: basic loop
	/// for( int i=0; i < 100M; i++ ) {}
	/// 100000 = 100K | 1000000 = 1M | 10000000 = 10M | 100000000 = 100M | 1000000000 = 1B
	/**         
	 *          alloc   4
	 *          addi    r0, r0, 32000
	 *          muli    r0, r0, 3125 ; makes 100M
	 * loop:    addi    r1, r1, 1
	 *          jne     r1, r0, loop
	 *          ret
	 */
	/*
	instrs.len += vmachine_instr_gen(&instrs.buf[instrs.len], alloc, 4);          /// 12 bytes
	instrs.len += vmachine_instr_gen(&instrs.buf[instrs.len], addi, 0, 0, 32000); /// 12 bytes
	instrs.len += vmachine_instr_gen(&instrs.buf[instrs.len], muli, 0, 0, 31250); /// 12 bytes // 31250 for 1b, 3125 for 100m
	instrs.len += vmachine_instr_gen(&instrs.buf[instrs.len], addi, 1, 1, 1);     /// 12 bytes
	instrs.len += vmachine_instr_gen(&instrs.buf[instrs.len], jne, 1, 0, -6);     /// 12 bytes
	instrs.len += vmachine_instr_gen(&instrs.buf[instrs.len], ret);               /// 8 bytes
	*/
	
/**
fib:
    enter   6
    jlti    r6, 2, .calc ;; if( n<2 )
    
    ;; return 1;
    addi    r0, zero, 1
    leave   6
    
.calc
    ;; temp1 = fib(n - 1)
    subi    r1, r6, 1  ;; _temp1 = n-1;
    subi    r2, r6, 2  ;; _temp2 = n-2;
    addi    r0, r1, 0  ;; temp = _temp1;
    call    fib        ;; res1 = factorial(temp);
    addi    r3, r0, 0  ;; __temp1 = res1;
    
    ;; temp2 = fib(n - 2)
    addi    r0, r2, 0  ;; temp = _temp2;
    call    fib        ;; res2 = factorial(temp);
    addi    r4, r0, 0  ;; __temp2 = res2;
    
    ;; return temp1 + temp2
    add     r6, r3, r4      ;; n = __temp1 + __temp2;
    leave   6
 */
	instrs.len += vmachine_instr_gen(&instrs.buf[instrs.len], enter, 6);          /// 12 bytes
	
	
	struct VMachineState vmach         = {0};
	union VMachineVal    stackmem[256] = {0};
	uintptr_t            callmem[256]  = {0};
	vmachine_run(
		sizeof stackmem / sizeof stackmem[0],
		&stackmem[0],
		sizeof callmem / sizeof callmem[0],
		&callmem[0],
		&instrs.buf[0],
		&vmach
	);
	free(instrs.buf);
}
