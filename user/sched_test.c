// this is my simle scheduler test

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int i;

	cprintf("Hello, I am environment %08x.\n", thisenv->env_id);
	
	sys_make_me_real(100, 100, 100);
	
	for(;;) {
	}

	cprintf("All done in environment %08x.\n", thisenv->env_id);
}