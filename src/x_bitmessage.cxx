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
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <boost/crc.hpp>
#include <zlib.h>

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_bitmessage.h"
#include "x_protocol.h"



bitmessage_c::bitmessage_c()
{
	_allowoverflow = false;
	_overflowed = false;
	
	_maxsize = 0;
	_cursize = 0;
	_readcount = 0;
}

bitmessage_c::bitmessage_c(int bits_num, bool allowoverflow)
{
	_allowoverflow = allowoverflow;

	_maxsize = bits_num;
	_readcount = 0;
	
	beginWriting();
}

bool	bitmessage_c::isConnectionless()
{
	_readcount = 0;
	int c = readLong();
	_readcount = 0;
	
	if(c == -1)
		return true;
	else
		return false;
}

void	bitmessage_c::copyTo(boost::dynamic_bitset<byte> &bits, int bits_offset, int size) const
{
	if(size == -1)
		size = _cursize;

	if(bits_offset > size)
		Com_Error(ERR_FATAL, "bitmessage_c::copyTo: bad bits_offset %i > current size %i", bits_offset, _cursize);

	uint_t bits_num = size - bits_offset;

	bits.clear();
	bits.resize(bits_num, 0);
	
	for(uint_t i=0; i<bits_num; i++)
		bits[i] = _data[(bits_offset+i) >> 3] & (1<<((bits_offset+i) & 7));
}

void	bitmessage_c::copyTo(std::vector<byte> &bytes, int bits_offset, int size)
{
	if(size == -1)
		size = _cursize;

	if(bits_offset > size)
		Com_Error(ERR_FATAL, "bitmessage_c::copyTo: bad bits_offset %i > current size %i", bits_offset, _cursize);

	uint_t bits_num = size - bits_offset;
	
	bytes.clear();
	bytes.resize(tobytes(bits_num));
	
	for(uint_t i=0; i<bits_num; i++)
	{
		bool bit = _data[(bits_offset+i) >> 3] & (1<<((bits_offset+i) & 7));
	
		bytes[i >> 3] |= (bit << (i % 8));
	}
}

int	bitmessage_c::calcCheckSum(int bits_offset) const
{
	if(bits_offset < 0 || bits_offset > (int)_cursize)
		Com_Error(ERR_FATAL, "bitmessage_c::calcCheckSum: bad bits_offset %i > current size %i", bits_offset, _cursize);
	
	boost::crc_32_type crc;
//	for(uint_t i=0; i<_cursize; i++)
//		crc.process_bits(_data[(bits_offset+i) >> 3] & (1<<((bits_offset+i) & 7)));

	crc.process_bytes(&_data[bits_offset >> 3], tobytes(_cursize));
	
	return crc.checksum();
}

void	bitmessage_c::beginWriting()
{
	_overflowed = false;

	_data = std::vector<byte>(_maxsize/8, 0);
//	_data = std::vector<std::bitset<8> >(_maxsize/8);
//	_data = boost::dynamic_bitset<byte>(_maxsize);
	
	_cursize = 0;
}

void	bitmessage_c::writeBit(bool bit)
{
	if((_cursize + 1) > _maxsize)
	{
		if(!_allowoverflow)
			Com_Error(ERR_FATAL, "bitmessage_c::writeBit: overflow without allowoverflow set");
		
		Com_Printf("bitmessage_c::writeBit: overflow maxsize %i\n", _maxsize);
		
		//assert(false);
		beginWriting();
		_overflowed = true;
		throw std::overflow_error(va("bitmessage_c::writeBit: overflow maxsize %i", _maxsize));
		//return;
	}

	_data[_cursize >> 3] |= (bit << (_cursize % 8));
//	_data[_cursize / 8][_cursize % 8] = bit;
//	_data[_cursize] = bit;

	_cursize++;
}

void	bitmessage_c::writeBits(const boost::dynamic_bitset<byte> &bits)
{
	for(uint_t i=0; i<bits.size(); i++)
	{
		try
		{
			writeBit(bits[i]);
		}
		catch(std::overflow_error)
		{
			Com_Printf("bitmessage_c::writeBits: overflow error while writing %i bits from bitset\n", bits.size());
			throw;
		}
	}
}

