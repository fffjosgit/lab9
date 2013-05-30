// this is my simple scheduler testester

#include <inc/lib.h>
#include <inc/random.h>
#include <inc/x86.h>



void
umain(int argc, char **argv)
{

        //int t = thisenv->env_id;

        srand(read_tsc());
        int t = 1 + rand() % 5;
	
	sys_make_me_real(t * 100, t * 5, t * 100);

	cprintf("Hello, I am real-time environment [%08x].\n", thisenv->env_id);
	
	int x[1000][1000];
	int y[1000][1000];
	int z[1000][1000];
	int i, j, k, l;
	int tmp;

	
	for(l = 0; l < 10; l++) {

	    //sys_sleep(5);
	
    	cprintf("[%08x]: A \n", thisenv->env_id);
    	
    	srand(read_tsc());

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

	    cprintf("[%08x]: B \n", thisenv->env_id);
	    sys_work_done();	    
	    
	    //sys_sleep(20);    	
    }

	cprintf("All done in environment [%08x].\n", thisenv->env_id);
}
