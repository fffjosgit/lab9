#include <inc/assert.h>
#include <inc/random.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#include <kern/time.h>

void sched_halt(void);

/*void perform_io_simulation( void )
{
	if ( in_clock_interrupt() ) {
		int i;
		for(i = 0; i < NENV; ++i) {
			if (envs[i].env_status == ENV_BLOCKING) {
				if (!(--envs[i].blocking_cycles))
					envs[i].env_status = ENV_RUNNABLE;
			}
		}
	}
}*/

void
sched_yield(void)
{
	struct Env *idle;

	uint32_t envid = thiscpu->cpu_env ? ENVX(thiscpu->cpu_env->env_id) : 0;
	uint32_t first_eid = (++envid) % NENV;
	int next_envid;
	int i;
	struct Env env;
	int min_cc;

	//perform_io_simulation();

	for(i = 0; i < NENV; i++) {
	    if((env = envs[i]).env_priority == ENV_PRIORITY_HIGH) {
	        
	        cprintf("[%08x]: %d %d %d %d. \n", env.env_id, 
	            env.env_c, env.env_p, env.env_cp, env.env_cc);

	        if(env.env_cp <= 0) {
	            env.env_cp = env.env_p;
	            if(env.env_cc > 0) {
	                panic("Sched: i'm a real!");	                
	            } else {
	                env.env_cc = env.env_c;    
	            }
	        }
	        env.env_cp--;	        	            
	    }
	}
	
	min_cc = 100000;//env.env_cc;
	next_envid = -1;

	if(curenv && (curenv->env_priority == ENV_PRIORITY_HIGH) && (curenv->env_status == ENV_RUNNING)) {
	    curenv->env_cc--;        
	}

	for(i = 0; i < NENV; i++) {
	    if(((env = envs[i]).env_priority == ENV_PRIORITY_HIGH) 
	    && ((env.env_status == ENV_RUNNABLE) || (env.env_status == ENV_RUNNING)) ) {
	        if((env.env_cc > 0) && (env.env_cc <= min_cc)) {
	            next_envid = i;
	        }                                               
	    }    
	}

	if(next_envid >= 0) {
	    cprintf("envrun real-time: %d [%08x]\n", next_envid, envs[next_envid].env_id);
	    env_run(&envs[next_envid]);    
	}

	for(i = 0; i < NENV; i++) {
        next_envid = (first_eid + i) % NENV;
        if (envs[next_envid].env_status == ENV_RUNNABLE) {
            cprintf("envrun RUNNABLE: %d [%08x]\n", next_envid, envs[next_envid].env_id);
            env_run(&envs[next_envid]);
            break;
        }
    }
    
    for(i = 0; i < NENV; i++) {
        next_envid = (first_eid + i) % NENV;
        if ((envs[next_envid].env_status == ENV_RUNNING) && (envs[next_envid].env_cpunum == thiscpu->cpu_id)) {
            cprintf("envrun RUNNING: %d [%08x]\n", next_envid, envs[next_envid].env_id);
            env_run(&envs[next_envid]);
            break;
        }
    }

	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	for(i = 0; i < NENV; ++i) {
		if (envs[i].env_status == ENV_BLOCKING) {
			envs[i].env_status = ENV_RUNNABLE;
			sched_yield();
		}
	}

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile (
		"movl $0, %%ebp\n"
		"movl %0, %%esp\n"
		"pushl $0\n"
		"pushl $0\n"
		"sti\n"
		"hlt\n"
	: : "a" (thiscpu->cpu_ts.ts_esp0));
}

