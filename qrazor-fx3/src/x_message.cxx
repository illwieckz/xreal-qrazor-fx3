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
#include <bitset>
#include <boost/dynamic_bitset.hpp>
#include <zlib.h>

// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "x_message.h"
#include "x_protocol.h"



message_c::message_c()
{
	_type = MSG_TYPE_UNINIT;
	_allowoverflow = false;
	_overflowed = false;
	
	_data = std::vector<byte>(0);
	_maxsize = 0;
	_cursize = 0;
	_readcount_bytes = 0;
	_readcount_bits = 0;
}

message_c::message_c(message_type_t type, int data_size, bool allowoverflow)
{
	_type = type;
	_allowoverflow = allowoverflow;
	_overflowed = false;
	
	_data = std::vector<byte>(data_size);
	_maxsize = data_size;
	_cursize = 0;
	_readcount_bytes = 0;
	_readcount_bits = 0;
}

void	message_c::clear()
{
	_overflowed = false;
	_cursize = 0;
}

void	message_c::beginWriting()
{
	_type = MSG_TYPE_RAWBYTES;
	_overflowed = false;
	_cursize = 0;
}

void*	message_c::getSpace(int length)
{
	if(length < 0)
	{
		Com_Error(ERR_DROP, "message_c::getSpace: length %i", length);
		return NULL;
	}

	if((_cursize + length) > _maxsize)
	{
		if(!_allowoverflow)
			Com_Error(ERR_FATAL, "message_c::getSpace: overflow without allowoverflow set");
		
		if(length > (int)_maxsize)
			Com_Error(ERR_FATAL, "message_c::getSpace: %i is > full buffer size", length);
			
		Com_Printf("MSG_GetSpace: overflow\n");
		
		clear(); 
		
		_overflowed = true;
	}

	void *data = &_data[_cursize];
	
	_cursize += length;
	
	return data;
}

void	message_c::addSpace(int length)
{
	//std::vector<byte> v(length);
	//std::copy(_data.begin(), _data.end(), std::back_inserter(v));
	
	if(!length)
	{
		Com_Error(ERR_DROP, "message_c::addSpace: length %i", length);
		return;
	}
	
	for(int i=0; i<length; i++)
	{
		byte b = 0;
		
		_data.push_back(b);
	}
	
	_maxsize += length;
}

void	message_c::writeRawBytes(int c, int bits)
{
	byte* buf;
	
	if(bits <= 8)
	{
		buf = (byte*)getSpace(1);
		buf[0] = c;
	}
	else if(bits <= 16)
	{
		buf = (byte*)getSpace(2);
		buf[0] = c&0xff;
		buf[1] = c>>8;
	}
	else if(bits <= 32)
	{
		buf = (byte*)getSpace(4);
		buf[0] = c&0xff;
		buf[1] = (c>>8)&0xff;
		buf[2] = (c>>16)&0xff;
		buf[3] = c>>24;
	}
}

void	message_c::writeRawBits(int c, int bits)
{
	//FIXME

	union
	{
		int	l;
		byte	b[4];
	} dat;
	
	dat.l = c;

	byte* buf = (byte*)getSpace(bits / 8);
	
	for(int i=0; i<bits; i++)
	{
		buf[bits >> 3] = 0;
	}

	for(int i=0; i<bits; i++)
	{
		//int val = dat.b[i>>3] >> (i&7);
		//buf[i>>3] |= (val & 1) << i;
	
		//buf[ >> 3] |= dat.b[bits >> 3];
		
		buf[i>>3] |= (c>>i) & 1;
	}
}

void	message_c::write(const void *data, int length)
{
	memcpy(getSpace(length), data, length);
}

void	message_c::print(const char *data)
{
	int len = strlen(data)+1;

	if(_cursize)
	{
		if(_data[_cursize -1])
			memcpy((byte*)getSpace(len), data, len);	// no trailing 0
		else
			memcpy((byte*)getSpace(len -1) -1, data, len);	// write over trailing 0
	}
	else
	{
		memcpy((byte*)getSpace(len), data, len);
	}
}

void	message_c::writeRawData(const void *data, int length)
{
	if(length == 0)
		return;
		
	if(length < 0)
	{
		Com_Error(ERR_FATAL, "message_c::writeRawData: length %i", length);
		return;
	}

	memcpy(getSpace(length), data, length);
}

bool	message_c::writeCompressedData(const void *data, int length, bool skip)
{
	if(length == 0)
		return true;

	if(length < 0)
	{
		Com_Error(ERR_FATAL, "message_c::writeCompressedData: length %i", length);
		return false;
	}

	byte		data_compressed[MAX_PACKETLEN];
	unsigned long	length_compressed = sizeof(data_compressed);
	
	int err = compress(data_compressed, &length_compressed, (byte*)data, length);
	
	if(err != Z_OK)
	{
		Com_Error(ERR_DROP, "message_c::writeCompressedData: compression error %i", err);
		return false;
	}
	
	//if(skip && ((_cursize + length_compressed) >= _maxsize))
	if(skip && !(_maxsize - _cursize) >= length_compressed)
		return false;
	
	memcpy(getSpace(length_compressed), data_compressed, length_compressed);
	
	return true;
}

