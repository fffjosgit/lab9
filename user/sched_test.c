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
	int i, j, k;
	int tmp;

	int t;
	for(t = 0; t < 1000000; t++)
	
	for(i = 0; i < 1000; i++) {
	    for(j = 0; j < 1000; j++) {
	        z[i][j] = 0;
	        for(k = 0; k < 1000; k++) {
	            z[i][j] += x[i][k] * y[k][j];
	        }	            
	    }
	}

	cprintf("All done in environment %08x.\n", thisenv->env_id);
}