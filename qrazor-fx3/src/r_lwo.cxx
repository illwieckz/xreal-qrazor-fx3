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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include 	"r_lwo.h"

// xreal --------------------------------------------------------------------


r_lwo_model_c::r_lwo_model_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_static_model_c(name, buffer, buffer_size)
{
	ri.Com_Printf("r_lwo_model_c::ctor:\n");
}

r_lwo_model_c::~r_lwo_model_c()
{
}

void	r_lwo_model_c::load()
{
	U4	datasize;
	U4	type, size;
	ID4	id;
	r_mesh_c*	mesh = NULL;

	_readcount = 0;
	
	/* Make sure the Lightwave file is an IFF file. */
	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	if(type != ID_FORM)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::load: Not an IFF file (Missing FORM tag)");
	
	datasize = readU4();
	
	if(datasize != (_buffer_size - 8))
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::load: datasize(%i) != _buffer_size(%i)", datasize, (_buffer_size - 8));
	
	ri.Com_DPrintf("FORM [%d]\n", datasize);
	
	/* Make sure the IFF file has a LWO2 form type. */
	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	if(type != ID_LWO2)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::load: Not a lightwave object (Missing LWO2 tag)");
		
	ri.Com_DPrintf("LWO2\n");
	
	/* Read all Lightwave chunks. */
	while(_readcount < datasize)
	{
		readID4(id);
		size = readU4();
		
		_readcount_old = _readcount;
		
		type = MAKE_ID(id[0], id[1], id[2], id[3]);
		switch(type)
		{
			case ID_TAGS:
				readTags(size);
				break;
			/*
			case ID_CLIP:
				readClip(size);
				break;
				
			case ID_ENVL:
				readEnvl(size);
				break;
			*/
			case ID_LAYR:
				mesh = readLayr(size);
				addMesh(mesh);
				break;
			
			case ID_PNTS:
				readPnts(size, mesh);
				break;
				
			case ID_BBOX:
				readBbox(size, mesh);
				break;
			
			case ID_POLS:
				readPols(size, mesh);
				break;
			/*
			case ID_PTAG:
				readPtag(size);
				break;
				
			case ID_VMAP:
				readVmap(size);
				break;
			
			case ID_SURF:
				read_surf(size);
				break;
			*/
				
			default:
				_readcount += size;
				ri.Com_Printf("%s [%d]\n", id, size);
				break;
		}
		
		if(_readcount != (_readcount_old + size))
			ri.Com_Error(ERR_DROP, "r_lwo_model_c::load: bad readcount %i vs. %i", _readcount, (_readcount_old + size));
	}
	
#if 1
	ri.Com_Printf("r_lwo_model_c::load: model '%s' has %i meshes\n", getName(), _meshes.size());
	ri.Com_Printf("r_lwo_model_c::load: model '%s' has %i shaders\n", getName(), _shaders.size());
#endif
}

ushort_t	r_lwo_model_c::readU2()
{
	if(_readcount + 2 > _buffer_size)
		return 0;

	int c =	_buffer[_readcount] + (_buffer[_readcount+1] << 8);
		
	_readcount += 2;
	
	return BigShort(c);
}

uint_t	r_lwo_model_c::readU4()
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

float	r_lwo_model_c::readF4()
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

void	r_lwo_model_c::readID4(char *id)
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

void	r_lwo_model_c::readVEC12(vec3_c &vec)
{
	vec[0] = readF4();
	vec[2] = readF4();
	vec[1] = readF4();
}

void	r_lwo_model_c::readVX(index_t &vx)
{
	if(_readcount + 1 > _buffer_size)
		return;

	vx = 0;
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
}

void	r_lwo_model_c::readName(char *name)
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

void	r_lwo_model_c::readTags(uint_t nbytes)
{
	uint_t	n = 0;
	char	name[255];

	ri.Com_DPrintf("TAGS [%d]\n", nbytes);
	
	while(_readcount < (_readcount_old + nbytes)) 
	{
		readName(name);
		ri.Com_DPrintf("\t[%d] [%s]\n", n++, name);
		addShader(new r_model_shader_c(name, R_RegisterShader(name), X_SURF_NONE, X_CONT_NONE));
	}
}

