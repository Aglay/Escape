/**
 * $Id$
 * Copyright (C) 2008 - 2014 Nils Asmussen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <esc/common.h>
#include <esc/driver.h>
#include <esc/io.h>
#include <esc/debug.h>
#include <esc/proc.h>
#include <esc/thread.h>
#include <esc/keycodes.h>
#include <esc/messages.h>
#include <esc/ringbuffer.h>
#include <esc/driver/vterm.h>
#include <esc/mem.h>
#include <esc/irq.h>
#include <keyb/keybdev.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "set2.h"

#define KEYBOARD_BASE		0x30200000

#define KEYBOARD_CTRL		0
#define KEYBOARD_DATA		1

#define KEYBOARD_RDY		0x01
#define KEYBOARD_IEN		0x02

#define BUF_SIZE			128
#define SC_BUF_SIZE			16
#define TIMEOUT				60
#define SLEEP_TIME			20

static int kbIrqThread(A_UNUSED void *arg);

static uint32_t *kbRegs;

int main(void) {
	uintptr_t phys = KEYBOARD_BASE;
	kbRegs = (uint32_t*)mmapphys(&phys,2 * sizeof(uint32_t),0);
	if(kbRegs == NULL)
		error("Unable to map keyboard registers");

	if(startthread(kbIrqThread,NULL) < 0)
		error("Unable to start irq-thread");

	/* enable interrupts */
	kbRegs[KEYBOARD_CTRL] |= KEYBOARD_IEN;

	keyb_driverLoop();

	/* clean up */
	munmap(kbRegs);
	return EXIT_SUCCESS;
}

static int kbIrqThread(A_UNUSED void *arg) {
	int sem = semcrtirq(IRQ_SEM_KEYB);
	if(sem < 0)
		error("Unable to get irq-semaphore for IRQ %d",IRQ_SEM_KEYB);
	while(1) {
		semdown(sem);

		uint32_t sc = kbRegs[KEYBOARD_DATA];
		sKbData data;
		if(kb_set2_getKeycode(&data.isBreak,&data.keycode,sc))
			keyb_broadcast(&data);

		/* re-enable interrupts; the kernel has disabled it in order to get into userland */
		kbRegs[KEYBOARD_CTRL] |= KEYBOARD_IEN;
	}
	return 0;
}
