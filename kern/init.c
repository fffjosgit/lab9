/* See COPYRIGHT for copyright information. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/monitor.h>
#include <kern/console.h>
#include <kern/env.h>
#include <kern/trap.h>
#include <kern/sched.h>
#include <kern/cpu.h>
#include <kern/picirq.h>
#include <kern/kclock.h>

void
i386_init(void)
{
	extern char edata[], end[];
	extern uint8_t _binary_obj_prog_sc_test1_start[], _binary_obj_prog_sc_test1_end[], _binary_obj_prog_sc_test1_size[];
	extern uint8_t _binary_obj_kern_kernel_pre_sym_start[], _binary_obj_kern_kernel_pre_sym_end[], _binary_obj_kern_kernel_pre_sym_size[];

	// Before doing anything else, complete the ELF loading process.
	// Clear the uninitialized global data (BSS) section of our program.
	// This ensures that all static/global variables start out zero.
	memset(edata, 0, end - edata);

	// Initialize the console.
	// Can't call cprintf until after we do this!
	cons_init();

	cprintf("6828 decimal is %o octal!\n", 6828);
	cprintf("END: %p\n", end);
	cprintf("TEST1 START: %p\t END: %p\t SIZE: 0x%x\n", _binary_obj_prog_sc_test1_start, _binary_obj_prog_sc_test1_end, ( int ) _binary_obj_prog_sc_test1_size );
	//cprintf("kernel_sym: %p\t END: %p\t SIZE: %d\n", _binary_obj_kern_kernel_pre_sym_start, _binary_obj_kern_kernel_pre_sym_end, (int) _binary_obj_kern_kernel_pre_sym_size );
	_binary_obj_kern_kernel_pre_sym_end[0] = 0;
	//cprintf("kernel_sym: %s\n", _binary_obj_kern_kernel_pre_sym_start);

	// user environment initialization functions
	env_init();
	
	trap_init();

	pic_init();
	rtc_init();

#if defined(TEST)
	// Don't touch -- used by grading script!
	//ENV_CREATE(TEST, ENV_TYPE_USER);
#else
	// Touch all you want.
	ENV_CREATE(prog_sc_test1, ENV_TYPE_KERNEL, 1);
	ENV_CREATE(prog_sc_test2, ENV_TYPE_KERNEL, 2);
	ENV_CREATE(prog_sc_test3, ENV_TYPE_KERNEL, 3);
	//ENV_CREATE(prog_sc_test4, ENV_TYPE_KERNEL, 4);
	//ENV_CREATE(prog_sc_test5, ENV_TYPE_KERNEL, 5);
	ENV_CREATE(prog_sc_test6, ENV_TYPE_KERNEL, 6);
	ENV_CREATE(prog_sc_test7, ENV_TYPE_KERNEL, 7);
	ENV_CREATE(prog_sc_test8, ENV_TYPE_KERNEL, 8);
	ENV_CREATE(prog_sc_test9, ENV_TYPE_KERNEL, 9);
	ENV_CREATE(prog_sc_test10, ENV_TYPE_KERNEL, 10);
#endif // TEST*

	// Schedule and run the first user environment!
	sched_yield();
}


/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", and then enters the kernel monitor.
 */
void
_panic(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	if (panicstr)
		goto dead;
	panicstr = fmt;

	// Be extra sure that the machine is in as reasonable state
	__asm __volatile("cli; cld");

	va_start(ap, fmt);
	cprintf("kernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

dead:
	/* break into the kernel monitor */
	while (1)
		monitor(NULL);
}

/* like panic, but don't */
void
_warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf("kernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}