void	bitmessage_c::writeBits(int bits, int bits_num)
{
	if(!bits_num || bits_num < -31 || bits_num > 32)
		Com_Error(ERR_FATAL, "message_c::writeBits: bad bits number %i", bits_num);
		
	if(bits_num < 0)
		bits_num = -bits_num;
		
	for(int i=0; i<bits_num; i++)
	{
		try
		{
			writeBit(bits & (1 << i));
		}
		catch(std::overflow_error)
		{
			Com_Printf("bitmessage_c::writeBits: overflow error while writing %i bits from integer bitmask\n", bits_num);
			throw;
		}
	}
}

int	bitmessage_c::writeBitsCompressed(const boost::dynamic_bitset<byte> &bits, bool skip_if_fails)
{
	if(bits.size() == 0)
	{
		writeBit(false);	// nothing to follow
		return 1;
	}

	// convert bits to bytes
	std::vector<byte> bytes(tobytes(bits.size()), 0);

	for(uint_t i=0; i<bits.size(); i++)
		bytes[i >> 3] |= (bits[i] << (i % 8));
		
	// compress converted bytes
	std::vector<byte>	bytes_compressed(tobytes(_maxsize - _cursize), 0);
	unsigned long		size_compressed = bytes_compressed.size();
	
	int err = compress(&bytes_compressed[0], &size_compressed, &bytes[0], bytes.size());
	
	switch(err)
	{
		case Z_NEED_DICT:	// 2
			Com_Error(ERR_DROP, "message_c::writeBytesCompressed: need dictionary");
			return -1;
			
		case Z_STREAM_END:	// 1
			Com_Error(ERR_DROP, "message_c::writeBytesCompressed: stream end");
			return -1;
			
		case Z_OK:		// 0
			break;
			
		case Z_ERRNO:		// -1
			Com_Error(ERR_DROP, "message_c::writeBytesCompressed: file error");
			return -1;
			
		case Z_STREAM_ERROR:	// -2
			Com_Error(ERR_DROP, "message_c::writeBytesCompressed: stream error");
			return -1;
			
		case Z_DATA_ERROR:	// -3
		{
			if(!skip_if_fails)
			{
				Com_Error(ERR_DROP, "bitmessage_c::writeBytesCompressed: data error");
				writeBit(false);
				return 1;
			}
			else
			{
				throw std::exception();
				writeBit(false);
				return 1;
				break;
			}
		}
		
		case Z_MEM_ERROR:	// -4
			Com_Error(ERR_DROP, "message_c::writeBytesCompressed: insufficient memory");
			return -1;
			
		case Z_BUF_ERROR:	// -5
			Com_Error(ERR_DROP, "message_c::writeBytesCompressed: buffer error");
			return -1;
			
		case Z_VERSION_ERROR:	// -6
			Com_Error(ERR_DROP, "message_c::writeBytesCompressed: incompatible version");
			return -1;
			
		default:
			Com_Error(ERR_DROP, "message_c::writeBytesCompressed: unknown error");
	}

	// write result
	writeBit(true);						// yes there is more to come
	writeBits(bits.size(), 14);				// original bits size
	writeBits(size_compressed, 11);				// compressed bytes size
	writeBytes(&bytes_compressed[0], size_compressed);	// compressed bytes
	
	return 1+14+11+tobits(size_compressed);
}

void 	bitmessage_c::writeByte(int c)
{
	try
	{
		writeBits(c, 8);
	}
	catch(std::overflow_error)
	{
		Com_Printf("bitmessage_c::writeByte: overflow error\n");
		throw;
	}
}

void	bitmessage_c::writeBytes(const byte *bytes, int bytes_num)
{
	if(bytes_num == 0)
		return;
		
	if(bytes_num < 0)
	{
		Com_Error(ERR_FATAL, "bitmessage_c::writeBytes: length %i", bytes_num);
		return;
	}
	
	for(int i=0; i<bytes_num; i++)
	{
		try
		{
			writeByte(bytes[i]);
		}
		catch(std::overflow_error)
		{
			Com_Printf("bitmessage_c::writeBytes: overflow error while writing %i bytes\n", bytes_num);
			throw;
		}
	}
}

/*
int	bitmessage_c::writeBytesCompressed(const byte *bytes, int bytes_num, bool skip_if_fails)
{
	
}
*/

void 	bitmessage_c::writeShort(int c)
{
	try
	{
		writeBits(c, 16);
	}
	catch(std::overflow_error)
	{
		Com_Printf("bitmessage_c::writeShort: overflow error\n");
		throw;
	}
}

