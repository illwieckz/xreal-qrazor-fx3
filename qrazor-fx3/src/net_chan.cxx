/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#include "x_protocol.h"
#include "net_chan.h"

#include "common.h"
#include "cvar.h"
#include "sys.h"


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
void	netchan_c::setup(const netadr_t &adr, int qport)
{
	memset(this, 0, sizeof(*this));
	
	_remote_address = adr;
	_qport = qport;
	
	_last_received = Sys_Milliseconds();
	
	_incoming_sequence = 0;
	_outgoing_sequence = 1;

	message = bitmessage_c(toBits(MAX_MSGLEN)-NETCHAN_PACKET_HEADER_BITS, true);
}



/*
===============
Netchan_Transmit

tries to send an unreliable message to a connection, and handles the
transmition / retransmition of the reliable messages.

A 0 length will still generate a packet and deal with the reliable messages.
================
*/
void 	netchan_c::transmit(const bitmessage_c &msg)
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

	if(!_outgoing_reliable_buf.size() && message.getCurSize())
	{
		message.copyTo(_outgoing_reliable_buf, 0, message.getCurSize());
		message.beginWriting();
		_outgoing_reliable ^= true;
	}


	//
	// write the packet header
	//
	bitmessage_c	packet((MAX_PACKETLEN*8), false);

	packet.writeBits(_outgoing_sequence, NETCHAN_PACKET_HEADER_BITS_SEQUENCE);
	packet.writeBit(send_reliable);
	
	packet.writeBits(_incoming_sequence, NETCHAN_PACKET_HEADER_BITS_SEQUENCE_ACK);
	packet.writeBit(_incoming_reliable);
	
	packet.writeBits(0, NETCHAN_PACKET_HEADER_BITS_UNCOMPRESSED_SIZE);	// dummy value to correct later
	packet.writeBits(0, NETCHAN_PACKET_HEADER_BITS_COMPRESSED_SIZE);	// - " -
	
	packet.writeBits(0, NETCHAN_PACKET_HEADER_BITS_CHECKSUM);		// - " -
	
	packet.writeBits(net_qport->getInteger(), NETCHAN_PACKET_HEADER_BITS_QPORT);
	
	
	//
	// track packet sizes
	//
	uint_t		packet_size_uncompressed = packet.getCurSize();
	uint_t		packet_size_compressed = packet.getCurSize();
	
	
	//
	// update current state
	//
	_outgoing_sequence++;
	_last_sent = Sys_Milliseconds();
	

	//
	// copy the reliable message to the packet first
	//
	if(send_reliable)
	{
		packet_size_uncompressed += _outgoing_reliable_buf.size();
	
		if(net_compression->getInteger())
		{
			packet_size_compressed += packet.writeBitsCompressed(_outgoing_reliable_buf);
		}
		else
		{
			packet_size_compressed += _outgoing_reliable_buf.size();
			packet.writeBits(_outgoing_reliable_buf);
		}
			
		_outgoing_reliable_last = _outgoing_sequence;
	}
	
	
	//
	// add the unreliable part if space is available
	//
	#if 1
	if(net_compression->getInteger())
	{
		try
		{
			packet_size_uncompressed += msg.getCurSize();
			packet_size_compressed += packet.writeMessageCompressed(msg, true);
		}
		catch(...)
		{
			Com_Printf("netchan_c::transmit: dumped unreliable with %i bits\n", msg.getCurSize());
		}
	}
	else
	{
		if((packet.getMaxSize() - packet.getCurSize()) >= msg.getCurSize())
		{
			packet_size_uncompressed += msg.getCurSize();
			packet_size_compressed += msg.getCurSize();
			packet.writeMessage(msg);
		}
		else
		{
			Com_Printf("netchan_c::transmit: dumped unreliable with %i bits\n", msg.getCurSize());
		}
	}
	#endif
	
	//
	// correct packet header
	//
	if(packet_size_compressed != packet.getCurSize())
		Com_Error(ERR_DROP, "netchan_c::transmit: corrupt packet size %i != %i", packet_size_compressed, packet.getCurSize());
		
	int checksum = packet.calcCheckSum(NETCHAN_PACKET_HEADER_BITS);
	
	packet.setCurSize(NETCHAN_PACKET_HEADER_BITS_SIZE_OFFSET);
	packet.writeBits(packet_size_uncompressed, NETCHAN_PACKET_HEADER_BITS_UNCOMPRESSED_SIZE);
	packet.writeBits(packet_size_compressed, NETCHAN_PACKET_HEADER_BITS_COMPRESSED_SIZE);
	packet.writeBits(checksum, NETCHAN_PACKET_HEADER_BITS_CHECKSUM);
	packet.setCurSize(packet_size_compressed);
	
	
	//
	// send the datagram
	//
	Sys_SendPacket(packet, _remote_address);

	if(net_showpackets->getValue())
	{
		if(send_reliable)
			Com_Printf("send %5i : s=%i reliable=%i ack=%i rack=%i saved=%i\n"
				, packet.getCurSize()
				, _outgoing_sequence - 1
				, _outgoing_reliable
				, _incoming_sequence
				, _incoming_reliable
				, packet_size_uncompressed - packet_size_compressed);
		else
			Com_Printf("send %5i : s=%i ack=%i rack=%i saved=%i\n"
				, packet.getCurSize()
				, _outgoing_sequence - 1
				, _incoming_sequence
				, _incoming_reliable
				, packet_size_uncompressed - packet_size_compressed);
	}
}


