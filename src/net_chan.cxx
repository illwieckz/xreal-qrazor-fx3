/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================


/// includes ===================================================================
// system -------------------------------------------------------------------
#include <zlib.h>

// qrazor-fx ----------------------------------------------------------------
#include "net_chan.h"

#include "common.h"
#include "cvar.h"
#include "sys.h"


/*

packet header
-------------
31	sequence
1	does this message contain a reliable payload
31	acknowledge sequence
1	acknowledge receipt of even/odd message
16	qport 					 // Tr3B - removed

The remote connection never knows if it missed a reliable message, the
local side detects that it has been dropped by seeing a sequence acknowledge
higher thatn the last reliable sequence, but without the correct evon/odd
bit for the reliable set.

If the sender notices that a reliable message has been dropped, it will be
retransmitted.  It will not be retransmitted again until a message after
the retransmit has been acknowledged and the reliable still failed to get there.

if the sequence number is -1, the packet should be handled without a netcon

The reliable message can be added to at any time by doing
MSG_Write* (&netchan->message, <data>).

If the message buffer is overflowed, either by a single message, or by
multiple frames worth piling up while the last reliable transmit goes
unacknowledged, the netchan signals a fatal error.

Reliable messages are always placed first in a packet, then the unreliable
message is included if there is sufficient room.

To the receiver, there is no distinction between the reliable and unreliable
parts of the message, they are just processed out as a single larger message.

Illogical packet sequence numbers cause the packet to be dropped, but do
not kill the connection.  This, combined with the tight window of valid
reliable acknowledgement numbers provides protection against malicious
address spoofing.


The qport field is a workaround for bad address translating routers that
sometimes remap the client's source port on a packet during gameplay.

If the base part of the net address matches and the qport matches, then the
channel matches even if the IP port differs.  The IP port should be updated
to the new value before sending out any replies.


If there is no information that needs to be transfered on a given frame,
such as during the connection stage while waiting for the client to load,
then a packet only needs to be delivered if there is something in the
unacknowledged reliable
*/

static cvar_t*	net_showpackets;
static cvar_t*	net_showdrop;
static cvar_t*	net_qport;
static cvar_t*	net_compression;


/*
==============
Netchan_Setup

called to open a channel to a remote system
==============
*/
void	netchan_c::setup(const netadr_t &adr)
{
	memset(this, 0, sizeof(*this));
	
	_remote_address = adr;
	
	_last_received = Sys_Milliseconds();
	
	_incoming_sequence = 0;
	_outgoing_sequence = 1;

	message = message_c(MSG_TYPE_RAWBYTES, MAX_MSGLEN, true);
}



/*
===============
Netchan_Transmit

tries to send an unreliable message to a connection, and handles the
transmition / retransmition of the reliable messages.

A 0 length will still generate a packet and deal with the reliable messages.
================
*/
void 	netchan_c::transmit(const byte *data, int length)
{
	//
	// check for message overflow
	//
	if(message.isOverFlowed())
	{
		Com_Printf("%s:Outgoing message overflow\n", Sys_AdrToString(_remote_address));
		return;
	}

	bool send_reliable = needReliable();

	if(!_reliable_length && message.getCurSize())
	{
		memcpy(_reliable_buf, &message[0], message.getCurSize());
		_reliable_length = message.getCurSize();
		message.clear();
		_reliable_sequence ^= 1;
	}


	//
	// write the packet header
	//
	message_c packet(MSG_TYPE_RAWBYTES, MAX_PACKETLEN-16, false);

	unsigned long w1 = ( _outgoing_sequence & ~(1<<31) ) | (send_reliable<<31);
	unsigned long w2 = ( _incoming_sequence & ~(1<<31) ) | (_incoming_reliable_sequence<<31);

	_outgoing_sequence++;
	_last_sent = Sys_Milliseconds();

	packet.writeLong(w1);
	packet.writeLong(w2);


	//
	// copy the reliable message to the packet first
	//
	if(send_reliable)
	{
		if(net_compression->getInteger())
			packet.writeCompressedData(_reliable_buf, _reliable_length);
		else
			packet.writeRawData(_reliable_buf, _reliable_length);
			
		_last_reliable_sequence = _outgoing_sequence;
	}
	
	
	//
	// add the unreliable part if space is available
	//
	if(net_compression->getInteger())
	{
		if((packet.writeCompressedData(data, length, true)) == false)
			Com_Printf("netchan_c::transmit: dumped unreliable\n");
	}
	else
	{
		if((int)(packet.getMaxSize() - packet.getCurSize()) >= length)
			packet.writeRawData(data, length);
		else
			Com_Printf("netchan_c::transmit: dumped unreliable\n");
	}
	
	
	//
	// send the datagram
	//
	Sys_SendPacket(&packet[0], packet.getCurSize(), _remote_address);

	if(net_showpackets->getValue())
	{
		if(send_reliable)
			Com_Printf("send %4i : s=%i reliable=%i ack=%i rack=%i\n"
				, packet.getCurSize()
				, _outgoing_sequence - 1
				, _reliable_sequence
				, _incoming_sequence
				, _incoming_reliable_sequence);
		else
			Com_Printf("send %4i : s=%i ack=%i rack=%i\n"
				, packet.getCurSize()
				, _outgoing_sequence - 1
				, _incoming_sequence
				, _incoming_reliable_sequence);
	}
}