void	message_c::writeBits(const boost::dynamic_bitset<> &bits)
{
	writeBits(bits.to_ulong(), bits.size());
}

void	message_c::writeBits(int value, int bits)
{
	if(!bits || bits < -31 || bits > 32)
		Com_Error(ERR_FATAL, "message_c::writeBits: bad bits %i", bits);
		
	if(bits < 0)
		bits = -bits;
		
	switch(_type)
	{
		case MSG_TYPE_UNINIT:
			Com_Error(ERR_FATAL, "message_c::writeBits: message is not initialized");
			break;
		
		case MSG_TYPE_RAWBYTES:
			writeRawBytes(value, bits);
			break;
		
		case MSG_TYPE_RAWBITS:
			writeRawBits(value, bits);
			break;
	}
}

void 	message_c::writeByte(int c)
{
	writeBits(c, 8);
}

void 	message_c::writeShort(int c)
{
	writeBits(c, 16);
}

void 	message_c::writeLong(int c)
{
	writeBits(c, 32);
}

void 	message_c::writeFloat(float f)
{
	union
	{
		float	f;
		int	l;
	} dat;
	
	dat.f = f;
	dat.l = LittleLong(dat.l);
	
	writeBits(dat.l, 32);
}

void 	message_c::writeString(const char *s)
{
	if(!s)
		write((void*)"", 1);
	else
		write(s, strlen(s)+1);
}

void 	message_c::writeCoord(float f)
{
	writeFloat(f);
}

void 	message_c::writeVector3(const vec3_c &v)
{
	writeFloat(v[0]);
	writeFloat(v[1]);
	writeFloat(v[2]);
}

void	message_c::writeAngle(float f)
{
	//MSG_WriteByte (sb, (int)(f*256/360) & 255);
	writeFloat(f);
}

void 	message_c::writeDir(const vec3_c &dir)
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


void	message_c::writeColor(const vec4_c &color)
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

void	message_c::writeQuaternion(const quaternion_c &q)
{
	writeFloat(q._q[0]);
	writeFloat(q._q[1]);
	writeFloat(q._q[2]);
	writeFloat(q._q[3]);
}


void 	message_c::writeDeltaUsercmd(const usercmd_t *from, const usercmd_t *to)
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
void 	message_c::writeDeltaEntity(const entity_state_t *from, const entity_state_t *to, bool force)
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




void 	message_c::beginReading()
{
	_readcount_bytes = 0;
	_readcount_bits = 0;
}


int	message_c::readRawBytes(int bits)
{
	int	bitmask = -1;
	
	if(bits <= 8)
	{
		bitmask = (unsigned char)_data[_readcount_bytes];
		
		_readcount_bytes += 1;
		_readcount_bits += 8;
	}
	else if(bits <= 16)
	{
		bitmask = (short)(_data[_readcount_bytes]
		+ (_data[_readcount_bytes+1]<<8));
		
		_readcount_bytes += 2;
		_readcount_bits += 16;
	}
	else if(bits <= 32)
	{
		bitmask = _data[_readcount_bytes]
		+ (_data[_readcount_bytes+1]<<8)
		+ (_data[_readcount_bytes+2]<<16)
		+ (_data[_readcount_bytes+3]<<24);
	
		_readcount_bytes += 4;
		_readcount_bits += 32;
	}
	
	return bitmask;
}

int	message_c::readRawBits(int bits)
{
#if 1
	boost::dynamic_bitset<>	bitmask(bits);

	for(int i=0; i<bits; i++)
	{
		int val = _data[_readcount_bits >> 3] >> (_readcount_bits & 7);
		_readcount_bits++;
		bitmask[i] = (val & 1) << i;
	}
	
	_readcount_bytes += bits / 8;
	
	return bitmask.to_ulong();

#elif 0
	std::bitset<32>	bitmask;

	for(int i=0; i<bits; i++)
	{
		int val = _data[_readcount_bits >> 3] >> (_readcount_bits & 7);
		_readcount_bits++;
		bitmask[i] = (val & 1) << i;
	}
	
	_readcount_bytes += bits / 8;
	
	return bitmask.to_ulong();

#else
	int	bitmask = 0;

	for(int i=0; i<bits; i++)
	{
		int val = _data[_readcount_bits >> 3] >> (_readcount_bits & 7);
		_readcount_bits++;
		bitmask |= (val & 1) << i;
	}
	
	_readcount_bytes += bits / 8;

	return bitmask;
#endif
}

