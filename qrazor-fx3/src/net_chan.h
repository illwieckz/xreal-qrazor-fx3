/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
#include "x_message.h"


//#define FRAGMENT_MASK			0x80000000
//#define FRAGMENTATION_TRESHOLD	(MAX_PACKETLEN-100)

class netchan_c
{
public:
	void	setup(const netadr_t &adr);
	
	void 	transmit(const byte *data, int length);
	void	transmit(const message_c &msg);
	
	bool	process(message_c &msg);
	
	int			getDropped() const		{return _dropped;}
	
	const netadr_t&		getRemoteAddress() const	{return _remote_address;}
		
	int			getLastReceived() const		{return _last_received;}
	int			getLastSent() const		{return _last_sent;}
	
	int			getIncomingSequence() const	{return _incoming_sequence;}
	int			getIncomingAcknowledged() const	{return _incoming_acknowledged;}
	int			getOutgoingSequence() const	{return _outgoing_sequence;}

private:
	bool	needReliable();
	bool 	canReliable();
	
	int			_dropped;			// between last packet and previous

	netadr_t		_remote_address;
	
	int			_last_received;			// for timeouts
	int			_last_sent;			// for retransmits

	// sequencing variables
	int			_incoming_sequence;
	int			_incoming_acknowledged;
	int			_incoming_reliable_acknowledged;	// single bit

	int			_incoming_reliable_sequence;	// single bit, maintained local

	int			_outgoing_sequence;
	int			_reliable_sequence;			// single bit
	int			_last_reliable_sequence;		// sequence number of last send

public:
	// reliable staging and holding areas
	message_c		message;			// writing buffer to send to server
private:	
	// message is copied to this buffer when it is first transfered
	int			_reliable_length;
	byte			_reliable_buf[MAX_MSGLEN-16];	// unacked reliable message
};

void 	Netchan_Init();

void 	Netchan_OutOfBand(const netadr_t &adr, const byte *data, int length);
void 	Netchan_OutOfBandPrint(const netadr_t &adr, const char *format, ...);





#endif	// NET_CHAN_H