void 	bitmessage_c::writeLong(int c)
{
	try
	{
		writeBits(c, 32);
	}
	catch(std::overflow_error)
	{
		Com_Printf("bitmessage_c::writeLong: overflow error\n");
		throw;
	}
}

void 	bitmessage_c::writeFloat(float f)
{
	union
	{
		float	f;
		int	l;
	} dat;
	
	dat.f = f;
	dat.l = LittleLong(dat.l);
	
	try
	{
		writeBits(dat.l, 32);
	}
	catch(std::overflow_error)
	{
		Com_Printf("bitmessage_c::writeFloat: overflow error\n");
		throw;
	}
}

void 	bitmessage_c::writeString(const std::string &s)
{
	int len = s.length();
	
	try
	{
		writeLong(len);	
	}
	catch(std::overflow_error)
	{
		Com_Printf("bitmessage_c::writeString: overflow error while writing length %i\n", len);
		throw;
	}
	
	for(int i=0; i<len; i++)
	{
		try
		{
			writeByte(s[i]);
		}
		catch(std::overflow_error)
		{
			Com_Printf("bitmessage_c::writeString: overflow error while writing %i bytes\n", len);
			throw;
		}
	}
}

void 	bitmessage_c::writeCoord(float f)
{
	writeFloat(f);
}

void 	bitmessage_c::writeVector3(const vec3_c &v)
{
	writeFloat(v[0]);
	writeFloat(v[1]);
	writeFloat(v[2]);
}

void	bitmessage_c::writeAngle(float f)
{
	writeFloat(f);
}

void 	bitmessage_c::writeDir(const vec3_c &dir)
{
	int	best = 0;
	float	bestd = 0;
	
	for(int i=0; i<NUMVERTEXNORMALS; i++)
	{
		float d = dir.dotProduct(bytedirs[i]);
		
		if(d > bestd)
		{
			bestd = d;
			best = i;
		}
	}
	
	writeByte(best);
}


void	bitmessage_c::writeColor(const vec4_c &color)
{
	vec4_c ccolor(color);

	X_clamp(ccolor[0], 0.0, 1.0);
	X_clamp(ccolor[1], 0.0, 1.0);
	X_clamp(ccolor[2], 0.0, 1.0);
	X_clamp(ccolor[3], 0.0, 1.0);

	writeByte((byte)(ccolor[0] * 255));
	writeByte((byte)(ccolor[1] * 255));
	writeByte((byte)(ccolor[2] * 255));
	writeByte((byte)(ccolor[3] * 255));
}

void	bitmessage_c::writeQuaternion(const quaternion_c &q)
{
	writeFloat(q._q[0]);
	writeFloat(q._q[1]);
	writeFloat(q._q[2]);
	writeFloat(q._q[3]);
}


void 	bitmessage_c::writeDeltaUsercmd(const usercmd_t *from, const usercmd_t *to)
{
	//
	// send the movement message
	//
	int bits = 0;
	if(to->angles[0] != from->angles[0])
		bits |= CM_ANGLE1;
	
	if(to->angles[1] != from->angles[1])
		bits |= CM_ANGLE2;
	
	if(to->angles[2] != from->angles[2])
		bits |= CM_ANGLE3;
	
	if(to->forwardmove != from->forwardmove)
		bits |= CM_FORWARD;
	
	if(to->sidemove != from->sidemove)
		bits |= CM_SIDE;
	
	if(to->upmove != from->upmove)
		bits |= CM_UP;
	
	if(to->buttons != from->buttons)
		bits |= CM_BUTTONS;

   	writeByte(bits);
	
	// write current angles
	if(bits & CM_ANGLE1)
		writeFloat(to->angles[0]);
		
	if(bits & CM_ANGLE2)
		writeFloat(to->angles[1]);
		
	if(bits & CM_ANGLE3)
		writeFloat(to->angles[2]);
	
	// write current movement
	if(bits & CM_FORWARD)
		writeFloat(to->forwardmove);
		
	if(bits & CM_SIDE)
	  	writeFloat(to->sidemove);
		
	if(bits & CM_UP)
		writeFloat(to->upmove);

	// write current buttons
 	if(bits & CM_BUTTONS)
	  	writeByte(to->buttons);

	writeLong(to->msec);
}

