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

#ifndef DRIVER_VTERM_H_
#define DRIVER_VTERM_H_

#include <esc/common.h>
#include <esc/messages.h>

#ifdef __cplusplus
extern "C" {
#endif

int vterm_setEnabled(int fd,bool enabled);
int vterm_setEcho(int fd,bool echo);
int vterm_setReadline(int fd,bool readline);
int vterm_setReadKB(int fd,bool read);
int vterm_setNavi(int fd,bool navi);
int vterm_backup(int fd);
int vterm_restore(int fd);
int vterm_setShellPid(int fd,pid_t pid);
int vterm_getSize(int fd,sVTSize *size);
int vterm_select(int fd,int vterm);

#ifdef __cplusplus
}
#endif

#endif /* DRIVER_VTERM_H_ */