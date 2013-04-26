#include <inc/mmu.h>
#include <inc/x86.h>
#include <inc/assert.h>

#include <kern/pmap.h>
#include <kern/trap.h>
#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/env.h>
#include <kern/syscall.h>
#include <kern/sched.h>
#include <kern/kclock.h>
#include <kern/picirq.h>
#include <kern/cpu.h>
#include <kern/spinlock.h>
#include <kern/picirq.h>

static struct Taskstate ts;

/* For debugging, so print_trapframe can distinguish between printing
 * a saved trapframe and printing the current trapframe and print some
 * additional information in the latter case.
 */
static struct Trapframe *last_tf;

/* Interrupt descriptor table.  (Must be built at run time because
 * shifted function addresses can't be represented in relocation records.)
 */
struct Gatedesc idt[256] = { { 0 } };
struct Pseudodesc idt_pd = {
	sizeof(idt) - 1, (uint32_t) idt
};

bool in_clk_intr = false;

static const char *trapname(int trapno)
{
	static const char * const excnames[] = {
		"Divide error",
		"Debug",
		"Non-Maskable Interrupt",
		"Breakpoint",
		"Overflow",
		"BOUND Range Exceeded",
		"Invalid Opcode",
		"Device Not Available",
		"Double Fault",
		"Coprocessor Segment Overrun",
		"Invalid TSS",
		"Segment Not Present",
		"Stack Fault",
		"General Protection",
		"Page Fault",
		"(unknown trap)",
		"x87 FPU Floating-Point Error",
		"Alignment Check",
		"Machine-Check",
		"SIMD Floating-Point Exception"
	};

	if (trapno < sizeof(excnames)/sizeof(excnames[0]))
		return excnames[trapno];
	if (trapno == T_SYSCALL)
		return "System call";
	if (trapno >= IRQ_OFFSET && trapno < IRQ_OFFSET + 16)
		return "Hardware Interrupt";
	return "(unknown trap)";
}

// Set up a normal interrupt/trap gate descriptor.
// - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
// - sel: Code segment selector for interrupt/trap handler
// - off: Offset in code segment for interrupt/trap handler
// - dpl: Descriptor Privilege Level -
//	  the privilege level required for software to invoke
//	  this interrupt/trap gate explicitly using an int instruction.
//#define SETGATE(gate, istrap, sel, off, dpl)


void
trap_init(void)
{
	extern struct Segdesc gdt[];

	// LAB 3: Your code here.

	extern void diverr_entry();   // T_DIVIDE
    extern void debug_entry();    // T_DEBUG
    extern void nmi_entry();      //T_NMI
    extern void brkpt_entry();    //T_BRKPT
    extern void overflow_entry(); //T_OFLOW
    extern void bound_entry();    //T_BOUND
    extern void illop_entry();    //T_ILLOP
    extern void device_entry();   //T_DEVICE
    extern void dblflt_entry();   //T_DBLFLT
    extern void tss_entry();      //T_TSS
    extern void segnp_entry();    //T_SEGNP
    extern void stack_entry();    //T_STACK
    extern void gpflt_entry();    //T_GPFLT
    extern void pgflt_entry();    //T_PGFLT
    extern void fperr_entry();    //T_FPERR
    extern void align_entry();    //T_ALIGN
    extern void mchk_entry();     //T_MCHK
    extern void simderr_entry();  //T_SIMDERR
    extern void syscall_entry();  //T_SYSCALL

    SETGATE(idt[T_DIVIDE],  1, GD_KT, diverr_entry, 0);
	SETGATE(idt[T_DEBUG],   1, GD_KT, debug_entry, 0);
	SETGATE(idt[T_NMI],     0, GD_KT, nmi_entry, 0);
	SETGATE(idt[T_BRKPT],   1, GD_KT, brkpt_entry, 3);
	SETGATE(idt[T_OFLOW],   1, GD_KT, overflow_entry, 0);
	SETGATE(idt[T_BOUND],   1, GD_KT, bound_entry, 0);
	SETGATE(idt[T_ILLOP],   1, GD_KT, illop_entry, 0);
	SETGATE(idt[T_DEVICE],  1, GD_KT, device_entry, 0);
	SETGATE(idt[T_DBLFLT],  1, GD_KT, dblflt_entry, 0);
	SETGATE(idt[T_TSS],     1, GD_KT, tss_entry, 0);
	SETGATE(idt[T_SEGNP],   1, GD_KT, segnp_entry, 0);
	SETGATE(idt[T_STACK],   1, GD_KT, stack_entry, 0);
	SETGATE(idt[T_GPFLT],   1, GD_KT, gpflt_entry, 0);
	SETGATE(idt[T_PGFLT],   1, GD_KT, pgflt_entry, 0);
	SETGATE(idt[T_FPERR],   1, GD_KT, fperr_entry, 0);
	SETGATE(idt[T_ALIGN],   1, GD_KT, align_entry, 0);
	SETGATE(idt[T_MCHK],    1, GD_KT, mchk_entry, 0);
	SETGATE(idt[T_SIMDERR], 1, GD_KT, simderr_entry, 0);
	SETGATE(idt[T_SYSCALL], 0, GD_KT, syscall_entry, 3);

    extern void irq0_entry();
    extern void irq1_entry();
    extern void irq2_entry();
    extern void irq3_entry();
    extern void irq4_entry();
    extern void irq5_entry();
    extern void irq6_entry();
    extern void irq7_entry();
    extern void irq8_entry();
    extern void irq9_entry();
    extern void irq10_entry();
    extern void irq11_entry();
    extern void irq12_entry();
    extern void irq13_entry();
    extern void irq14_entry();

    SETGATE(idt[IRQ_OFFSET + 0], 0, GD_KT, irq0_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 1], 0, GD_KT, irq1_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 2], 0, GD_KT, irq2_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 3], 0, GD_KT, irq3_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 4], 0, GD_KT, irq4_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 5], 0, GD_KT, irq5_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 6], 0, GD_KT, irq6_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 7], 0, GD_KT, irq7_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 8], 0, GD_KT, irq8_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 9], 0, GD_KT, irq9_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 10], 0, GD_KT, irq10_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 11], 0, GD_KT, irq11_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 12], 0, GD_KT, irq12_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 13], 0, GD_KT, irq13_entry, 0);
	SETGATE(idt[IRQ_OFFSET + 14], 0, GD_KT, irq14_entry, 0);

	// Per-CPU setup 
	trap_init_percpu();
}

