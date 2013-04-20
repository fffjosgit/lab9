/* See COPYRIGHT for copyright information. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>

#include <kern/monitor.h>
#include <kern/console.h>
#include <kern/pmap.h>
#include <kern/kclock.h>
#include <kern/env.h>
#include <kern/trap.h>
#include <kern/sched.h>
#include <kern/picirq.h>
#include <kern/cpu.h>
#include <kern/spinlock.h>
#include <kern/time.h>


//static void rtc_init(void)
//{
//}

void
i386_init(void)
{
	extern char edata[], end[];

	// Before doing anything else, complete the ELF loading process.
	// Clear the uninitialized global data (BSS) section of our program.
	// This ensures that all static/global variables start out zero.
	memset(edata, 0, end - edata);

	// Initialize the console.
	// Can't call cprintf until after we do this!
	cons_init();

	cprintf("6828 decimal is %o octal!\n", 6828);

	// Lab 2 memory management initialization functions
	mem_init();

	// Lab 3 user environment initialization functions
	env_init();
	trap_init();

	//mp_init();
	//lapic_init();

	pic_init();
	rtc_init();
	time_init();

#if defined(TEST)
	// Don't touch -- used by grading script!
	ENV_CREATE(TEST, ENV_TYPE_USER);
#else
	// Touch all you want.
	//ENV_CREATE(prog_sc_test1, ENV_TYPE_KERNEL, 1);
	//ENV_CREATE(prog_sc_test2, ENV_TYPE_KERNEL, 2);
	//ENV_CREATE(prog_sc_test3, ENV_TYPE_KERNEL, 3);
	//ENV_CREATE(prog_sc_test4, ENV_TYPE_KERNEL, 4);
	//ENV_CREATE(prog_sc_test5, ENV_TYPE_KERNEL, 5);
	//ENV_CREATE(prog_sc_test6, ENV_TYPE_KERNEL, 6);
	//ENV_CREATE(prog_sc_test7, ENV_TYPE_KERNEL, 7);
	//ENV_CREATE(prog_sc_test8, ENV_TYPE_KERNEL, 8);
	//ENV_CREATE(prog_sc_test9, ENV_TYPE_KERNEL, 9);
	//ENV_CREATE(prog_sc_test10, ENV_TYPE_KERNEL, 10);

	NEW_ENV_CREATE(user_primes, ENV_TYPE_USER);
#endif // TEST*

	// We only have one user environment for now, so just run it.
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
