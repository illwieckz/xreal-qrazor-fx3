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
#ifndef NET_CHAN_H
#define NET_CHAN_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_bitmessage.h"


enum
{
	NETCHAN_PACKET_HEADER_BITS_SEQUENCE			= 31,
	NETCHAN_PACKET_HEADER_BITS_RELIABLE			= 1,
	NETCHAN_PACKET_HEADER_BITS_SEQUENCE_ACK			= 31,
	NETCHAN_PACKET_HEADER_BITS_RELIABLE_ACK			= 1,
	
	NETCHAN_PACKET_HEADER_BITS_SIZE_OFFSET			= NETCHAN_PACKET_HEADER_BITS_SEQUENCE +
								  NETCHAN_PACKET_HEADER_BITS_RELIABLE +
								  NETCHAN_PACKET_HEADER_BITS_SEQUENCE_ACK+
								  NETCHAN_PACKET_HEADER_BITS_RELIABLE_ACK,
								  
	NETCHAN_PACKET_HEADER_BITS_UNCOMPRESSED_SIZE		= 16,
	NETCHAN_PACKET_HEADER_BITS_COMPRESSED_SIZE		= 16,
	
	NETCHAN_PACKET_HEADER_BITS_CHECKSUM_OFFSET		= NETCHAN_PACKET_HEADER_BITS_SIZE_OFFSET +
								  NETCHAN_PACKET_HEADER_BITS_UNCOMPRESSED_SIZE+
								  NETCHAN_PACKET_HEADER_BITS_COMPRESSED_SIZE,
	NETCHAN_PACKET_HEADER_BITS_CHECKSUM			= 32,
	
	NETCHAN_PACKET_HEADER_BITS_QPORT			= 16,
	
	NETCHAN_PACKET_HEADER_BITS				= NETCHAN_PACKET_HEADER_BITS_SEQUENCE +
								  NETCHAN_PACKET_HEADER_BITS_RELIABLE +
								  NETCHAN_PACKET_HEADER_BITS_SEQUENCE_ACK+
								  NETCHAN_PACKET_HEADER_BITS_RELIABLE_ACK+
								  NETCHAN_PACKET_HEADER_BITS_UNCOMPRESSED_SIZE+
								  NETCHAN_PACKET_HEADER_BITS_COMPRESSED_SIZE+
								  NETCHAN_PACKET_HEADER_BITS_CHECKSUM+
								  NETCHAN_PACKET_HEADER_BITS_QPORT
};

/*
packet header
-------------
31	sequence
1	does this message contain a reliable payload
31	acknowledge sequence
1	acknowledge receipt of even/odd message
16	uncompressed total packet size in bits
16	compressed total packet size in bits
32	checksum of all data after this packet header
16	qport

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


//#define FRAGMENT_MASK			0x80000000
//#define FRAGMENTATION_TRESHOLD	(MAX_PACKETLEN-100)

class netchan_c
{
public:
	void			setup(const netadr_t &adr, int qport, bool client);
	
	//! Tries to send an unreliable message to a connection, and handles the
	//! transmition / retransmition of the reliable messages.
	//! A 0 length will still generate a packet and deal with the reliable messages.
	void			transmit(const bitmessage_c &msg);
	
	//! Called when the current net_message is from remote_address.
	bool			process(bitmessage_c &msg);
	
	int			getDropped() const		{return _dropped;}
	
	const netadr_t&		getRemoteAddress() const	{return _remote_address;}
	void			fixRemoteAddressPort(int port)	{_remote_address.port = port;}
	int			getQPort() const		{return _qport;}
		
	int			getLastReceived() const		{return _last_received;}
	int			getLastSent() const		{return _last_sent;}
	
	int			getIncomingSequence() const	{return _incoming_sequence;}
	int			getIncomingAcknowledged() const	{return _incoming_acknowledged;}
	int			getOutgoingSequence() const	{return _outgoing_sequence;}

private:
	bool			needReliable();
	bool 			canReliable();
	
	int				_dropped;				// between last packet and previous

	netadr_t			_remote_address;
	int				_qport;					// qport value to write when transmitting
	bool				_client;				// are we a client?
	
	// timestamps
	int				_last_received;				// for timeouts
	int				_last_sent;				// for retransmits

	// sequencing variables
	int				_incoming_sequence;
	bool				_incoming_reliable;			// single bit, maintained local
	int				_incoming_acknowledged;
	bool				_incoming_reliable_acknowledged;	// single bit

	int				_outgoing_sequence;
	bool				_outgoing_reliable;			// single bit
	boost::dynamic_bitset<byte>	_outgoing_reliable_buf;			// message is copied to this buffer when it is first transfered
	int				_outgoing_reliable_last;		// sequence number of last send

public:
	// reliable staging and holding areas
	bitmessage_c			message;				// writing buffer to send to server
};

void 	Netchan_Init();

//void 	Netchan_OutOfBand(const netadr_t &adr, const byte *data, int length);
void 	Netchan_OutOfBandPrint(const netadr_t &adr, const char *format, ...);





#endif	// NET_CHAN_H
