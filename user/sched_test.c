// this is my simle scheduler test

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	int i;

	sys_make_me_real(10, 10, 10);

	cprintf("Hello, I am real-time environment %08x.\n", thisenv->env_id);
	
	for(;;) {
	}

	cprintf("All done in environment %08x.\n", thisenv->env_id);
}