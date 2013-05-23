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

int get_highest_env(int x, int status) 
{
	int i, j; 
	int maxprio = -1; 
	int ret = -1;
		
	for(j = 0; j < NENV; j++) {
		if(j == x) {
			continue;
		} else {
		    i = (j + x) % NENV; 
		}
		if(envs[i].env_status == status) {
		    if(envs[i].env_priority == ENV_PRIORITY_HIGH) {
		        return i;
		    } else if(envs[i].env_priority > maxprio) {
		        maxprio = envs[i].env_priority;
		        ret = i;
		    } 
		}
	}
	return ret;
}

int get_rand(int status);
int get_rand(int status)
{
    srand(time_msec());

    int i, j; 
	int n = 0, maxprio = -1;

	for(i = 0; i < NENV; i++) {
	    if(envs[i].env_status == status) {
	        if(envs[i].env_priority > maxprio) {
	            n = 1;
	            maxprio = envs[i].env_priority; 
	        } else if(envs[i].env_priority == maxprio) {
	            n++;
	        }    
	    }    
	}

	n = rand() % n;
	
	for(i = 0; i < NENV; i++) {
	    if(envs[i].env_status == status) {
	        if(envs[i].env_priority == maxprio) {
	            n--;
	            if(!n) {
	                return i;
	            } 
	        }    
	    }    
	}
	return -1;

}

// Choose a user environment to run and run it.
//shed with priority
//random, if we have more then one highest priority env
void
sched_yield(void)
{
	struct Env *idle;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// LAB 2: Your code here.
	uint32_t envid = thiscpu->cpu_env ? ENVX(thiscpu->cpu_env->env_id) : 0;
	uint32_t first_eid = (++envid) % NENV;
	int next_envid;
	int i;

	//perform_io_simulation();

	/*if((next_envid = get_highest_env(envid, ENV_RUNNABLE)) >= 0) {
	    cprintf("envrun RUNNABLE: %08x with priority: %d\n", next_envid, envs[next_envid].env_priority);
	    env_run(&envs[next_envid]);    
	}*/

	/*if((next_envid = get_highest_env(envid, ENV_RUNNING)) >= 0) {
	    cprintf("envrun RUNNING: %08x with priority: %d\n", next_envid, envs[next_envid].env_priority);
	    env_run(&envs[next_envid]);    
	}*/
	
	/*for (i = 0; i < NENV; i++) {
		next_envid = (first_eid + i) % NENV;
		if (envs[next_envid].env_status == ENV_RUNNABLE) {
			cprintf("envrun RUNNABLE: %d\n", next_envid);
			env_run(&envs[next_envid]);
			break;
		}
	}*/
    
    if((next_envid = get_rand(ENV_RUNNABLE)) >= 0) {
	    cprintf("envrun RUNNABLE: %08x with priority: %d\n", next_envid, envs[next_envid].env_priority);
	    env_run(&envs[next_envid]);    
	}

    for (i = 0; i < NENV; i++) {
        next_envid = (first_eid + i) % NENV;
        if ((envs[next_envid].env_status == ENV_RUNNING) && (envs[next_envid].env_cpunum == thiscpu->cpu_id)) {
            cprintf("envrun RUNNING: %d\n", next_envid);
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

