/**
 * $Id$
 * Copyright (C) 2008 - 2011 Nils Asmussen
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

#include <sys/common.h>
#include <sys/arch/i586/fpu.h>
#include <sys/task/smp.h>
#include <sys/task/thread.h>
#include <sys/mem/cache.h>
#include <sys/video.h>
#include <sys/cpu.h>
#include <sys/util.h>
#include <string.h>

extern void fpu_finit(void);
extern void fpu_saveState(sFPUState *state);
extern void fpu_restoreState(sFPUState *state);

/* current FPU state-memory */
static sFPUState ***curStates = NULL;

void fpu_init(void) {
	uint32_t cr0 = cpu_getCR0();
	/* enable coprocessor monitoring */
	cr0 |= CR0_MONITOR_COPROC;
	/* disable emulate */
	cr0 &= ~CR0_EMULATE;
	cpu_setCR0(cr0);

	/* allocate a state-pointer for each cpu */
	curStates = (sFPUState***)cache_calloc(smp_getCPUCount(),sizeof(sFPUState**));
	if(!curStates)
		util_panic("Unable to allocate memory for FPU-states");

	/* TODO check whether we have a FPU */
	/* set the OSFXSR bit
	cpu_setCR4(cpu_getCR4() | 0x200);*/
	/* init the fpu */
	fpu_finit();
}

void fpu_lockFPU(void) {
	/* set the task-switched-bit in CR0. as soon as a process uses any FPU instruction
	 * we'll get a EX_CO_PROC_NA and call fpu_handleCoProcNA() */
	cpu_setCR0(cpu_getCR0() | CR0_TASK_SWITCHED);
}

void fpu_handleCoProcNA(sFPUState **state) {
	sThread *t = thread_getRunning();
	sFPUState **current = curStates[t->cpu];
	if(current != state) {
		/* if any process has used the FPU in the past */
		if(current != NULL) {
			/* do we have to allocate space for the state? */
			if(*current == NULL) {
				*current = (sFPUState*)cache_alloc(sizeof(sFPUState));
				/* if we can't save the state, don't unlock the FPU for another process */
				/* TODO ok? */
				if(*current == NULL)
					return;
			}
			/* unlock FPU (necessary here because otherwise fpu_saveState would cause a #NM) */
			cpu_setCR0(cpu_getCR0() & ~CR0_TASK_SWITCHED);
			/* save state */
			fpu_saveState(*current);
		}
		else
			cpu_setCR0(cpu_getCR0() & ~CR0_TASK_SWITCHED);

		/* init FPU for new process */
		curStates[t->cpu] = state;
		if(*state != NULL)
			fpu_restoreState(*state);
		else
			fpu_finit();
	}
	/* just unlock */
	else
		cpu_setCR0(cpu_getCR0() & ~CR0_TASK_SWITCHED);
}

void fpu_cloneState(sFPUState **dst,const sFPUState *src) {
	if(src != NULL) {
		*dst = (sFPUState*)cache_alloc(sizeof(sFPUState));
		/* simply ignore it here if alloc fails */
		if(*dst)
			memcpy(*dst,src,sizeof(sFPUState));
	}
	else
		*dst = NULL;
}

void fpu_freeState(sFPUState **state) {
	size_t i,n;
	if(*state != NULL)
		cache_free(*state);
	*state = NULL;
	/* we have to unset the current state because maybe the next created process gets
	 * the same slot, so that the pointer is the same. */
	for(i = 0, n = smp_getCPUCount(); i < n; i++) {
		if(curStates[i] == state) {
			curStates[i] = NULL;
			break;
		}
	}
}
