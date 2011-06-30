/**
 * $Id$
 * Copyright (C) 2008 - 2009 Nils Asmussen
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
#include <esc/messages.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

int isatty(int fd) {
	sVTSize consSize;
	int res = recvMsgData(fd,MSG_VT_GETSIZE,&consSize,sizeof(sVTSize)) >= 0;
	/* this is not considered as an error in this case */
	/* note that we include no-exec-perm here because we assume that we can always send messages
	 * with stdin when it is connected to a vterm. */
	if(errno == ERR_UNSUPPORTED_OP || errno == ERR_NO_EXEC_PERM)
		errno = 0;
	return res;
}