/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message.
Can delta from either a baseline or a previous packet_entity
==================
*/
void 	bitmessage_c::writeDeltaEntity(const entity_state_t *from, const entity_state_t *to, bool force)
{
	int		bits = 0;
	
	if(!from && !to)
	{
		Com_Error(ERR_FATAL, "message_c::writeDeltaEntity: NULL");
	}
	
	if(!to)
	{
		writeShort(from->getNumber());
		writeLong(U_REMOVE);
		return;
	}

	if(!to->getNumber())
		Com_Error(ERR_FATAL, "Unset entity number");
		
	if(to->getNumber() >= MAX_ENTITIES)
		Com_Error(ERR_FATAL, "Entity number >= MAX_ENTITIES");


	// send an update
	bits = 0;
	
	if(to->type != from->type)
		bits |= U_TYPE;
		
	if(to->origin != from->origin)
		bits |= U_ORIGIN;
		
	if(to->origin2 != from->origin2)
		bits |= U_ORIGIN2;
		
	if(to->quat != from->quat)
		bits |= U_QUATERNION;		
		
	if(to->quat2 != from->quat2)
		bits |= U_QUATERNION2;
		
	if(to->velocity_linear != from->velocity_linear)
		bits |= U_VELOCITY_LINEAR;
		
	if(to->velocity_angular != from->velocity_angular)
		bits |= U_VELOCITY_ANGULAR;
				
	if(to->index_model != from->index_model)
		bits |= U_INDEX_MODEL;
		
	if(to->index_shader != from->index_shader)
		bits |= U_INDEX_SHADER;	
		
	if(to->index_animation != from->index_animation)
		bits |= U_INDEX_ANIMATION;
		
	if(to->index_sound != from->index_sound)
		bits |= U_INDEX_SOUND;
		
	if(to->index_light != from->index_light)
		bits |= U_INDEX_LIGHT;
	
	if(to->frame != from->frame)
		bits |= U_FRAME;

	if(to->effects != from->effects)
		bits |= U_EFFECTS;
	
	if(to->renderfx != from->renderfx)
		bits |= U_RENDERFX;

	if(to->event)	// event is not delta compressed, just 0 compressed
		bits |= U_EVENT;
		
	if(to->shaderparms[0] != from->shaderparms[0])
		bits |= U_SHADERPARM0;
		
	if(to->shaderparms[1] != from->shaderparms[1])
		bits |= U_SHADERPARM1;
		
	if(to->shaderparms[2] != from->shaderparms[2])
		bits |= U_SHADERPARM2;
		
	if(to->shaderparms[3] != from->shaderparms[3])
		bits |= U_SHADERPARM3;
		
	if(to->shaderparms[4] != from->shaderparms[4])
		bits |= U_SHADERPARM4;
	
	if(to->shaderparms[5] != from->shaderparms[5])
		bits |= U_SHADERPARM5;
		
	if(to->shaderparms[6] != from->shaderparms[6])
		bits |= U_SHADERPARM6;
		
	if(to->shaderparms[7] != from->shaderparms[7])
		bits |= U_SHADERPARM7;
		
	if(to->vectors[0] != from->vectors[0])
		bits |= U_VECTOR0;
		
	if(to->vectors[1] != from->vectors[1])
		bits |= U_VECTOR1;
		
	if(to->vectors[2] != from->vectors[2])
		bits |= U_VECTOR2;


	//
	// write the message
	//
	if(!bits && !force)
		return;		// nothing to send!

	//Com_Printf("message_c::writeDeltaEntity: %3i %3i\n", to->getNumber());
	
	writeShort(to->getNumber());
	writeLong(bits);
	
	
	if(bits & U_TYPE)
		writeByte(to->type);

	if(bits & U_ORIGIN)
		writeVector3(to->origin);
				
	if(bits & U_ORIGIN2)
		writeVector3(to->origin2);
		
	if(bits & U_QUATERNION)
		writeQuaternion(to->quat);
		
	if(bits & U_QUATERNION2)
		writeQuaternion(to->quat2);
		
	if(bits & U_VELOCITY_LINEAR)
		writeVector3(to->velocity_linear);
		
	if(bits & U_VELOCITY_ANGULAR)
		writeVector3(to->velocity_angular);

	if(bits & U_INDEX_MODEL)
		writeShort(to->index_model);
		
	if(bits & U_INDEX_SHADER)
		writeShort(to->index_shader);
		
	if(bits & U_INDEX_ANIMATION)
		writeShort(to->index_animation);
		
	if(bits & U_INDEX_SOUND)
		writeShort(to->index_sound);
		
	if(bits & U_INDEX_LIGHT)
		writeShort(to->index_light);
	
	if(bits & U_FRAME)
		writeShort(to->frame);
	
	if(bits & U_EFFECTS)
		writeLong(to->effects);

	if(bits & U_RENDERFX)
		writeLong(to->renderfx);
		
	if(bits & U_EVENT)
		writeByte(to->event);
		
	if(bits & U_SHADERPARM0)
		writeFloat(to->shaderparms[0]);
		
	if(bits & U_SHADERPARM1)
		writeFloat(to->shaderparms[1]);
		
	if(bits & U_SHADERPARM2)
		writeFloat(to->shaderparms[2]);
		
	if(bits & U_SHADERPARM3)
		writeFloat(to->shaderparms[3]);
		
	if(bits & U_SHADERPARM4)
		writeFloat(to->shaderparms[4]);
		
	if(bits & U_SHADERPARM5)
		writeFloat(to->shaderparms[5]);
		
	if(bits & U_SHADERPARM6)
		writeFloat(to->shaderparms[6]);
		
	if(bits & U_SHADERPARM7)
		writeFloat(to->shaderparms[7]);
		
	if(bits & U_VECTOR0)
		writeVector3(to->vectors[0]);
		
	if(bits & U_VECTOR1)
		writeVector3(to->vectors[1]);
		
	if(bits & U_VECTOR2)
		writeVector3(to->vectors[2]);
}


