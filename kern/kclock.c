/* See COPYRIGHT for copyright information. */

/* Support for reading the NVRAM from the real-time clock. */

#include <inc/x86.h>
#include <inc/stdio.h>
#include <kern/kclock.h>


#define NMI_LOCK	0x80

static inline void
nmi_enable(void)
{
	outb(0x70, inb(0x70) & ~NMI_LOCK );
}


unsigned
mc146818_read(unsigned reg)
{
	outb(IO_RTC, reg);
	return inb(IO_RTC+1);
}

void
mc146818_write(unsigned reg, unsigned datum)
{
	outb(IO_RTC, reg);
	outb(IO_RTC+1, datum);
}

void
rtc_init(void)
{
	uint8_t prev;

	outb(IO_RTC_CMND, NMI_LOCK | RTC_BREG);
	prev = inb(IO_RTC_DATA);
	outb(IO_RTC_CMND, NMI_LOCK | RTC_BREG);
	outb(IO_RTC_DATA, prev | RTC_PIE);
	
	outb(IO_RTC_CMND, NMI_LOCK | RTC_AREG);
	prev = inb(IO_RTC_DATA);
	outb(IO_RTC_CMND, NMI_LOCK | RTC_AREG);
	outb(IO_RTC_DATA, prev | 0xF);
	

	nmi_enable();
}

uint8_t
rtc_check_status(void)
{
	outb(IO_RTC_CMND, RTC_CREG);
	return inb(IO_RTC_DATA);
}