int	message_c::readBits(int bits)
{
	int	bitmask = -1;

	if(!bits || bits < -31 || bits > 32)
		Com_Error(ERR_FATAL, "readBits: bad bits %i", bits);
		
	if(bits < 0)
		bits = -bits;
		
	switch(_type)
	{
		case MSG_TYPE_UNINIT:
			Com_Error(ERR_FATAL, "readBits: message is not initialized");
			break;
		
		case MSG_TYPE_RAWBYTES:
			bitmask = readRawBytes(bits);
			break;
		
		case MSG_TYPE_RAWBITS:
			bitmask = readRawBits(bits);
			break;
	}
	
	return bitmask;
}


int 	message_c::readByte()
{
	if(_readcount_bytes + 1 > _cursize)
		return -1;
	else
		return readBits(8);
}

int 	message_c::readShort()
{
	if(_readcount_bytes + 2 > _cursize)
		return -1;
	else		
		return readBits(16);
}

int 	message_c::readLong()
{
	if(_readcount_bytes + 4 > _cursize)
		return -1;
	else		
		return readBits(32);
}

float	message_c::readFloat()
{
	union
	{
		float	f;
		int	l;
	} dat;
	
	if(_readcount_bytes + 4 > _cursize)
		dat.f = -1;
	else		
		dat.l = readBits(32);
	
	dat.l = LittleLong(dat.l);

	return dat.f;
}

char*	message_c::readString()
{
	static char	string[MAX_STRING_CHARS];
	int		l,c;
	
	l = 0;
	do
	{
		c = readByte();
		if(c == -1 || c == 0)
			break;
		string[l] = c;
		l++;
	} while(l < (int)sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

char*	message_c::readStringLine()
{
	static char	string[MAX_STRING_CHARS];
	int		l,c;
	
	l = 0;
	do
	{
		c = readByte();
		if(c == -1 || c == 0 || c == '\n')
			break;
		string[l] = c;
		l++;
	} while(l < (int)sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

float	message_c::readCoord()
{
	return readFloat();
}

void 	message_c::readVector3(vec3_c &v)
{
	v[0] = readFloat();
	v[1] = readFloat();
	v[2] = readFloat();
}

float	message_c::readAngle()
{
	//return MSG_ReadChar(msg_read) * (360.0/256);
	return readFloat();
}

void	message_c::readDir(vec3_c &dir)
{
	int b = readByte();
	
	if(b >= NUMVERTEXNORMALS)
		Com_Error(ERR_DROP, "message_C::readDir: out of range");
		
	dir = bytedirs[b];
}

void 	message_c::readColor(vec4_c &color)
{
	color[0] = readByte() * (1.0/255);
	color[1] = readByte() * (1.0/255);
	color[2] = readByte() * (1.0/255);
	color[3] = readByte() * (1.0/255);
}

void	message_c::readQuaternion(quaternion_c &q)
{
	q._q[0] = readFloat();
	q._q[1] = readFloat();
	q._q[2] = readFloat();
	q._q[3] = readFloat();
}

void 	message_c::readRawData(void *data, int length)
{
	for(int i=0; i<length; i++)
		((byte*)data)[i] = readByte();
}

void	message_c::readCompressedData(void *data, int length)
{
	if(!length)
	{
		Com_Error(ERR_FATAL, "message_c::writeCompressedData: length %i", length);
		return;
	}

	byte		data_uncompressed[MAX_MSGLEN];
	unsigned long	length_uncompressed;
	
	int err = ::uncompress(data_uncompressed, &length_uncompressed, &_data[_readcount_bytes], length);
	
	if(err != Z_OK)
	{
		Com_Error(ERR_DROP, "message_c::readCompressedData: uncompression error %i", err);
		return;
	}
	
	memcpy(data, data_uncompressed, length_uncompressed);
}

void	message_c::readDeltaUsercmd(const usercmd_t *from, usercmd_t *to)
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
bool	message_c::readDeltaEntity(const entity_state_t *from, entity_state_t *to, int number)
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


void	message_c::uncompress()
{
	if(_readcount_bytes == _cursize)
	{
		// nothing left to uncompress
		return;
	}

	byte		data_uncompressed[MAX_MSGLEN];
	unsigned long	length_uncompressed = sizeof(data_uncompressed);
	
	int err = ::uncompress(data_uncompressed, &length_uncompressed, &_data[_readcount_bytes], _cursize - _readcount_bytes);
	
	if(err != Z_OK)
	{
		Com_Error(ERR_DROP, "message_c::uncompress: uncompression error %i", err);
		return;
	}
	
	if((_readcount_bytes + length_uncompressed) > _maxsize)
	{
		//Com_Error(ERR_DROP, "message_c::uncompress: uncompressed data > MAX_MSGLEN");
		addSpace((_readcount_bytes + length_uncompressed) - _maxsize);
		_cursize = _maxsize;
	}
	else
	{
		_cursize = _readcount_bytes + length_uncompressed;
	}
	
	memcpy(&_data[_readcount_bytes], data_uncompressed, length_uncompressed);
}


