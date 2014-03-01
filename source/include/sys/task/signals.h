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

#pragma once

#include <sys/common.h>
#include <sys/spinlock.h>

#define SIG_COUNT			10

#define SIG_IGN				((Signals::handler_func)-3)			/* ignore signal */
#define SIG_DFL				((Signals::handler_func)-2)			/* reset to default behaviour */
#define SIG_ERR				((Signals::handler_func)-1)			/* error-return */

/* the signals */
#define SIG_RET				-1						/* used to tell the kernel the addr of sigRet */
#define SIG_KILL			0						/* kills a proc; not catchable */
#define SIG_TERM			1						/* terminates a proc; catchable */
#define SIG_ILL_INSTR		2						/* TODO atm unused */
#define SIG_SEGFAULT		3						/* segmentation fault */
#define SIG_PIPE_CLOSED		4						/* sent to the pipe-writer when reader died */
#define SIG_CHILD_TERM		5						/* sent to parent-proc */
#define SIG_INTRPT			6						/* used to interrupt a process; used by shell */
#define SIG_ALARM			7						/* for alarm() */
#define SIG_USR1			8						/* can be used for everything */
#define SIG_USR2			9						/* can be used for everything */

#define SIG_CHECK_CUR		0
#define SIG_CHECK_OTHER		1
#define SIG_CHECK_NO		2

class Thread;
class ThreadBase;
class OStream;

class Signals {
	friend class ThreadBase;

	Signals() = delete;

public:
	/* signal-handler-signature */
	typedef void (*handler_func)(int);

private:
	struct PendingSig {
		int sig;
		PendingSig *next;
	};

	struct PendingQueue {
		PendingSig *first;
		PendingSig *last;
		size_t count;
	};

	static const size_t SIGNAL_COUNT	= 8192;

public:
	/**
	 * Initializes the signal-handling
	 */
	static void init();

	/**
	 * Checks whether we can handle the given signal
	 *
	 * @param signal the signal
	 * @return true if so
	 */
	static bool canHandle(int signal);

	/**
	 * Checks whether the given signal can be send by user-programs
	 *
	 * @param signal the signal
	 * @return true if so
	 */
	static bool canSend(int signal);

	/**
	 * @param sig the signal
	 * @return whether the given signal is a fatal one, i.e. should kill the process
	 */
	static bool isFatal(int sig);

	/**
	 * Sets the given signal-handler for <signal>
	 *
	 * @param tid the thread-id
	 * @param signal the signal
	 * @param func the handler-function
	 * @param old will be set to the old handler
	 * @return 0 on success
	 */
	static int setHandler(tid_t tid,int signal,handler_func func,handler_func *old);

	/**
	 * Removes the signal-handler for <signal>
	 *
	 * @param tid the thread-id
	 * @param signal the signal
	 * @return the old handler
	 */
	static handler_func unsetHandler(tid_t tid,int signal);

	/**
	 * Removes all handler for the given thread
	 *
	 * @param tid the thread-id
	 */
	static void removeHandlerFor(tid_t tid);

	/**
	 * Clones all handler of <parent> for <child>.
	 *
	 * @param parent the parent-thread-id
	 * @param child the child-thread-id
	 */
	static void cloneHandler(tid_t parent,tid_t child);

	/**
	 * Checks whether <tid> has a signal
	 *
	 * @param tid the thread-id
	 * @return true if so
	 */
	static bool hasSignalFor(tid_t tid);

	/**
	 * Checks whether the current thread should handle a signal. If so, it sets *sig and *handler
	 * correspondingly.
	 *
	 * @param tid the thread-id of the current thread
	 * @param sig will be set to the signal to handle, if the current thread has a signal
	 * @param handler will be set to the handler, if the current thread has a signal
	 * @return true if there is a signal to handle
	 */
	static bool checkAndStart(tid_t tid,int *sig,handler_func *handler);

	/**
	 * Adds the given signal for the given thread
	 *
	 * @param tid the thread-id
	 * @param signal the signal
	 * @return true if the signal has been added
	 */
	static bool addSignalFor(tid_t tid,int signal);

	/**
	 * Adds the given signal to all threads that have announced a handler for it
	 *
	 * @param signal the signal
	 * @return whether the signal has been delivered to somebody
	 */
	static bool addSignal(int signal);

	/**
	 * Acknoledges the current signal with given thread (marks handling as finished)
	 *
	 * @param tid the thread-id
	 * @return the handled signal
	 */
	static int ackHandling(tid_t tid);

	/**
	 * @return the total number of announced handlers
	 */
	static size_t getHandlerCount();

	/**
	 * @param signal the signal-number
	 * @return the name of the given signal
	 */
	static const char *getName(int signal);

	/**
	 * Prints all announced signal-handlers
	 *
	 * @param os the output-stream
	 */
	static void print(OStream &os);

private:
	static bool add(Thread *t,int sig);
	static void removePending(Thread *t,int sig);
	static Thread *getThread(tid_t tid);

	static SpinLock lock;
	static SpinLock listLock;
	static PendingSig signals[SIGNAL_COUNT];
	static PendingSig *freelist;
};

inline bool Signals::canHandle(int signal) {
	/* we can't add a handler for SIG_KILL */
	return signal >= 1 && signal < SIG_COUNT;
}

inline bool Signals::canSend(int signal) {
	return signal = SIG_KILL || signal == SIG_TERM || signal == SIG_INTRPT ||
			signal == SIG_USR1 || signal == SIG_USR2;
}

inline bool Signals::isFatal(int sig) {
	return sig == SIG_INTRPT || sig == SIG_TERM || sig == SIG_KILL || sig == SIG_SEGFAULT ||
		sig == SIG_PIPE_CLOSED;
}
