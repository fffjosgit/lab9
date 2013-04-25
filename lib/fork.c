// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *)utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int ret;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.

	if (!((err & FEC_WR) && (vpt[VPN((unsigned int)addr)] & PTE_COW))) {
		panic("not a write and not to a COW page, addr: %x, err: %x", addr, err);
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.

	envid_t envid = sys_getenvid();
	void *va = ROUNDDOWN(addr, PGSIZE);
	
	if ((ret = sys_page_alloc(envid, (void *)PFTEMP, PTE_U | PTE_W | PTE_P)) < 0) {
		//panic("sys_page_alloc error: %e", r);
		panic("sys_page_alloc error");
	}
	memmove((void *)PFTEMP, va, PGSIZE);
	if ((ret = sys_page_map(envid, (void *)PFTEMP, envid, va, PTE_U | PTE_W | PTE_P)) < 0)  {
		//panic("sys_page_map error: %e", r);
		panic("sys_page_map error");
	}
	if ((ret = sys_page_unmap(envid, (void *)PFTEMP)) < 0) {
		//panic("sys_page_unmap error: %e", r);
		panic("sys_page_unmap error");
	}

	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//

//i have some questions!!!!!!!!!!!!!
static int
duppage(envid_t envid, unsigned pn)
{
	int ret;
	pte_t pte = vpn[pn];
	unsigned int perm = 0;
	void *va = (void *)(pn << PGSHIFT);

	if (!(pte & PTE_P)) {
		return -E_INVAL;
	}

	/*if ((pte & PTE_W) && (pte & PTE_COW)) {
        panic("PTE_W & PTE_COW\n");
    }*/

	if (!(pte & PTE_SHARE) && ((pte & PTE_W) || (pte & PTE_COW))) {
	    if ((ret = sys_page_map(curenv->env_id, va, envid, va, PTE_U | PTE_P | PTE_COW)) < 0) {
		    //panic("sys_page_map error: %e", r);
			panic("sys_page_map error");
	    }
	    //if ((ret = sys_page_map(curenv->envid, va, curenv->envid, va, PTE_U | PTE_P | PTE_COW)) < 0) {
	    if ((ret = sys_page_map(envid, va, curenv->envid, va, PTE_U | PTE_P | PTE_COW)) < 0) {
		    //panic("sys_page_map error: %e", r);
			panic("sys_page_map error");
	    }
	} else {
	    if ((ret = sys_page_map(curenv->env_id, va, envid, va, pte & PTE_SYSCALL)) < 0) {
			//panic("sys_page_map error: %e", r);
			panic("sys_page_map error");
		}
	}
	

	return 0;

	// LAB 4: Your code here.
	//panic("duppage not implemented");
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	envid_t envid;
	int ret;
	int pn, i;

	set_pgfault_handler(pgfault);

	if((envid = sys_exofork()) < 0) {
	    //panic("sys_exofork: error %e\n", envid);
	    panic("fork: sys_exofork error");
	}

	if(envid == 0) {
	    thisenv = &envs[ENVX(sys_getenvid())];
	    return 0;
	}

	pn = UTOP / PGSIZE - 1;
	//? what is this????
	while (--pn >= 0) {
		if (!(vpd[pn >> 10] & PTE_P)) { 
			pn = (pn >> 10) << 10;
	    } else if (vpt[pn] & PTE_P) {
			duppage(envid, pn);
		}
    }

	if ((ret = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_W |PTE_U |PTE_P)) < 0) {
		//panic("sys_page_alloc error: %e", r);
		panic("fork: sys_page_alloc error");
    }

    if ((ret = sys_env_set_pgfault_upcall(envid, thisenv->env_pgfault_upcall)) < 0) {
        //panic("sys_env_set_pgfault_upcall: error %e\n", r);
        panic("fork: sys_env_set_pgfault_upcall error");
    }

    if ((ret = sys_env_set_status(envid, ENV_RUNNABLE)) < 0) {
		//panic("sys_env_set_status: %e", r);
		panic("fork: sys_env_set_statuserror");
	}

	return envid;
	
	// LAB 4: Your code here.
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
