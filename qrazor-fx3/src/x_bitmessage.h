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
#ifndef X_BITMESSAGE_H
#define X_BITMESSAGE_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <boost/crc.hpp>

// qrazor-fx ----------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"


#define	MAX_PACKETLEN		1400
#define MAX_MSGLEN		MAX_PACKETLEN


class netchan_c;

class bitmessage_c
{
	friend class netchan_c;
	friend void 	Sys_SendPacket(const bitmessage_c &msg, const netadr_t &to);
	friend void	CL_Record_f();
	friend void	CL_Stop_f();
public:
	bitmessage_c();
	bitmessage_c(int bits_num, bool allowoverflow = false, bool expand = false);
	
	//! check wether this messsage is a connection less packet
	bool		isConnectionless();
	
	//! copy all bits from bits_offset to _cursize into the bits buffer
	void		copyTo(boost::dynamic_bitset<byte> &bits, int bits_offset, int bits_num) const;
	
	//! copy all bits from bits_offset to _cursize into the bytes buffer
	void		copyTo(std::vector<byte> &bytes, int bits_offset, int bits_num) const;
	
	//! calculate quick a checksum byte
	int		calcCheckSum(int bits_offset = 0) const;
	
	void		beginWriting();	
	void		writeBit(bool bit);
	void		writeBits(const boost::dynamic_bitset<byte> &bits);
	int		writeBitsCompressed(const boost::dynamic_bitset<byte> &bits, bool skip_if_fails = false);
	void		writeBits(int bits, int bits_num);
	void 		writeByte(int c);
	void		writeBytes(const byte *bytes, int bytes_num);
	int		writeBytesCompressed(const std::vector<byte> &bytes, bool skip_if_fails = false);
	void 		writeShort(int c);
	void 		writeLong(int c);
	void 		writeFloat(float f);
	void		writeString(const std::string &s);
	void 		writeVec(vec_t v);
	void 		writeVec3(const vec3_c &v);
	void 		writeAngle(float f);
	void 		writeDir(const vec3_c &dir);
	void		writeColor(const vec4_c &color);
	void		writeQuaternion(const quaternion_c &q);

	void		writeDeltaUsercmd(const usercmd_t *from, const usercmd_t *to);
	void 		writeDeltaEntity(const entity_state_t *from, const entity_state_t *to, bool force);
	void		writeDeltaPlayerState(const player_state_t *from, const player_state_t *to);
	
	void		writeMessage(const bitmessage_c &msg);
	int		writeMessageCompressed(const bitmessage_c &msg, bool skip_if_fails = false);

	void		beginReading();
	bool		readBit();
	int		readBits(int bits_num);
	void		readBits(int bits_num, boost::dynamic_bitset<byte> &bits);
	bool		readBitsCompressed(boost::dynamic_bitset<byte> &bits);
	int		readByte();
	void		readBytes(byte *bytes, int bytes_num);
	bool		readBytesCompressed(std::vector<byte> &bytes);
	int		readShort();
	int		readLong();
	float		readFloat();
	const char*	readString();
	vec_t		readVec();
	void		readVec3(vec3_c &v);
	float		readAngle();
	void		readDir(vec3_c &dir);
	void		readColor(vec4_c &color);
	void		readQuaternion(quaternion_c &q);

	void		readDeltaUsercmd(const usercmd_t *from, usercmd_t *to);
	void		readDeltaEntity(const entity_state_t *from, entity_state_t *to, int number);
	void		readDeltaPlayerState(const player_state_t *from, player_state_t *to);
	
	
	inline bool		isOverFlowed() const		{return _overflowed;}
	
	inline uint_t		getMaxSize() const		{return _maxsize;}
	inline uint_t		getMaxSizeInBytes() const	{return toBytes(_maxsize);}
	
	inline uint_t		getCurSizeInBytes() const	{return toBytes(_cursize);}
	inline uint_t		getCurSize() const		{return _cursize;}
	inline void		setCurSize(int size)		{_cursize = size;}
	
	inline uint_t		getReadCount() const		{return _readcount;}
	
private:
	bool				_allowoverflow;		// if false, do a Com_Error
	bool				_overflowed;		// set to true if the buffer size failed
	bool				_expand;		// expand if overflow
	
	std::vector<byte>		_data;
//	std::vector<std::bitset<8> >	_data;
//	boost::dynamic_bitset<byte>	_data;
	
	uint_t				_maxsize;
	uint_t				_cursize;
	uint_t				_readcount;
};

#endif	// X_BITMESSAGE_H
