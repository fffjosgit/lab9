// this is my simple scheduler testester

#include <inc/lib.h>
#include <inc/random.h>

void
umain(int argc, char **argv)
{

	sys_make_me_real(300, 100, 300);

	cprintf("Hello, I am real-time environment [%08x].\n", thisenv->env_id);
	
	int x[1000][1000];
	int y[1000][1000];
	int z[1000][1000];
	int i, j, k, l;
	int tmp;

	
	for(l = 0; l < 10; l++) {
	
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

	    //cprintf("X \n");

	    sys_work_done();
	    
	    sys_sleep(20);    	
    }

	cprintf("All done in environment [%08x].\n", thisenv->env_id);
}