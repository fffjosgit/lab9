#include <inc/lib.h>

void (* volatile sys_yield)(void);
void (* volatile sys_write)(unsigned int x) __attribute__((regparm(1)));
int (* volatile env_getpid)(void);
int (* volatile cprintf)(const char *fmt, ...);

void
umain( int argc, char **argv )
{
   	cprintf("%d\n", env_getpid());
	sys_write( 5 );
	for(;;) {}
}

