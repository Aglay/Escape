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

#include <esc/ipc/clientdevice.h>
#include <sys/common.h>
#include <sys/io.h>
#include <sys/messages.h>
#include <sys/mman.h>
#include <assert.h>
#include <list>
#include <string.h>

#include "../common.h"
#include "../packet.h"

class Socket : public esc::Client {
public:
	struct QueuedPacket {
		std::shared_ptr<PacketData> data;
		size_t offset;
		esc::Socket::Addr sa;

		explicit QueuedPacket(std::shared_ptr<PacketData> _data,size_t _offset,
				const esc::Socket::Addr &_sa)
			: data(_data), offset(_offset), sa(_sa) {
		}
	};

	explicit Socket(int f,int proto = esc::Socket::PROTO_ANY)
		: esc::Client(f), _proto(proto), _pending() {
	}
	virtual ~Socket() {
	}

	int protocol() const {
		return _proto;
	}

	virtual int cancel(msgid_t mid) {
		if(!_pending.count)
			return esc::DevCancel::READY;
		if(_pending.mid != mid)
			return -EINVAL;
		_pending.count = 0;
		return esc::DevCancel::CANCELED;
	}

	virtual int connect(const esc::Socket::Addr *,msgid_t) {
		return -ENOTSUP;
	}
	virtual int bind(const esc::Socket::Addr *) {
		return -ENOTSUP;
	}
	virtual int listen() {
		return -ENOTSUP;
	}
	virtual int accept(msgid_t,int,esc::ClientDevice<Socket> *) {
		return -ENOTSUP;
	}
	virtual ssize_t sendto(msgid_t,const esc::Socket::Addr *,const void *,size_t) {
		return -ENOTSUP;
	}
	virtual int abort() {
		return -ENOTSUP;
	}
	virtual void disconnect() {
		delete this;
	}

	virtual ssize_t recvfrom(msgid_t mid,bool needsSrc,void *buffer,size_t size) {
		if(size == 0)
			return -EINVAL;

		if(_packets.size() > 0) {
			const QueuedPacket &pkt = _packets.front();
			size_t cpysize = pkt.data->size - pkt.offset;
			if(cpysize <= size) {
				if(buffer != NULL)
					memcpy(buffer,pkt.data->data + pkt.offset,cpysize);
				reply(mid,pkt.sa,needsSrc,buffer,pkt.data->data + pkt.offset,cpysize);
				_packets.pop_front();
				return 0;
			}
		}

		if(_pending.count > 0)
			return -EAGAIN;
		_pending.mid = mid;
		_pending.count = size;
		_pending.d.read.data = buffer;
		_pending.d.read.needsSrc = needsSrc;
		return 0;
	}

	virtual void push(const esc::Socket::Addr &sa,const Packet &pkt,size_t offset = 0) {
		if(_pending.count > 0) {
			if(_pending.count >= pkt.size() - offset) {
				if(_pending.d.read.data != NULL)
					memcpy(_pending.d.read.data,pkt.data<uint8_t*>() + offset,pkt.size() - offset);
				reply(_pending.mid,sa,_pending.d.read.needsSrc,_pending.d.read.data,
					pkt.data<uint8_t*>() + offset,pkt.size() - offset);
				_pending.count = 0;
			}
		}
		else
			_packets.push_back(QueuedPacket(pkt.copy(),offset,sa));
	}

protected:
	void reply(msgid_t mid,const esc::Socket::Addr &sa,bool needsSrc,void *dst,const void *src,ssize_t size) {
		ulong buffer[IPC_DEF_SIZE / sizeof(ulong)];
		esc::IPCStream is(fd(),buffer,sizeof(buffer),mid);
		is << esc::FileRead::Response::result(size);
		if(needsSrc)
			is << sa;
		is << esc::Reply();
		if(size > 0 && dst == NULL)
			is << esc::ReplyData(src,size);
	}

	int _proto;
	PendingRequest _pending;
	std::list<QueuedPacket> _packets;
};
