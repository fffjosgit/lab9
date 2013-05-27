// this is my simple scheduler testester

#include <inc/lib.h>
#include <inc/random.h>

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

	srand(1);
	for(i = 0; i < 1000; i++) {
	    for(j = 0; j < 1000; j++) {
	        x[i][j] = rand();
	        y[i][j] = rand();	        
	    }
	}

	for(i = 0; i < 1000; i++) {
	    for(j = 0; j < 1000; j++) {
	        z[i][j] = 0;
	        for(k = 0; k < 1000; k++) {
	            z[i][j] += x[i][k] * y[k][j];
	        }	            
	    }
	}

	pause(1000);

	cprintf("All done in environment %08x.\n", thisenv->env_id);
}