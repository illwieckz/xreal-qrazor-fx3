/// ============================================================================
/*
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
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "cm_lwo.h"

#include "common.h"
#include "vfs.h"
#include "files.h"


cmodel_lwo_c::cmodel_lwo_c(const std::string &name, byte *buffer, uint_t buffer_size)
:cmodel_c(name, buffer, buffer_size)
{
	Com_DPrintf("%p %i\n", buffer, buffer_size);
}

void	cmodel_lwo_c::load()
{
	U4		datasize;
	U4		type, size;
	ID4		id;
	
	matrix_c		transform;
	aabb_c			bbox;
	
	_readcount = 0;
	
	/* Make sure the Lightwave file is an IFF file. */
	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	if(type != ID_FORM)
		Com_Error(ERR_DROP, "cmodel_lwo_c::load: Not an IFF file (Missing FORM tag)");
	
	datasize = readU4();
	
	if(datasize != (_buffer_size - 8))
		Com_Error(ERR_DROP, "cmodel_lwo_c::load: datasize(%i) != _buffer_size(%i)", datasize, (_buffer_size - 8));
	
	Com_DPrintf("FORM [%d]\n", datasize);
	
	/* Make sure the IFF file has a LWO2 form type. */
	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	if(type != ID_LWO2)
		Com_Error(ERR_DROP, "cmodel_lwo_c::load: Not a lightwave object (Missing LWO2 tag)");
		
	Com_DPrintf("LWO2\n");
	
	/* Read all Lightwave chunks. */
	while(_readcount < datasize)
	{
		readID4(id);
		size = readU4();
		
		_readcount_old = _readcount;
		
		type = MAKE_ID(id[0], id[1], id[2], id[3]);
		switch(type)
		{
			/*
			case ID_TAGS:
				readTags(size);
				break;
				
			case ID_CLIP:
				readClip(size);
				break;
				
			case ID_ENVL:
				readEnvl(size);
				break;
			*/
			case ID_LAYR:
				readLayr(size, transform);
				break;
			
			case ID_PNTS:
				readPnts(size);
				break;
				
			case ID_BBOX:
				readBbox(size);
				break;
			
			case ID_POLS:
				readPols(size);
				break;
			
			/*
			case ID_PTAG:
				readPtag(size, ptags);
				break;
				
			case ID_VMAP:
				readVmap(size, vmaps);
				break;

			case ID_VMAD:
				readVmad(size, vmads);
				break;
				
			case ID_SURF:
				read_surf(size);
				break;
			*/
				
			default:
				_readcount += size;
				Com_DPrintf("%s [%d]\n", id, size);
				break;
		}
		
		if(_readcount != (_readcount_old + size))
			Com_Error(ERR_DROP, "cmodel_lwo_c::load: bad readcount %i vs. %i", _readcount, (_readcount_old + size));
	}
}

ushort_t	cmodel_lwo_c::readU2()
{
	if(_readcount + 2 > _buffer_size)
		return 0;

	int c =	_buffer[_readcount] + (_buffer[_readcount+1] << 8);
		
	_readcount += 2;
	
	return BigShort(c);
}

uint_t	cmodel_lwo_c::readU4()
{
	if(_readcount + 4 > _buffer_size)
		return 0;

	int c =	_buffer[_readcount]		+
		(_buffer[_readcount+1] << 8)	+
		(_buffer[_readcount+2] << 16)	+
		(_buffer[_readcount+3] << 24);
		
	_readcount += 4;
	
	return BigLong(c);
}

float	cmodel_lwo_c::readF4()
{
	union
        {
                byte    b[4];
                float   f;
                int     l;
        } dat;

	if(_readcount + 4 > _buffer_size)
		dat.f = 0;

	dat.b[0] = _buffer[_readcount];
	dat.b[1] = _buffer[_readcount+1];
	dat.b[2] = _buffer[_readcount+2];
	dat.b[3] = _buffer[_readcount+3];
		
	_readcount += 4;
	
	dat.l = BigLong(dat.l);
	
	return dat.f;
}

void	cmodel_lwo_c::readID4(char *id)
{
	int   bytesread = 0;
	
	if(_readcount + 4 > _buffer_size)
		return;
	else
	{
		id[bytesread++] = _buffer[_readcount++];
		id[bytesread++] = _buffer[_readcount++];
		id[bytesread++] = _buffer[_readcount++];
		id[bytesread++] = _buffer[_readcount++];
		id[bytesread  ] = 0x00;
	}
}

void	cmodel_lwo_c::readVEC12(vec3_c &vec)
{
	vec[0] = readF4();
	vec[2] = readF4();
	vec[1] = readF4();
}

void	cmodel_lwo_c::readCOL12(vec3_c &vec)
{
	vec[0] = readF4();
	vec[1] = readF4();
	vec[2] = readF4();
}

