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

#include <sys/common.h>

#include "../../CPUInfo.h"

using namespace esc;

uint64_t MMIXCPUInfo::getRN() const {
	uint64_t rn;
	asm volatile ("GET %0,rN" : "=r"(rn));
	return rn;
}

void MMIXCPUInfo::print(esc::OStream &os,info::cpu &cpu) {
	uint64_t rn = getRN();
	os << fmt("Speed:","-",12) << cpu.speed() << " Hz\n";
	os << fmt("Vendor:","-",12) << "THM\n";
	os << fmt("Model:","-",12) << "GIMMIX\n";
	os << fmt("Version:","-",12)
	   << (rn >> 56) << "." << ((rn >> 48) & 0xFF) << "." << ((rn >> 40) & 0xFF) << "\n";
	os << fmt("Builddate:","-",12) << (rn & 0xFFFFFFFFFF) << "\n";
}