void	bitmessage_c::writeMessage(const bitmessage_c &msg)
{
	boost::dynamic_bitset<byte> bits;
	msg.copyTo(bits);
	
	writeBits(bits);
}

int	bitmessage_c::writeMessageCompressed(const bitmessage_c &msg, bool skip_if_fails)
{
	boost::dynamic_bitset<byte> bits;
	msg.copyTo(bits);

	return writeBitsCompressed(bits, skip_if_fails);
//	return writeBytesCompressed(&msg._data[0], msg.getCurSizeInBytes(), skip_if_fails);
}


void 	bitmessage_c::beginReading()
{
	_readcount = 0;
}

bool	bitmessage_c::readBit()
{
	if(_readcount+1 > _cursize)
	{
		Com_Printf("bitmessage_c::readBit: range error: readcount %i + 1 > cursize %i\n", _readcount, _cursize);
		throw std::range_error(va("bitmessage_c::readBit: range error: readcount %i + 1 > cursize %i\n", _readcount, _cursize));
		//return;
	}

	bool bit = _data[_readcount >> 3] & (1<<((_readcount) & 7));
//	bool bit = _data[_readcount / 8][_readcount % 8];
//	bool bit = _data[_readcount];
	_readcount++;
	return bit;
}

int	bitmessage_c::readBits(int bits_num)
{
	if(!bits_num || bits_num < -31 || bits_num > 32)
		Com_Error(ERR_FATAL, "bitmessage_c::readBits: bad bits number %i", bits_num);
		
	if(bits_num < 0)
		bits_num = -bits_num;
		
	boost::dynamic_bitset<byte>	bitset(bits_num);
		
	for(int i=0; i<bits_num; i++)
	{
		try
		{
			bitset[i] = readBit();
		}
		catch(std::range_error)
		{
			Com_Printf("bitmessage_c::readBits: range error while reading %i bits\n", bits_num);
			throw;
		}
	}
	
	return bitset.to_ulong();
}

int	bitmessage_c::readBits(int n, boost::dynamic_bitset<byte> &bits)
{
	if(n < 0)
		n = -n;

	for(int i=0; i<n; i++)
		bits[i] = readByte();
		
	return 0;
}

