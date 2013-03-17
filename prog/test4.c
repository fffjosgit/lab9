#include <inc/lib.h>
#include <inc/x86.h>

int (* volatile cprintf) (const char *fmt, ...);
void * (* volatile test_alloc) (uint8_t nbytes);
void (* volatile test_free) (void *ap);

void (* volatile sys_yield)(void);

int deep = 0;

void
test_rec( void )
{
	void *buf;

	if ( !(read_tsc() % 7) ) {
		sys_yield();
	}

	deep++;
	buf = test_alloc( (deep % 3) ? 15 : 29 );
	if ( buf ) {
		if (deep <= 200) {
			test_rec();
			deep--;
		}
		test_free( buf );
	} else {
		return;
	}
}

void
umain( int argc, char **argv )
{
	for(;;)
		test_rec();
}
