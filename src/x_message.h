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
#ifndef X_MESSAGE_H
#define X_MESSAGE_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
#include <bitset>
#include <boost/dynamic_bitset.hpp>

// qrazor-fx ----------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_shared.h"


#define	MAX_PACKETLEN		1400
#define MAX_MSGLEN		MAX_PACKETLEN*5


enum message_type_t
{
	MSG_TYPE_UNINIT,
	MSG_TYPE_RAWBYTES,
	MSG_TYPE_RAWBITS
};

class message_c
{
public:
	message_c();
	message_c(message_type_t type, int data_size, bool allowoverflow = false);
	
	void	clear();

	void	beginWriting();
private:
	void*	getSpace(int length);
	void	addSpace(int length);
	void	writeRawBytes(int c, int bits);
	void	writeRawBits(int c, int bits);
public:
	
	void	write(const void *data, int length);
	void 	print(const char *data);		// strcats onto the sizebuf
	void	writeRawData(const void *data, int length);				// write raw data without any modifications
	bool	writeCompressedData(const void *data, int length, bool skip = false);	// first compress data and then write it
											// don't write it if not needed
	void	writeBits(const boost::dynamic_bitset<> &bits);
	void	writeBits(int value, int bits);
	void 	writeByte(int c);
	void 	writeShort(int c);
	void 	writeLong(int c);
	void 	writeFloat(float f);
	void	writeString(const char *s);
	void 	writeCoord(float f);
	void 	writeVector3(const vec3_c &v);
	void 	writeAngle(float f);
	void 	writeDir(const vec3_c &dir);
	void	writeColor(const vec4_c &color);
	void	writeQuaternion(const quaternion_c &q);

	void	writeDeltaUsercmd(const usercmd_t *from, const usercmd_t *to);
	void 	writeDeltaEntity(const entity_state_t *from, const entity_state_t *to, bool force);

	void	beginReading();
private:
	int	readRawBytes(int bits);
	int	readRawBits(int bits);
public:
	int	readBits(int bits);
	int	readByte();
	int	readShort();
	int	readLong();
	float	readFloat();
	char*	readString();
	char*	readStringLine();
	float	readCoord();
	void	readVector3(vec3_c &v);
	float	readAngle();
	void	readDir(vec3_c &dir);
	void	readColor(vec4_c &color);
	void	readQuaternion(quaternion_c &q);
	void	readRawData(void *data, int length);
	void	readCompressedData(void *data, int length);

	void	readDeltaUsercmd(const usercmd_t *from, usercmd_t *to);
	bool	readDeltaEntity(const entity_state_t *from, entity_state_t *to, int number);
	
	// uncompress all data after current bytes read count
	void	uncompress();	
	
	
	inline message_type_t	getType() const			{return _type;}
	inline bool		isOverFlowed() const		{return _overflowed;}
	
	inline uint_t		getMaxSize() const		{return _maxsize;}
	inline uint_t		getCurSize() const		{return _cursize;}
	inline void		incCurSize(uint_t size)		{_cursize += size;}
	
	inline uint_t		getBytesReadCount() const	{return _readcount_bytes;}
	inline uint_t		getBitsReadCount() const	{return _readcount_bits;}
	
	
	byte	operator [] (const int index) const
	{
		return _data[index];
	}

	byte&	operator [] (const int index)
	{
		return _data[index];
	}
	
	inline operator byte * () const
	{
		return (byte*)&_data[0];
	}
	
private:
	message_type_t		_type;
	bool			_allowoverflow;	// if false, do a Com_Error
	bool			_overflowed;		// set to true if the buffer size failed
	
	std::vector<byte>	_data;
	
	uint_t			_maxsize;		// in bytes
	uint_t			_cursize;
	uint_t			_readcount_bytes;
	uint_t			_readcount_bits;
};

#endif	// X_MESSAGE_H