// Initialize and load the per-CPU TSS and IDT
void
trap_init_percpu(void)
{
	// The example code here sets up the Task State Segment (TSS) and
	// the TSS descriptor for CPU 0. But it is incorrect if we are
	// running on other CPUs because each CPU has its own kernel stack.
	// Fix the code so that it works for all CPUs.
	//
	// Hints:
	//   - The macro "thiscpu" always refers to the current CPU's
	//     struct CpuInfo;
	//   - The ID of the current CPU is given by cpunum() or
	//     thiscpu->cpu_id;
	//   - Use "thiscpu->cpu_ts" as the TSS for the current CPU,
	//     rather than the global "ts" variable;
	//   - Use gdt[(GD_TSS0 >> 3) + i] for CPU i's TSS descriptor;
	//   - You mapped the per-CPU kernel stacks in mem_init_mp()
	//
	// ltr sets a 'busy' flag in the TSS selector, so if you
	// accidentally load the same TSS on more than one CPU, you'll
	// get a triple fault.  If you set up an individual CPU's TSS
	// wrong, you may not get a fault until you try to return from
	// user space on that CPU.
	//
	// LAB 4: Your code here:

	// Setup a TSS so that we get the right stack
	// when we trap to the kernel.
	ts.ts_esp0 = KSTACKTOP;
	ts.ts_ss0 = GD_KD;

	// Initialize the TSS slot of the gdt.
	gdt[GD_TSS0 >> 3] = SEG16(STS_T32A, (uint32_t) (&ts),  sizeof(struct Taskstate), 0);
	gdt[GD_TSS0 >> 3].sd_s = 0;

	// Load the TSS selector (like other segment selectors, the
	// bottom three bits are special; we leave them 0)
	ltr(GD_TSS0);

	// Load the IDT
	lidt(&idt_pd);
}

void
print_trapframe(struct Trapframe *tf)
{
	cprintf("TRAP frame at %p\n", tf);
	print_regs(&tf->tf_regs);
	cprintf("  es   0x----%04x\n", tf->tf_es);
	cprintf("  ds   0x----%04x\n", tf->tf_ds);
	cprintf("  trap 0x%08x %s\n", tf->tf_trapno, trapname(tf->tf_trapno));
	// If this trap was a page fault that just happened
	// (so %cr2 is meaningful), print the faulting linear address.
	if (tf == last_tf && tf->tf_trapno == T_PGFLT)
		cprintf("  cr2  0x%08x\n", rcr2());
	cprintf("  err  0x%08x", tf->tf_err);
	// For page faults, print decoded fault error code:
	// U/K=fault occurred in user/kernel mode
	// W/R=a write/read caused the fault
	// PR=a protection violation caused the fault (NP=page not present).
	if (tf->tf_trapno == T_PGFLT)
		cprintf(" [%s, %s, %s]\n",
			tf->tf_err & 4 ? "user" : "kernel",
			tf->tf_err & 2 ? "write" : "read",
			tf->tf_err & 1 ? "protection" : "not-present");
	else
		cprintf("\n");
	cprintf("  eip  0x%08x\n", tf->tf_eip);
	cprintf("  cs   0x----%04x\n", tf->tf_cs);
	cprintf("  flag 0x%08x\n", tf->tf_eflags);
	if ((tf->tf_cs & 3) != 0) {
		cprintf("  esp  0x%08x\n", tf->tf_esp);
		cprintf("  ss   0x----%04x\n", tf->tf_ss);
	}
}