r_mesh_c*	r_lwo_model_c::readLayr(uint_t nbytes)
{
	ushort_t	flags[2], parent;
	vec3_c		pivot;
	char		name[255];
	r_mesh_c*	mesh = new r_mesh_c();
	
	ri.Com_DPrintf("LAYR [%d]\n", nbytes);

	/*  Layer no. and flags  */
	flags[0] = readU2();
	flags[1] = readU2();

	/*  Pivot point  */
	readVEC12(pivot);
	readName(name);

	ri.Com_DPrintf(" NO [%d] NAME [%s]\n", flags[0], name);
	ri.Com_DPrintf("\tFLAGS [0x%04x] PIVOT [%f,%f,%f]\n", flags[1], pivot[0], pivot[1], pivot[2]);

	if((nbytes - _readcount) == sizeof(U2))
	{
		parent = readU2();
		ri.Com_DPrintf("\tPARENT [%d]\n", parent);
	}
	
	return mesh;
}

void	r_lwo_model_c::readPnts(uint_t nbytes, r_mesh_c *mesh)
{
	if(!mesh)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readPnts: NULL mesh");
	
	uint_t points_num = nbytes/sizeof(VEC12);
	if(points_num <= 0)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readPnts: mesh has invalid vertices number %i", points_num);
	
	ri.Com_Printf("PNTS [%d] points_num [%d]\n", nbytes, points_num);
	
	mesh->fillVertexes(points_num);
	
	for(uint_t i=0; i<points_num; i++)
	{
		readVEC12(mesh->vertexes[i]);
		//ri.Com_DPrintf("\t[%d] [%f,%f,%f]\n", i, mesh->vertexes[i][0], mesh->vertexes[i][1], mesh->vertexes[i][2]);
	}
}

void	r_lwo_model_c::readBbox(uint_t nbytes, r_mesh_c *mesh)
{
	if(!mesh)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readBBox: NULL mesh");
		
	readVEC12(mesh->bbox._mins);
	readVEC12(mesh->bbox._maxs);
	
	_bbox.addPoint(mesh->bbox._mins);
	_bbox.addPoint(mesh->bbox._maxs);
	
	ri.Com_DPrintf("BBOX [%d]\n", nbytes);
	ri.Com_DPrintf("\tMIN [%f,%f,%f]\n", mesh->bbox._mins[0], mesh->bbox._mins[1], mesh->bbox._mins[2]);
	ri.Com_DPrintf("\tMAX [%f,%f,%f]\n", mesh->bbox._maxs[0], mesh->bbox._maxs[1], mesh->bbox._maxs[2]);
}

void	r_lwo_model_c::readPols(uint_t nbytes, r_mesh_c *mesh)
{
	ushort_t	numvert, flags;
	uint_t		nPols;
	index_t		vx;
	ID4		id;
	uint_t		type;
	
	if(!mesh)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readPols: NULL mesh");
	
	ri.Com_DPrintf("POLS [%d]", nbytes);
	
	nPols = 0L;

	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	
	ri.Com_DPrintf(" [%s]\n", id);

	while(_readcount < (_readcount_old + nbytes))
	{
		numvert = readU2();
		flags      = (0xFC00 & numvert) >> 10;
		numvert    =  0x03FF & numvert;
		//ri.Com_DPrintf("\t[%d] NVERT[%d] FLAG[%02x] <", nPols, numvert, flags);
		nPols++;

		for(int n=0; n<(int)numvert; n++)
		{
			readVX(vx);
			
			if(type == ID_FACE && numvert == 3)
			{
				mesh->indexes.push_back(vx);
			}
			
			/*
			if(n+1 == numvert)
				ri.Com_DPrintf("%d>\n", vx);
			else
				ri.Com_DPrintf("%d, ", vx);
			*/
		}
	}
	
	//std::reverse(mesh->indexes.begin(), mesh->indexes.end());
}

