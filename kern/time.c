#include <kern/time.h>
#include <inc/assert.h>

static unsigned int ticks;

#define PERIOD 10

void
time_init(void)
{
	ticks = 0;
}

void
time_tick(void)
{
	ticks++;
	if(ticks * PERIOD < ticks)
		panic("time_tick: time overflowed");
}

unsigned int
time_msec(void)
{
	return ticks * PERIOD;
}
