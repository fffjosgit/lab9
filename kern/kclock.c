/* See COPYRIGHT for copyright information. */

/* Support for reading the NVRAM from the real-time clock. */

#include <inc/x86.h>
#include <kern/kclock.h>


unsigned
mc146818_read(unsigned reg)
{
	outb(IO_RTC_CMND, reg);
	return inb(IO_RTC_DATA);
}

void
mc146818_write(unsigned reg, unsigned datum)
{
	outb(IO_RTC_CMND, reg);
	outb(IO_RTC_DATA, datum);
}

void
rtc_init(void)
{
	uint8_t prev;

	nmi_enable();
}

uint8_t
rtc_check_status(void)
{
	outb(IO_RTC_CMND, RTC_CREG);
	return inb(IO_RTC_DATA);
}