void
print_regs(struct PushRegs *regs)
{
	cprintf("  edi  0x%08x\n", regs->reg_edi);
	cprintf("  esi  0x%08x\n", regs->reg_esi);
	cprintf("  ebp  0x%08x\n", regs->reg_ebp);
	cprintf("  oesp 0x%08x\n", regs->reg_oesp);
	cprintf("  ebx  0x%08x\n", regs->reg_ebx);
	cprintf("  edx  0x%08x\n", regs->reg_edx);
	cprintf("  ecx  0x%08x\n", regs->reg_ecx);
	cprintf("  eax  0x%08x\n", regs->reg_eax);
}

void ssched_yield(void)
{
}

static void
trap_dispatch(struct Trapframe *tf)
{
	// Handle processor exceptions.
	// LAB 3: Your code here.
	int ret;
	
	switch(tf->tf_trapno) {
	case T_DIVIDE:     		// divide error
	    break;
    case T_DEBUG:      		// debug exception
        monitor(tf);
        return;
    case T_NMI:        		// non-maskable interrupt
        break;
    case T_BRKPT:      		// breakpoint
        panic("breakpoint here");
        break;
    case T_OFLOW:      		// overflow
        break;
    case T_BOUND:      		// bounds check
        break;
    case T_ILLOP:      		// illegal opcode
        break;
    case T_DEVICE:     		// device not available
        break;
    case T_DBLFLT:     		// double fault
        break;
    /* #define T_COPROC  9 */	// reserved (not generated by recent processors)
    case T_TSS:       		// invalid task switch segment
        break;
    case T_SEGNP:     		// segment not present
        break;
    case T_STACK:     		// stack exception
        break;
    case T_GPFLT:     		// general protection fault
        break;
    case T_PGFLT:     		// page fault
        page_fault_handler(tf);
        break;
    /* #define T_RES    15 */	// reserved
    case T_FPERR:     		// floating point error
        break;
    case T_ALIGN:     		// aligment check
        break;
    case T_MCHK:      		// machine check
        break;
    case T_SIMDERR:         // SIMD floating point error
        break;
	case IRQ_OFFSET + IRQ_TIMER:
	    //sched_yield();
	    break;
	case IRQ_OFFSET + IRQ_KBD:        //keyboard
	    kbd_intr();
	    return;
	case T_SYSCALL:         //system call
	    ret = syscall(tf->tf_regs.reg_eax, tf->tf_regs.reg_edx,
				      tf->tf_regs.reg_ecx, tf->tf_regs.reg_ebx,
				      tf->tf_regs.reg_edi, tf->tf_regs.reg_esi);
		tf->tf_regs.reg_eax = ret;
	    return;
	}

	// Handle spurious interrupts
	// The hardware sometimes raises these because of noise on the
	// IRQ line or other reasons. We don't care.
	if (tf->tf_trapno == IRQ_OFFSET + IRQ_SPURIOUS) {
		cprintf("Spurious interrupt on irq 7\n");
		print_trapframe(tf);
		return;
	}

	// Handle clock interrupts. Don't forget to acknowledge the
	// interrupt using lapic_eoi() before calling the scheduler!
	// LAB 4: Your code here.

	// Unexpected trap: The user process or the kernel has a bug.
	print_trapframe(tf);
	if (tf->tf_cs == GD_KT)
		panic("trap_dispatch: unhandled trap in kernel.\n");
	else {
		env_destroy(curenv);
		return;
	}
}

