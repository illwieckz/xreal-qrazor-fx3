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


// snapped vectors are packed in 13 bits instead of 32
enum
{
	SNAPPED_BITS		= 13,
	MAX_SNAPPED		= (1<<SNAPPED_BITS)
};


bitmessage_c::bitmessage_c()
{
	_allowoverflow = false;
	_overflowed = false;
	_expand = false;
	
	_maxsize = 0;
	_cursize = 0;
	_readcount = 0;
}

bitmessage_c::bitmessage_c(int bits_num, bool allowoverflow, bool expand)
{
	_allowoverflow = allowoverflow;
	_expand = expand;

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

void	bitmessage_c::copyTo(boost::dynamic_bitset<byte> &bits, int bits_offset, int bits_num) const
{
	if(bits_offset < 0 || bits_num < 0 || (bits_offset + bits_num) > (int)_cursize)
		Com_Error(ERR_FATAL, "bitmessage_c::copyTo: bad bits_offset %i + bits_num %i > current size %i", bits_offset, bits_num, _cursize);

	bits = boost::dynamic_bitset<byte>(bits_num);
	
	for(int i=0; i<bits_num; i++)
		bits[i] = _data[(bits_offset+i) >> 3] & (1<<((bits_offset+i) & 7));
}

void	bitmessage_c::copyTo(std::vector<byte> &bytes, int bits_offset, int bits_num) const
{
	if(bits_offset < 0 || bits_num < 0 || (bits_offset + bits_num) > (int)_cursize)
		Com_Error(ERR_FATAL, "bitmessage_c::copyTo: bad bits_offset %i + bits_num %i > current size %i", bits_offset, bits_num, _cursize);
	
	bytes = std::vector<byte>(toBytes(bits_num), 0);
	
	for(int i=0; i<bits_num; i++)
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

	crc.process_bytes(&_data[bits_offset >> 3], toBytes(_cursize));
	
	return crc.checksum();
}

void	bitmessage_c::beginWriting()
{
	_overflowed = false;

	_data = std::vector<byte>(toBytes(_maxsize), 0);
	
	_cursize = 0;
}

void	bitmessage_c::writeBit(bool bit)
{
	if((_cursize + 1) > _maxsize)
	{
		if(!_allowoverflow)
		{
			Com_Error(ERR_DROP, "bitmessage_c::writeBit: overflow without allowoverflow set");
		}
		else
		{
			if(_expand)
			{
				_data.resize(_data.size() + 8);
				_maxsize += 8;
			}
			else
			{
				Com_Printf("bitmessage_c::writeBit: overflow maxsize %i\n", _maxsize);
			
				beginWriting();
			
				_overflowed = true;
		
				throw std::overflow_error(va("bitmessage_c::writeBit: overflow maxsize %i", _maxsize));
			}
		}
	}

	_data[_cursize >> 3] |= (bit << (_cursize % 8));

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
	if(bits_num == 0 || bits_num < -31 || bits_num > 32)
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
	if(!bits.size())
	{
		writeBit(false);	// nothing to follow
		return 1;
	}

	// convert bits to bytes
	std::vector<byte> bytes(toBytes(bits.size()), 0);

	for(uint_t i=0; i<bits.size(); i++)
		bytes[i >> 3] |= (bits[i] << (i % 8));
		
	// compress converted bytes
	std::vector<byte>	bytes_compressed(toBytes(_maxsize - _cursize), 0);
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
				Com_Error(ERR_DROP, "bitmessage_c::writeBytesCompressed: data error");
			
			throw std::exception();
			writeBit(false);
			return 1;
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
	
	return 1+14+11+toBits(size_compressed);
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

int	bitmessage_c::writeBytesCompressed(const std::vector<byte> &bytes, bool skip_if_fails)
{
	boost::dynamic_bitset<byte> bits;
	bytesToBits(bytes, bits);

	return writeBitsCompressed(bits, skip_if_fails);
}

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

void 	bitmessage_c::writeVec(vec_t v)
{
#ifdef DOUBLEVEC_T
#error bitmessage_c::writeVec TODO
#else
	union
	{
		float	f;
		int	l;
	} dat;
	
	dat.f = v;
	dat.l = LittleLong(dat.l);
	
	try
	{
		if(	(float)dat.l == dat.f
			&& dat.l + MAX_SNAPPED/2 >= 0
			&& dat.l + MAX_SNAPPED/2 < MAX_SNAPPED	)
		{
			writeBit(true);	// mark as compressed
			writeBits(dat.l + MAX_SNAPPED/2, SNAPPED_BITS);
		}
		else
		{
			writeBit(false);
			writeBits(dat.l, 32);
		}
	}
	catch(std::overflow_error)
	{
		Com_Printf("bitmessage_c::writeFloat: overflow error\n");
		throw;
	}
#endif
}

void 	bitmessage_c::writeVec3(const vec3_c &v)
{
	writeVec(v[0]);
	writeVec(v[1]);
	writeVec(v[2]);
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
	writeLong(to->msec);
	
	// write current buttons
	if(to->buttons != from->buttons)
	{
		writeBit(true);
		writeByte(to->buttons);
	}
	else
	{
		writeBit(false);
	}

	// write current angles
	if(to->angles[0] != from->angles[0])
	{
		writeBit(true);
		writeFloat(to->angles[0]);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->angles[1] != from->angles[1])
	{
		writeBit(true);
		writeFloat(to->angles[1]);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->angles[2] != from->angles[2])
	{
		writeBit(true);
		writeFloat(to->angles[2]);
	}
	else
	{
		writeBit(false);
	}
	
	// write current movement
	if(to->forwardmove != from->forwardmove)
	{
		writeBit(true);
		writeFloat(to->forwardmove);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->sidemove != from->sidemove)
	{
		writeBit(true);
		writeFloat(to->sidemove);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->upmove != from->upmove)
	{
		writeBit(true);
		writeFloat(to->upmove);
	}
	else
	{
		writeBit(false);
	}	
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
//	Com_Printf("bitmessage_c::writeDeltaEntity: from %i to %i\n", from->getNumber(), to->getNumber());
	
	if(!from && !to)
	{
		Com_Error(ERR_FATAL, "bitmessage_c::writeDeltaEntity: NULL");
	}
	
	if(!to)
	{
		writeBits(from->getNumber(), MAX_ENTITIES_BITS);
		writeBit(true);				// remove entity
		return;
	}

	if(!to->getNumber())
		Com_Error(ERR_FATAL, "Unset entity number");
		
	if(to->getNumber() >= MAX_ENTITIES)
		Com_Error(ERR_FATAL, "Entity number >= MAX_ENTITIES");
		
	
	
	if(((memcmp(from, to, sizeof(entity_state_t)) == 0) && !to->event) && !force)
		return;		// nothing to send!

	//Com_Printf("message_c::writeDeltaEntity: %3i %3i\n", to->getNumber());
	
	writeBits(to->getNumber(), MAX_ENTITIES_BITS);
	writeBit(false);	// keep entity

	if(to->type != from->type)
	{
		writeBit(true);
		writeByte(to->type);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->origin != from->origin)
	{
		writeBit(true);
		writeVec3(to->origin);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->origin2 != from->origin2)
	{
		writeBit(true);
		writeVec3(to->origin2);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->quat != from->quat)
	{
		writeBit(true);
		writeQuaternion(to->quat);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->quat2 != from->quat2)
	{
		writeBit(true);
		writeQuaternion(to->quat2);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->velocity_linear != from->velocity_linear)
	{
		writeBit(true);
		writeVec3(to->velocity_linear);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->velocity_angular != from->velocity_angular)
	{
		writeBit(true);
		writeVec3(to->velocity_angular);
	}
	else
	{
		writeBit(false);
	}
				
	if(to->index_model != from->index_model)
	{
		writeBit(true);
		writeBits(to->index_model, MAX_MODELS_BITS);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->index_shader != from->index_shader)
	{
		writeBit(true);
		writeBits(to->index_shader, MAX_SHADERS_BITS);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->index_animation != from->index_animation)
	{
		writeBit(true);
		writeBits(to->index_animation, MAX_ANIMATIONS_BITS);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->index_sound != from->index_sound)
	{
		writeBit(true);
		writeBits(to->index_sound, MAX_SOUNDS_BITS);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->index_light != from->index_light)
	{
		writeBit(true);
		writeBits(to->index_light, MAX_LIGHTS_BITS);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->frame != from->frame)
	{
		writeBit(true);
		writeShort(to->frame);
	}
	else
	{
		writeBit(false);
	}

	if(to->effects != from->effects)
	{
		writeBit(true);
		writeLong(to->effects);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->renderfx != from->renderfx)
	{
		writeBit(true);
		writeLong(to->renderfx);
	}
	else
	{
		writeBit(false);
	}

	if(to->event)	// event is not delta compressed, just 0 compressed
	{
		writeBit(true);
		writeByte(to->event);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->shaderparms[0] != from->shaderparms[0])
	{
		writeBit(true);
		writeFloat(to->shaderparms[0]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->shaderparms[1] != from->shaderparms[1])
	{
		writeBit(true);
		writeFloat(to->shaderparms[1]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->shaderparms[2] != from->shaderparms[2])
	{
		writeBit(true);
		writeFloat(to->shaderparms[2]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->shaderparms[3] != from->shaderparms[3])
	{
		writeBit(true);
		writeFloat(to->shaderparms[3]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->shaderparms[4] != from->shaderparms[4])
	{
		writeBit(true);
		writeFloat(to->shaderparms[4]);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->shaderparms[5] != from->shaderparms[5])
	{
		writeBit(true);
		writeFloat(to->shaderparms[5]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->shaderparms[6] != from->shaderparms[6])
	{
		writeBit(true);
		writeFloat(to->shaderparms[6]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->shaderparms[7] != from->shaderparms[7])
	{
		writeBit(true);
		writeFloat(to->shaderparms[7]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->vectors[0] != from->vectors[0])
	{
		writeBit(true);
		writeVec3(to->vectors[0]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->vectors[1] != from->vectors[1])
	{
		writeBit(true);
		writeVec3(to->vectors[1]);
	}
	else
	{
		writeBit(false);
	}
		
	if(to->vectors[2] != from->vectors[2])
	{
		writeBit(true);
		writeVec3(to->vectors[2]);
	}
	else
	{
		writeBit(false);
	}	
}

void	bitmessage_c::writeDeltaPlayerState(const player_state_t *from, const player_state_t *to)
{
	//
	// write the pmove_state_t
	//
	if(to->pmove.pm_type != from->pmove.pm_type)
	{
		writeBit(true);
		writeByte(to->pmove.pm_type);
	}
	else
	{
		writeBit(false);
	}

	if(to->pmove.origin != from->pmove.origin)
	{
		writeBit(true);
		writeVec3(to->pmove.origin);
	}
	else
	{
		writeBit(false);
	}

	if(to->pmove.velocity_linear != from->pmove.velocity_linear)
	{
		writeBit(true);
		writeVec3(to->pmove.velocity_linear);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->pmove.velocity_angular != from->pmove.velocity_angular)
	{
		writeBit(true);
		writeVec3(to->pmove.velocity_angular);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->pmove.pm_flags != from->pmove.pm_flags)
	{
		writeBit(true);
		writeByte(to->pmove.pm_flags);
	}
	else
	{
		writeBit(false);
	}

	if(to->pmove.pm_time != from->pmove.pm_time)
	{
		writeBit(true);
		writeByte(to->pmove.pm_time);
	}
	else
	{
		writeBit(false);
	}

	if(to->pmove.gravity != from->pmove.gravity)
	{
		writeBit(true);
		writeFloat(to->pmove.gravity);
	}
	else
	{
		writeBit(false);
	}

	if(to->pmove.delta_angles != from->pmove.delta_angles)
	{
		writeBit(true);
		writeAngle(to->pmove.delta_angles[0]);
		writeAngle(to->pmove.delta_angles[1]);
		writeAngle(to->pmove.delta_angles[2]);
	}
	else
	{
		writeBit(false);
	}
	
	//
	// write the rest of the player_state_t
	//
	if(to->view_angles != from->view_angles)
	{
		writeBit(true);
		writeAngle(to->view_angles[0]);
		writeAngle(to->view_angles[1]);
		writeAngle(to->view_angles[2]);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->view_offset != from->view_offset)
	{
		writeBit(true);
		writeVec3(to->view_offset);
	}
	else
	{
		writeBit(false);
	}

	if(to->kick_angles != from->kick_angles)
	{
		writeBit(true);
		writeAngle(to->kick_angles[0]);
		writeAngle(to->kick_angles[1]);
		writeAngle(to->kick_angles[2]);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->gun_angles != from->gun_angles)
	{
		writeBit(true);
		writeAngle(to->gun_angles[0]/**4*/);
		writeAngle(to->gun_angles[1]/**4*/);
		writeAngle(to->gun_angles[2]/**4*/);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->gun_offset != from->gun_offset)
	{
		writeBit(true);
		writeVec3(to->gun_offset);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->gun_model_index != from->gun_model_index)
	{
		writeBit(true);
		writeBits(to->gun_model_index, MAX_MODELS_BITS);
	}
	else
	{
		writeBit(false);
	}

	if(to->gun_anim_frame != from->gun_anim_frame)
	{
		writeBit(true);
		writeShort(to->gun_anim_frame);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->gun_anim_index != from->gun_anim_index)
	{
		writeBit(true);
		writeBits(to->gun_anim_index, MAX_ANIMATIONS_BITS);
	}
	else
	{
		writeBit(false);
	}
	
	if(to->blend != from->blend)
	{
		writeBit(true);
		writeColor(to->blend);
	}
	else
	{
		writeBit(false);
	}

	if(to->fov != from->fov)
	{
		writeBit(true);
		writeFloat(to->fov);
	}
	else
	{
		writeBit(false);
	}

	if(to->rdflags != from->rdflags)
	{
		writeBit(true);
		writeByte(to->rdflags);
	}
	else
	{
		writeBit(false);
	}
	
	// send stats
	int statbits = 0;
	for(int i=0; i<MAX_STATS; i++)
		if(to->stats[i] != from->stats[i])
			statbits |= 1<<i;
	writeLong(statbits);
	
	for(int i=0; i<MAX_STATS; i++)
		if(statbits & (1<<i))
			writeShort(to->stats[i]);
}


void	bitmessage_c::writeMessage(const bitmessage_c &msg)
{
	boost::dynamic_bitset<byte> bits;
	msg.copyTo(bits, 0, msg._cursize);
	
	writeBits(bits);
}

int	bitmessage_c::writeMessageCompressed(const bitmessage_c &msg, bool skip_if_fails)
{
	boost::dynamic_bitset<byte> bits;
	msg.copyTo(bits, 0, msg._cursize);

	return writeBitsCompressed(bits, skip_if_fails);
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
	if(bits_num == 0 || bits_num < -31 || bits_num > 32)
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
			Com_Printf("bitmessage_c::readBits: range error while reading %i bits to bitmask\n", bits_num);
			throw;
		}
	}
	
	return bitset.to_ulong();
}

void	bitmessage_c::readBits(int bits_num, boost::dynamic_bitset<byte> &bits)
{
//	if(bits_num < 0)
//		bits_num = -bits_num;

//	if(!bits_num)
//		Com_Error(ERR_FATAL, "bitmessage_c::readBits: bad bits number %i", bits_num);
		
	bits = boost::dynamic_bitset<byte>(bits_num);

	for(int i=0; i<bits_num; i++)
	{
		try
		{
			bits[i] = readBit();
		}
		catch(std::range_error)
		{
			Com_Printf("bitmessage_c::readBits: range error while reading %i bits to bitset\n", bits_num);
			throw;
		}
	}
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
		Com_Printf("bitmessage_c::readBytesCompressed: range error while reading initial bit\n");
		throw;
	}

	
	uint_t	bits_num = 0;
	uint_t	bytes_num = 0;
	
	try
	{
		bits_num = readBits(14);	// original bits size
	}
	catch(std::range_error)
	{
		Com_Printf("bitmessage_c::readBytesCompressed: range error while reading original bits size\n");
		throw;
	}
	
	try
	{
		bytes_num = readBits(11);	// compressed bytes size
	}
	catch(std::range_error)
	{
		Com_Printf("bitmessage_c::readBytesCompressed: range error while reading compressed bytes size\n");
		throw;
	}
	
	if(!bits_num)
	{
		bits.clear();
		return false;
	}
	
	// read the compressed bytes
	std::vector<byte>	data_compressed(bytes_num, 0);
	readBytes(&data_compressed[0], data_compressed.size());

	// uncompress the bytes
	std::vector<byte>	data_uncompressed(MAX_MSGLEN, 0);
	unsigned long		length_uncompressed = data_uncompressed.size();
	
	int err = ::uncompress(&data_uncompressed[0], &length_uncompressed, &data_compressed[0], data_compressed.size());
	
	switch(err)
	{
		case Z_NEED_DICT:	// 2
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: need dictionary");
			return false;
			
		case Z_STREAM_END:	// 1
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: stream end");
			return false;
			
		case Z_OK:		// 0
			break;
			
		case Z_ERRNO:		// -1
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: file error");
			return false;
			
		case Z_STREAM_ERROR:	// -2
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: stream error");
			return false;
			
		case Z_DATA_ERROR:	// -3
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: data error");
			return false;
		
		case Z_MEM_ERROR:	// -4
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: insufficient memory");
			return false;
			
		case Z_BUF_ERROR:	// -5
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: buffer error");
			return false;
			
		case Z_VERSION_ERROR:	// -6
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: incompatible version");
			return false;
			
		default:
			Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: unknown error");
			return false;
	}
	
	if(length_uncompressed != toBytes(bits_num))
	{
		Com_Error(ERR_DROP, "bitmessage_c::readBitsCompressed: uncompressed size %i is not excpeted size %i", length_uncompressed, toBytes(bits_num));
		bits.clear();
		return false;
	}
	
	// copy uncompressed bytes into bits buffer
	bits = boost::dynamic_bitset<byte>(bits_num);
	
	for(uint_t i=0; i<bits_num; i++)
		bits[i] = data_uncompressed[i >> 3] & (1 << (i & 7));
	
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

bool	bitmessage_c::readBytesCompressed(std::vector<byte> &bytes)
{
	boost::dynamic_bitset<byte> bits;
	bool ret = readBitsCompressed(bits);
	bytesToBits(bytes, bits);
	
	return ret;
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

vec_t	bitmessage_c::readVec()
{
#ifdef DOUBLEVEC_T
#error bitmessage_c::readVec TODO
#else
	union
	{
		float	f;
		int	l;
	} dat;
	
	if(readBit())
	{
		dat.l = 0;
		dat.l = readBits(13) - 0x1000;
	}
	else
	{
		dat.l = readBits(32);
	}
	
	dat.l = LittleLong(dat.l);

	return dat.f;
#endif
}

void 	bitmessage_c::readVec3(vec3_c &v)
{
	v[0] = readVec();
	v[1] = readVec();
	v[2] = readVec();
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
	// set everything to the state we are delta'ing from
	*to = *from;
	
	// read time to run command
	to->msec = readLong();
	
	// read buttons
	if(readBit())
		to->buttons = readByte();

	// read current angles
	if(readBit())
		to->angles[0] = readFloat();
		
	if(readBit())
		to->angles[1] = readFloat();
		
	if(readBit())
		to->angles[2] = readFloat();
		
	// read movement
	if(readBit())
		to->forwardmove = readFloat();
		
	if(readBit())
		to->sidemove = readFloat();
		
	if(readBit())
		to->upmove = readFloat();	
}


/*
==================
CG_ParseDelta

Can go from either a baseline or a previous packet_entity
==================
*/
void	bitmessage_c::readDeltaEntity(const entity_state_t *from, entity_state_t *to, int number)
{
	// set everything to the state we are delta'ing from
	*to = *from;
	to->number = number;
	
	bool remove = readBit();
	if(remove)
	{
		to->clear();
		return;
	}

	if(readBit())
		to->type = (entity_type_e)readByte();
	
	if(readBit())
		readVec3(to->origin);
		
	if(readBit())
		readVec3(to->origin2);
		
	if(readBit())
		readQuaternion(to->quat);
		 
	if(readBit())
		readQuaternion(to->quat2);
		 
	if(readBit())
		readVec3(to->velocity_linear);
		
	if(readBit())
		readVec3(to->velocity_angular);

	if(readBit())
		to->index_model = readBits(MAX_MODELS_BITS);
		
	if(readBit())
		to->index_shader = readBits(MAX_SHADERS_BITS);
		
	if(readBit())
		to->index_animation = readBits(MAX_ANIMATIONS_BITS);
		
	if(readBit())
		to->index_sound = readBits(MAX_SOUNDS_BITS);
		
	if(readBit())
		to->index_light = readBits(MAX_LIGHTS_BITS);
		
	if(readBit())
		to->frame = readShort();
	
	if(readBit())
		to->effects = readLong();
	
	if(readBit())
		to->renderfx = readLong();

	if(readBit())
		to->event = readByte();
	else
		to->event = 0;
		
	if(readBit())
		to->shaderparms[0] = readFloat();
		
	if(readBit())
		to->shaderparms[1] = readFloat();
		
	if(readBit())
		to->shaderparms[2] = readFloat();
		
	if(readBit())
		to->shaderparms[3] = readFloat();
		
	if(readBit())
		to->shaderparms[4] = readFloat();
	
	if(readBit())
		to->shaderparms[5] = readFloat();
		
	if(readBit())
		to->shaderparms[6] = readFloat();
		
	if(readBit())
		to->shaderparms[7] = readFloat();
		
	if(readBit())
		readVec3(to->vectors[0]);
		
	if(readBit())
		readVec3(to->vectors[1]);
		
	if(readBit())
		readVec3(to->vectors[2]);
}

void	bitmessage_c::readDeltaPlayerState(const player_state_t *from, player_state_t *to)
{
	// set everything to the state we are delta'ing from
	*to = *from;
	
	//
	// parse the pmove_state_t
	//
	if(readBit())
		to->pmove.pm_type = (pm_type_e) readByte();

	if(readBit())
		readVec3(to->pmove.origin);

	if(readBit())
		readVec3(to->pmove.velocity_linear);
		
	if(readBit())
		readVec3(to->pmove.velocity_angular);
		
	if(readBit())
		to->pmove.pm_flags = readByte();

	if(readBit())
		to->pmove.pm_time = readByte();

	if(readBit())
		to->pmove.gravity = readFloat();

	if(readBit())
	{
		to->pmove.delta_angles[0] = readAngle();
		to->pmove.delta_angles[1] = readAngle();
		to->pmove.delta_angles[1] = readAngle();
	}

//	if(cl.attractloop)	FIXME
//		to->pmove.pm_type = PM_FREEZE;		// demo playback

	//
	// parse the rest of the player_to_t
	//
	if(readBit())
	{		
		to->view_angles[0] = readAngle();
		to->view_angles[1] = readAngle();
		to->view_angles[2] = readAngle();
	}
	
	if(readBit())
		readVec3(to->view_offset);

	if(readBit())
	{
		to->kick_angles[0] = readAngle();
		to->kick_angles[1] = readAngle();
		to->kick_angles[2] = readAngle();
	}
	
	if(readBit())
	{
		to->gun_angles[0] = readAngle();
		to->gun_angles[1] = readAngle();
		to->gun_angles[2] = readAngle();
	}
	
	if(readBit())
		readVec3(to->gun_offset);

	if(readBit())
		to->gun_model_index = readBits(MAX_MODELS_BITS);

	if(readBit())
		to->gun_anim_frame = readShort();
	
	if(readBit())
		to->gun_anim_index = readBits(MAX_ANIMATIONS_BITS);

	if(readBit())
		readColor(to->blend);

	if(readBit())
		to->fov = readFloat();

	if(readBit())
		to->rdflags = readByte();

	// parse stats
	int statbits = readLong();
	for(int i=0; i<MAX_STATS; i++)
		if(statbits & (1<<i))
			to->stats[i] = readShort();
}