bool	bitmessage_c::readBitsCompressed(boost::dynamic_bitset<byte> &bits)
{
	// check if something is appended
	try
	{
		bool more = readBit();
		if(!more)
			return false;
	}
	catch(std::range_error)
	{
		Com_Printf("bitmessage_c::readBytesCompressed: range error while reading initial bit\n", _readcount, _cursize);
		throw;
	}
	
	// get size in bytes
	uint_t	bits_num = readBits(14);
	uint_t	bytes_num = readBits(11);
	
	// read the compressed bytes
	std::vector<byte>	data_compressed(bytes_num, 0);
	readBytes(&data_compressed[0], data_compressed.size());

	// uncompress the bytes
	byte			data_uncompressed[MAX_MSGLEN];
	unsigned long		length_uncompressed = sizeof(data_uncompressed);
	
	int err = ::uncompress(data_uncompressed, &length_uncompressed, &data_compressed[0], data_compressed.size());
	
	switch(err)
	{
		case Z_NEED_DICT:	// 2
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: need dictionary");
			return false;
			
		case Z_STREAM_END:	// 1
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: stream end");
			return false;
			
		case Z_OK:		// 0
			break;
			
		case Z_ERRNO:		// -1
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: file error");
			return false;
			
		case Z_STREAM_ERROR:	// -2
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: stream error");
			return false;
			
		case Z_DATA_ERROR:	// -3
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: data error");
			return false;
		
		case Z_MEM_ERROR:	// -4
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: insufficient memory");
			return false;
			
		case Z_BUF_ERROR:	// -5
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: buffer error");
			return false;
			
		case Z_VERSION_ERROR:	// -6
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: incompatible version");
			return false;
			
		default:
			Com_Error(ERR_DROP, "bitmessage_c::readBytesCompressed: unknown error");
			return false;
	}
	
	// copy uncompressed bytes into bits buffer
	bits = boost::dynamic_bitset<byte>(bits_num, 0);
	
	for(uint_t i=0; i<bits_num; i++)
		bits[i] = data_uncompressed[i >> 3] & (1<<(i & 7));
	
//	bytes = std::vector<byte>(length_uncompressed, 0);
//	memcpy(&bytes[0], &data_uncompressed[0], length_uncompressed);
	
	return true;
}

int 	bitmessage_c::readByte()
{
	return readBits(8);
}

void 	bitmessage_c::readBytes(byte *bytes, int bytes_num)
{
	for(int i=0; i<bytes_num; i++)
		bytes[i] = readByte();
}


int 	bitmessage_c::readShort()
{
	return readBits(16);
}

int 	bitmessage_c::readLong()
{
	return readBits(32);
}

float	bitmessage_c::readFloat()
{
	union
	{
		float	f;
		int	l;
	} dat;
	
	dat.l = readBits(32);
	dat.l = LittleLong(dat.l);

	return dat.f;
}

const char*	bitmessage_c::readString()
{
	int length = readLong();
	
	std::string s;
	for(int i=0; i<length; i++)
		s += readByte();
		
	return s.c_str();
}

float	bitmessage_c::readCoord()
{
	return readFloat();
}

void 	bitmessage_c::readVector3(vec3_c &v)
{
	v[0] = readFloat();
	v[1] = readFloat();
	v[2] = readFloat();
}

float	bitmessage_c::readAngle()
{
	return readFloat();
}

void	bitmessage_c::readDir(vec3_c &dir)
{
	int b = readByte();
	
	if(b >= NUMVERTEXNORMALS)
		Com_Error(ERR_DROP, "bitmessage_c::readDir: out of range");
		
	dir = bytedirs[b];
}

void 	bitmessage_c::readColor(vec4_c &color)
{
	color[0] = readByte() * (1.0/255);
	color[1] = readByte() * (1.0/255);
	color[2] = readByte() * (1.0/255);
	color[3] = readByte() * (1.0/255);
}

void	bitmessage_c::readQuaternion(quaternion_c &q)
{
	q._q[0] = readFloat();
	q._q[1] = readFloat();
	q._q[2] = readFloat();
	q._q[3] = readFloat();
}

void	bitmessage_c::readDeltaUsercmd(const usercmd_t *from, usercmd_t *to)
{
	memcpy(to, from, sizeof(*to));

	int bits = readByte();
		
	// read current angles
	if(bits & CM_ANGLE1)
		to->angles[0] = readFloat();
		
	if(bits & CM_ANGLE2)
		to->angles[1] = readFloat();
		
	if(bits & CM_ANGLE3)
		to->angles[2] = readFloat();
		
	// read movement
	if(bits & CM_FORWARD)
		to->forwardmove = readFloat();
		
	if(bits & CM_SIDE)
		to->sidemove = readFloat();
		
	if(bits & CM_UP)
		to->upmove = readFloat();
		
	// read buttons
	if(bits & CM_BUTTONS)
		to->buttons = readByte();

	// read time to run command
	to->msec = readLong();
}


