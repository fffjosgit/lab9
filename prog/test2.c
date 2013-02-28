#include <inc/lib.h>

//extern void *find_symbol( char *name ) __attribute__((weak));
volatile int (*cprintf) (const char *fmt, ...) __attribute__((weak));

void
umain( int argc, char **argv )
{
   int i;

   cprintf = (void *) NULL;

   cprintf( "HERE\n" );

   for( i = 0; i < 10000; ++i ) {}
}