/*
=================
Netchan_Process

called when the current net_message is from remote_address
modifies net_message so that it points to the packet payload
=================
*/
bool	netchan_c::process(bitmessage_c &msg)
{
	//
	// read the packet header and get sequence numbers
	//
	msg.beginReading();
	
	int	sequence = msg.readBits(NETCHAN_PACKET_HEADER_BITS_SEQUENCE);
	bool	reliable_message = msg.readBit();
	
	int	sequence_ack = msg.readBits(NETCHAN_PACKET_HEADER_BITS_SEQUENCE_ACK);
	bool	reliable_ack = msg.readBit();
	
	uint_t	packet_size_uncompressed = msg.readBits(NETCHAN_PACKET_HEADER_BITS_UNCOMPRESSED_SIZE);
	uint_t	packet_size_compressed = msg.readBits(NETCHAN_PACKET_HEADER_BITS_COMPRESSED_SIZE);
	
	int	checksum;
	checksum = msg.readBits(NETCHAN_PACKET_HEADER_BITS_CHECKSUM);
	
	msg.readBits(NETCHAN_PACKET_HEADER_BITS_QPORT);
	
	
	//
	// check transmitted packet size
	//
	if(packet_size_compressed != msg.getCurSize())
	{
		// packet size may differ because the network sends bytes instead of bits
		if((msg.getCurSize() - packet_size_compressed) >= 8)
			Com_Error(ERR_DROP, "netchan_c::process: corrupt compressed packet size %i != %i", packet_size_compressed, msg.getCurSize());
			
		msg.setCurSize(packet_size_compressed);
	}
	
	
	//
	// check sum
	//
	int checksum_new = msg.calcCheckSum(NETCHAN_PACKET_HEADER_BITS);
	if(checksum != checksum_new)
	{
		Com_Error(ERR_DROP, "netchan_c::process: corrupt packet: old checksum %i != new checksum %i", checksum, checksum_new);
	}
	
	
	//
	// uncompress the rest of the message and check message
	//
	if(net_compression->getInteger())
	{
		//msg.uncompress(NETCHAN_PACKET_HEADER_BITS);
		
		// make backup
		boost::dynamic_bitset<byte>	bits;		// backup bits + total uncompressed bits
		msg.copyTo(bits, 0, NETCHAN_PACKET_HEADER_BITS);
		
		int parts_num = 1;
		if(reliable_message)
			parts_num++;
		
		// uncompress data
		for(int i=0; i<parts_num; i++)
		{
			boost::dynamic_bitset<byte>	uncomp;
		
			try
			{
				if(!(msg.readBitsCompressed(uncomp)))
					continue;
			}
			catch(std::range_error)
			{
				Com_Printf("netchan_c::process: range error while reading compressed bytes\n", msg._readcount, msg._cursize);
				throw;
			}
			
			for(boost::dynamic_bitset<byte>::size_type i=0; i<uncomp.size(); ++i)
				bits.push_back(uncomp[i]);
		};
		
		if(msg._readcount != msg._cursize)
		{
			Com_Error(ERR_DROP, "netchan_c::process: read count %i != current size %i after reading compressed bytes", msg._readcount, msg._cursize);
		}
		
		// reset and expand _data if needed
		if((NETCHAN_PACKET_HEADER_BITS + bits.size()) > msg._maxsize)
		{
			msg._data = std::vector<byte>(toBytes(NETCHAN_PACKET_HEADER_BITS + bits.size()), 0);
			msg._maxsize = toBits(msg._data.size());
		}
		
		// copy back bits and bytes to _data
		msg._cursize = 0;
		msg.writeBits(bits);
		
		// reset read count
		msg._readcount = NETCHAN_PACKET_HEADER_BITS;
		
		if(packet_size_uncompressed != msg.getCurSize())
			Com_Error(ERR_DROP, "netchan_c::process: corrupt uncompressed packet size %i != %i", packet_size_uncompressed, msg.getCurSize());
	}
	

	if(net_showpackets->getValue())
	{
		if(reliable_message)
			Com_Printf("recv %5i : s=%i reliable=%i ack=%i rack=%i\n"
				, packet_size_compressed
				, sequence
				, _incoming_reliable ^ true
				, sequence_ack
				, reliable_ack);
		else
			Com_Printf("recv %5i : s=%i ack=%i rack=%i\n"
				, packet_size_compressed
				, sequence
				, sequence_ack
				, reliable_ack);
	}


	//
	// discard stale or duplicated packets
	//
	if(sequence <= _incoming_sequence)
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
	if(reliable_ack == _outgoing_reliable)
		_outgoing_reliable_buf.clear();		// it has been received
	
	
	//
	// if this message contains a reliable message, bump incoming_reliable_sequence 
	//
	_incoming_sequence = sequence;
	_incoming_acknowledged = sequence_ack;
	_incoming_reliable_acknowledged = reliable_ack;
	if(reliable_message)
	{
		_incoming_reliable ^= true;
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
	if(_incoming_acknowledged > _outgoing_reliable_last && _incoming_reliable_acknowledged != _outgoing_reliable)
		return true;

	// if the reliable transmit buffer is empty, copy the current message out
	if(!_outgoing_reliable_buf.size() && message.getCurSize())
		return true;

	return false;
}



/*
===============
Netchan_CanReliable

Returns true if the last reliable message has acked
================
*/
bool	netchan_c::canReliable()
{
	if(_outgoing_reliable_buf.size())
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
	net_compression	= Cvar_Get("net_compression", "0", CVAR_NONE);
}

/*
===============
Netchan_OutOfBand

Sends an out-of-band datagram
================
*/
/*
void	Netchan_OutOfBand(const netadr_t &adr, const byte *data, int length)
{
	bitmessage_c send(MAX_PACKETLEN*8);
	
	send.writeLong(-1);	// -1 sequence means out of band
	send.writeString(data, length);

	// send the datagram
	Sys_SendPacket(send, adr);
}
*/

/*
===============
Netchan_OutOfBandPrint

Sends a text message in an out-of-band datagram
================
*/
void	Netchan_OutOfBandPrint(const netadr_t &adr, const char *format, ...)
{
	va_list		argptr;
	static char	string[MAX_PACKETLEN - 4];
	
	va_start(argptr, format);
	vsprintf(string, format,argptr);
	va_end(argptr);

//	Netchan_OutOfBand(adr, (byte*)string, strlen(string));
	
	bitmessage_c send(MAX_PACKETLEN*8);
	
	send.writeLong(-1);	// -1 sequence means out of band
	send.writeString(string);

	// send the datagram
	Sys_SendPacket(send, adr);
}


