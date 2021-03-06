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

#include <gui/panel.h>
#include <gui/wrapper.h>
#include <sys/common.h>
#include <iomanip>

using namespace std;

namespace gui {
	bool Wrapper::layout() {
		_doingLayout = true;
		bool res = _ctrl->layout();
		_doingLayout = false;
		if(res)
			getParent()->layoutChanged();
		return res;
	}

	void Wrapper::layoutChanged() {
		if(_doingLayout)
			return;
		if(layout())
			repaint();
	}

	void Wrapper::print(esc::OStream &os, bool rec, size_t indent) const {
		UIElement::print(os,rec,indent);
		if(rec) {
			os << " {\n";
			_ctrl->print(os,rec,indent + 2);
			os << '\n' << esc::fmt("",indent) << "}";
		}
	}
}