void	netchan_c::transmit(const message_c &msg)
{
	transmit((byte*)msg, msg.getCurSize());
}


/*
=================
Netchan_Process

called when the current net_message is from remote_address
modifies net_message so that it points to the packet payload
=================
*/
bool	netchan_c::process(message_c &msg)
{
	unsigned int	sequence, sequence_ack;
	unsigned int	reliable_ack, reliable_message;


	//
	// read the packet header and get sequence numbers		
	//
	msg.beginReading();
	sequence = msg.readLong();
	sequence_ack = msg.readLong();

	reliable_message = sequence >> 31;
	reliable_ack = sequence_ack >> 31;

	sequence &= ~(1<<31);
	sequence_ack &= ~(1<<31);
	
	
	//
	// uncompress the rest of the message
	//
	if(net_compression->getInteger())
	{
		msg.uncompress();
	}

	if(net_showpackets->getValue())
	{
		if(reliable_message)
			Com_Printf("recv %4i : s=%i reliable=%i ack=%i rack=%i\n"
				, msg.getCurSize()
				, sequence
				, _incoming_reliable_sequence ^ 1
				, sequence_ack
				, reliable_ack);
		else
			Com_Printf("recv %4i : s=%i ack=%i rack=%i\n"
				, msg.getCurSize()
				, sequence
				, sequence_ack
				, reliable_ack);
	}


	//
	// discard stale or duplicated packets
	//
	if((int)sequence <= _incoming_sequence)
	{
		if(net_showdrop->getValue())
			Com_Printf("%s:Out of order packet %i at %i\n"
				, Sys_AdrToString(_remote_address)
				,  sequence
				, _incoming_sequence);
		return false;
	}


	//
	// dropped packets don't keep the message from being used
	//
	_dropped = sequence - (_incoming_sequence+1);
	if(_dropped > 0)
	{
		if(net_showdrop->getValue())
			Com_Printf("%s:Dropped %i packets at %i\n"
			, Sys_AdrToString(_remote_address)
			, _dropped
			, sequence);
	}


	//
	// if the current outgoing reliable message has been acknowledged
	// clear the buffer to make way for the next
	//
	if((int)reliable_ack == _reliable_sequence)
		_reliable_length = 0;	// it has been received
	
	
	//
	// if this message contains a reliable message, bump incoming_reliable_sequence 
	//
	_incoming_sequence = sequence;
	_incoming_acknowledged = sequence_ack;
	_incoming_reliable_acknowledged = reliable_ack;
	if(reliable_message)
	{
		_incoming_reliable_sequence ^= 1;
	}


	//
	// the message can now be read from the current message pointer
	//
	_last_received = Sys_Milliseconds();

	return true;
}


bool	netchan_c::needReliable()
{
	// if the remote side dropped the last reliable message, resend it
	bool send_reliable = false;

	if(_incoming_acknowledged > _last_reliable_sequence && _incoming_reliable_acknowledged != _reliable_sequence)
		send_reliable = true;

	// if the reliable transmit buffer is empty, copy the current message out
	if(!_reliable_length && message.getCurSize())
	{
		send_reliable = true;
	}

	return send_reliable;
}



/*
===============
Netchan_CanReliable

Returns true if the last reliable message has acked
================
*/
bool	netchan_c::canReliable()
{
	if(_reliable_length)
		return false;			// waiting for ack
		
	return true;
}



void	Netchan_Init()
{
	// pick a port value that should be nice and random
	int port = Sys_Milliseconds() & 0xffff;

	net_showpackets	= Cvar_Get("net_showpackets", "0", CVAR_NONE);
	net_showdrop	= Cvar_Get("net_showdrop", "0", CVAR_NONE);
	net_qport 	= Cvar_Get("qport", va("%i", port), CVAR_INIT);
	net_compression	= Cvar_Get("net_compression", "1", CVAR_NONE);
}

/*
===============
Netchan_OutOfBand

Sends an out-of-band datagram
================
*/
void	Netchan_OutOfBand(const netadr_t &adr, const byte *data, int length)
{
	message_c send(MSG_TYPE_RAWBYTES, MAX_PACKETLEN);
	
	send.writeLong(-1);	// -1 sequence means out of band
	send.write(data, length);

	// send the datagram
	Sys_SendPacket(&send[0], send.getCurSize(), adr);
}

/*
===============
Netchan_OutOfBandPrint

Sends a text message in an out-of-band datagram
================
*/
void	Netchan_OutOfBandPrint(const netadr_t &adr, const char *format, ...)
{
	va_list		argptr;
	static char		string[MAX_PACKETLEN - 4];
	
	va_start(argptr, format);
	vsprintf(string, format,argptr);
	va_end(argptr);

	Netchan_OutOfBand(adr, (byte*)string, strlen(string));
}