index_t	cmodel_lwo_c::readVX()
{
	if(_readcount + 1 > _buffer_size)
		return 0;

	index_t vx = 0;
	int c = _buffer[_readcount];
	if(c == 0xFF)
	{
		c += (_buffer[_readcount+1] << 8);
		c += (_buffer[_readcount+2] << 16);
		c += (_buffer[_readcount+3] << 24);
		
		_readcount += 4;
		
		vx = BigLong(c);
	}
	else
	{
		c += (_buffer[_readcount+1] << 8);
		
		_readcount += 2;
		
		vx = BigShort(c);
	}
	
	return vx;
}

void	cmodel_lwo_c::readName(char *name)
{
	int ch, readcount = 0;
	
	do
	{
		ch = _buffer[_readcount + readcount];
		name[readcount++] = ch;
	}while(ch);
	
	if(readcount & 1)
	{
		_readcount++;
	}
	
	_readcount += readcount;
}


void	cmodel_lwo_c::readLayr(uint_t nbytes, matrix_c &transform)
{
	ushort_t	flags[2], parent;
	vec3_c		pivot;
	char		name[255];
	
	Com_DPrintf("LAYR [%d]\n", nbytes);

	/*  Layer no. and flags  */
	flags[0] = readU2();
	flags[1] = readU2();

	/*  Pivot point  */
	readVEC12(pivot);
	transform.setupTranslation(pivot);
	
	readName(name);

	Com_DPrintf(" NO [%d] NAME [%s]\n", flags[0], name);
	Com_DPrintf("\tFLAGS [0x%04x] PIVOT [%f,%f,%f]\n", flags[1], pivot[0], pivot[1], pivot[2]);

	if((nbytes - _readcount) == sizeof(U2))
	{
		parent = readU2();
		Com_DPrintf("\tPARENT [%d]\n", parent);
	}
}

void	cmodel_lwo_c::readPnts(uint_t nbytes)
{
	uint_t points_num = nbytes/sizeof(VEC12);
	if(points_num <= 0)
		Com_Error(ERR_DROP, "cmodel_lwo_c::readPnts: mesh has invalid vertices number %i", points_num);
	
	Com_Printf("PNTS [%d] points_num [%d]\n", nbytes, points_num);
	
	vertexes.resize(points_num);
	
	for(uint_t i=0; i<points_num; i++)
	{
		readVEC12(vertexes[i]);
		//Com_DPrintf("\t[%d] [%f,%f,%f]\n", i, mesh->vertexes[i][0], mesh->vertexes[i][1], mesh->vertexes[i][2]);
	}
}

void	cmodel_lwo_c::readBbox(uint_t nbytes)
{
	readVEC12(_aabb._mins);
	readVEC12(_aabb._maxs);
	
	Com_DPrintf("BBOX [%d]\n", nbytes);
	Com_DPrintf("\tMIN [%f,%f,%f]\n", _aabb._mins[0], _aabb._mins[1], _aabb._mins[2]);
	Com_DPrintf("\tMAX [%f,%f,%f]\n", _aabb._maxs[0], _aabb._maxs[1], _aabb._maxs[2]);
}

void	cmodel_lwo_c::readPols(uint_t nbytes)
{
	ushort_t	numvert, flags;
	uint_t		nPols;
	index_t		vx;
	char		id[5];
	uint_t		type;
	
	Com_DPrintf("POLS [%d]", nbytes);
	
	nPols = 0L;

	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	if(type != ID_FACE)
		Com_Error(ERR_DROP, "cmodel_lwo_c::readVmap: bad id '%s'", id);
	Com_DPrintf(" [%s]\n", id);
	
	indexes.clear();

	while(_readcount < (_readcount_old + nbytes))
	{
		numvert = readU2();
		flags      = (0xFC00 & numvert) >> 10;
		numvert    =  0x03FF & numvert;
		
		if(numvert != 3)
			Com_Error(ERR_DROP, "cmodel_lwo_c::readPols: numvert %i != 3", numvert);
			
		//Com_DPrintf("\t[%d] NVERT[%d] FLAG[%02x] <", nPols, numvert, flags);
		nPols++;

		for(int n=0; n<(int)numvert; n++)
		{
			vx = readVX();
			
			//if(type == ID_FACE && numvert == 3)
			{
				//if(vx < 0 || vx >= mesh->vertexes.size())
				//	continue;
					
				indexes.push_back(vx);
			}
			
			/*
			if(n+1 == numvert)
				Com_DPrintf("%d>\n", vx);
			else
				Com_DPrintf("%d, ", vx);
			*/
		}
	}
	
	// reverse polygon indices to clip normals for ODE
	reverse(indexes.begin(), indexes.end());
}