/*
==================
CG_ParseDelta

Can go from either a baseline or a previous packet_entity
==================
*/
bool	bitmessage_c::readDeltaEntity(const entity_state_t *from, entity_state_t *to, int number)
{
	// set everything to the state we are delta'ing from
	*to = *from;
	to->number = number;

	unsigned int bits = readLong();
	
	if(bits & U_REMOVE)
	{
		to->clear();
		return true;
	}
	
	if(!bits)
		return false;
	
	if(bits & U_TYPE)
		to->type = (entity_type_e)readByte();
	
	if(bits & U_ORIGIN)
		readVector3(to->origin);
		
	if(bits & U_ORIGIN2)
		readVector3(to->origin2);
		
	if(bits & U_QUATERNION)
		readQuaternion(to->quat);
		 
	if(bits & U_QUATERNION2)
		readQuaternion(to->quat2);
		 
	if(bits & U_VELOCITY_LINEAR)
		readVector3(to->velocity_linear);
		
	if(bits & U_VELOCITY_ANGULAR)
		readVector3(to->velocity_angular);

	if(bits & U_INDEX_MODEL)
		to->index_model = readShort();
		
	if(bits & U_INDEX_SHADER)
		to->index_shader = readShort();
		
	if(bits & U_INDEX_ANIMATION)
		to->index_animation = readShort();
		
	if(bits & U_INDEX_SOUND)
		to->index_sound = readShort();
		
	if(bits & U_INDEX_LIGHT)
		to->index_light = readShort();
		
	if(bits & U_FRAME)
		to->frame = readShort();
	
	if(bits & U_EFFECTS)
		to->effects = readLong();
	
	if(bits & U_RENDERFX)
		to->renderfx = readLong();

	if(bits & U_EVENT)
		to->event = readByte();
	else
		to->event = 0;
		
	if(bits & U_SHADERPARM0)
		to->shaderparms[0] = readFloat();
		
	if(bits & U_SHADERPARM1)
		to->shaderparms[1] = readFloat();
		
	if(bits & U_SHADERPARM2)
		to->shaderparms[2] = readFloat();
		
	if(bits & U_SHADERPARM3)
		to->shaderparms[3] = readFloat();
		
	if(bits & U_SHADERPARM4)
		to->shaderparms[4] = readFloat();
	
	if(bits & U_SHADERPARM5)
		to->shaderparms[5] = readFloat();
		
	if(bits & U_SHADERPARM6)
		to->shaderparms[6] = readFloat();
		
	if(bits & U_SHADERPARM7)
		to->shaderparms[7] = readFloat();
		
	if(bits & U_VECTOR0)
		readVector3(to->vectors[0]);
		
	if(bits & U_VECTOR1)
		readVector3(to->vectors[1]);
		
	if(bits & U_VECTOR2)
		readVector3(to->vectors[2]);
		
	return true;
}

void	bitmessage_c::uncompress(int bits_offset)
{
	if(bits_offset < 0 || bits_offset > (int)_cursize)
		Com_Error(ERR_FATAL, "bitmessage_c::uncompress: bad bits_offset %i, current size %i", bits_offset, _cursize);
	
	
	_readcount = bits_offset;
	
	// make backup
	boost::dynamic_bitset<byte>	bits;		// backup bits + total uncompressed bits
	copyTo(bits, 0, bits_offset);
	
	// uncompress data
	do
	{
		boost::dynamic_bitset<byte>	uncomp;
		
		try
		{
			if(!(readBitsCompressed(uncomp)))
				break;
		}
		catch(std::range_error)
		{
			Com_Printf("bitmessage_c::uncompress: range error while reading compressed bytes\n", _readcount, _cursize);
			throw;
		}
		
		for(boost::dynamic_bitset<byte>::size_type i=0; i<uncomp.size(); ++i)
			bits.push_back(uncomp[i]);
		
	} while(_readcount < _cursize);
	
	// reset and expand _data if needed
	if((bits_offset + bits.size()) > _maxsize)
	{
		_data.clear();
		_data.resize(tobytes(bits_offset + bits.size()), 0);
		_maxsize = tobits(_data.size());
	}
	
	// copy back bits and bytes to _data
	_cursize = 0;
	writeBits(bits);
	
	// reset read count
	_readcount = bits_offset;
}