void
trap(struct Trapframe *tf)
{
	// The environment may have set DF and some versions
	// of GCC rely on DF being clear
	asm volatile("cld" ::: "cc");

	// Halt the CPU if some other CPU has called panic()
	extern char *panicstr;
	if (panicstr)
		asm volatile("hlt");

	// Re-acqurie the big kernel lock if we were halted in
	// sched_yield()
	if (xchg(&thiscpu->cpu_status, CPU_STARTED) == CPU_HALTED)
		lock_kernel();
	// Check that interrupts are disabled.  If this assertion
	// fails, DO NOT be tempted to fix it by inserting a "cli" in
	// the interrupt path.
	assert(!(read_eflags() & FL_IF));

	//cprintf("Incoming TRAP frame at %p\n", tf);

	if ((tf->tf_cs & 3) == 3) {
		// Trapped from user mode.
		// Acquire the big kernel lock before doing any
		// serious kernel work.
		// LAB 4: Your code here.
		assert(curenv);

		// Garbage collect if current enviroment is a zombie
		if (curenv->env_status == ENV_DYING) {
			env_free(curenv);
			curenv = NULL;
			sched_yield();
		}

		// Copy trap frame (which is currently on the stack)
		// into 'curenv->env_tf', so that running the environment
		// will restart at the trap point.
		curenv->env_tf = *tf;
		// The trapframe on the stack should be ignored from here on.
		tf = &curenv->env_tf;
	}

	// Record that tf is the last real trapframe so
	// print_trapframe can print some additional information.
	last_tf = tf;

	// Dispatch based on what type of trap occurred
	trap_dispatch(tf);

	// If we made it to this point, then no other environment was
	// scheduled, so we should return to the current environment
	// if doing so makes sense.
	if (curenv && curenv->env_status == ENV_RUNNING) {
		env_run(curenv);
	} else {
		sched_yield();
	}
}


void
page_fault_handler(struct Trapframe *tf)
{
	uint32_t fault_va;

	// Read processor's CR2 register to find the faulting address
	fault_va = rcr2();

	// Handle kernel-mode page faults.

	// LAB 3: Your code here.
	
	if ((tf->tf_cs & 3) == 0) {
		cprintf("page_fault_handler: kernel fault va %08x ip %08x.\n", fault_va, tf->tf_eip);
		panic("page_fault_handler: page fault in kernel mode.\n");
		return;
	}

	// We've already handled kernel-mode exceptions, so if we get here,
	// the page fault happened in user mode.

	// Call the environment's page fault upcall, if one exists.  Set up a
	// page fault stack frame on the user exception stack (below
	// UXSTACKTOP), then branch to curenv->env_pgfault_upcall.
	//
	// The page fault upcall might cause another page fault, in which case
	// we branch to the page fault upcall recursively, pushing another
	// page fault stack frame on top of the user exception stack.
	//
	// The trap handler needs one word of scratch space at the top of the
	// trap-time stack in order to return.  In the non-recursive case, we
	// don't have to worry about this because the top of the regular user
	// stack is free.  In the recursive case, this means we have to leave
	// an extra word between the current top of the exception stack and
	// the new stack frame because the exception stack _is_ the trap-time
	// stack.
	//
	// If there's no page fault upcall, the environment didn't allocate a
	// page for its exception stack or can't write to it, or the exception
	// stack overflows, then destroy the environment that caused the fault.
	// Note that the grade script assumes you will first check for the page
	// fault upcall and print the "user fault va" message below if there is
	// none.  The remaining three checks can be combined into a single test.
	//
	// Hints:
	//   user_mem_assert() and env_run() are useful here.
	//   To change what the user environment runs, modify 'curenv->env_tf'
	//   (the 'tf' variable points at 'curenv->env_tf').

	// LAB 4: Your code here.

	unsigned int orig_esp;
	struct UTrapframe utf;

	// Destroy the environment that caused the fault.
	if(!curenv->env_pgfault_upcall) {
	    cprintf("page_fault_handler: no page fault handler installed.\n");
	    cprintf("page_fault_handler: [%08x] user fault va %08x ip %08x.\n", curenv->env_id, fault_va, tf->tf_eip);
	    print_trapframe(tf);
	    env_destroy(curenv);
    }

    //user exception stack
    user_mem_assert(curenv, (void *)(UXSTACKTOP - 4), 4, PTE_P | PTE_W | PTE_U); 
    user_mem_assert(curenv, (void *)(curenv->env_pgfault_upcall), 4, PTE_P | PTE_U);

    utf.utf_fault_va = fault_va;
	utf.utf_err = tf->tf_err;
	utf.utf_regs = tf->tf_regs;
	utf.utf_eip = tf->tf_eip;
	utf.utf_eflags = tf->tf_eflags;
	utf.utf_esp = tf->tf_esp;

	if((tf->tf_esp >= (UXSTACKTOP - PGSIZE)) && (tf->tf_esp < UXSTACKTOP)) {
	    tf->tf_esp -= 4;
	} else {
	    tf->tf_esp = UXSTACKTOP;
	    
	    if(tf->tf_esp < UXSTACKTOP - PGSIZE) {
	        cprintf("page_fault_handler: user exception stack overflow.\n");
	        cprintf("page_fault_handler: [%08x] user fault va %08x ip %08x.\n", curenv->env_id, fault_va, tf->tf_eip);
	        print_trapframe(tf);
	        env_destroy(curenv);
	        return;
	    } 
	}
    
    //push utf on stack
    *((struct UTrapframe *) (tf->tf_esp)) = utf;
    tf->tf_eip = (unsigned int)curenv->env_pgfault_upcall;
    env_run(curenv);
}

