// this is my simple scheduler testester

#include <inc/lib.h>

void
umain(int argc, char **argv)
{

	sys_make_me_real(10, 10, 10);

	cprintf("Hello, I am real-time environment %08x.\n", thisenv->env_id);
	
	int x[1000][1000];
	int y[1000][1000];
	int z[1000][1000];
	int i, j;
	int tmp;

	for(i = 0; i < 1000; i++) {
	    for(j = 0; j < 1000; j++) {
	            
	    }
	}

	cprintf("All done in environment %08x.\n", thisenv->env_id);
}