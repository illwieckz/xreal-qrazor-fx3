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
	U4		datasize;
	U4		type, size;
	ID4		id;
	r_mesh_c*	mesh = NULL;
	matrix_c	transform;

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
				mesh = readLayr(size, transform);
				addMesh(mesh);
				break;
			
			case ID_PNTS:
				readPnts(size, mesh, transform);
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
			*/	
			case ID_VMAP:
				readVmap(size, mesh);
				break;

			case ID_VMAD:
				readVmad(size, mesh);
				break;
			/*
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

void	r_lwo_model_c::readCOL12(vec3_c &vec)
{
	vec[0] = readF4();
	vec[1] = readF4();
	vec[2] = readF4();
}

index_t	r_lwo_model_c::readVX()
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

r_mesh_c*	r_lwo_model_c::readLayr(uint_t nbytes, matrix_c &transform)
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
	transform.setupTranslation(pivot);
	
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

void	r_lwo_model_c::readPnts(uint_t nbytes, r_mesh_c *mesh, const matrix_c &transform)
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
		mesh->vertexes[i].transform(transform);
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
	char		id[5];
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
			vx = readVX();
			
			if(type == ID_FACE && numvert == 3)
			{
				if(vx < 0 || vx >= mesh->vertexes.size())
					continue;
					
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

void	r_lwo_model_c::readVmap(uint_t nbytes, r_mesh_c *mesh)
{
	ushort_t	dim;
	index_t		vx;
	float		value;
	char		name[255];
	char		id[5];
	uint_t		type;
	
	if(!mesh)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readVmap: NULL mesh");
		
	ri.Com_DPrintf("VMAP [%d]", nbytes);

	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	ri.Com_DPrintf(" [%s]", id);

	dim = readU2();
	readName(name);
	ri.Com_DPrintf(" DIM [%d] NAME [%s]\n", dim, name);

	while(_readcount < (_readcount_old + nbytes))
	{
		vx = readVX();
		
		if(dim == 0)
		{
			//ri.Com_DPrintf("\tVERT[%d]\n", vx);
		}
		else
		{
			//ri.Com_DPrintf("\tVERT[%d] VALS[", vx);
			
			for(int n=0; n<(int)dim; n++)
			{
				value = readF4();
				
				if(type == ID_TXUV && dim == 2)
				{
					try
					{
						if(n==0)
							mesh->texcoords.at(vx)[n] = value;
						else
							mesh->texcoords.at(vx)[n] = 1.0f - value;
					}
					catch(...)
					{
						ri.Com_Error(ERR_DROP, "r_lwo_model_c::loadVmap: exception occured");
					}
				}
				
				/*		
				if(n+1 == dim)
					ri.Com_DPrintf("%f]\n", value);
				else
					ri.Com_DPrintf("%f, " , value);
				*/
			}
		}
	}
}

void	r_lwo_model_c::readVmad(uint_t nbytes, r_mesh_c *mesh)
{
	ushort_t	dim;
	index_t		vx;
	index_t		poly;
	float		value;
	char		name[255];
	char		id[5];
	uint_t		type;
	
	if(!mesh)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readVmad: NULL mesh");
		
	ri.Com_DPrintf("VMAD [%d]", nbytes);

	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	ri.Com_DPrintf(" [%s]", id);

	dim = readU2();
	readName(name);
	ri.Com_DPrintf(" DIM [%d] NAME [%s]\n", dim, name);

	while(_readcount < (_readcount_old + nbytes))
	{
		vx = readVX();
		poly = readVX();//	poly *= 3;
		
		if(dim == 0)
		{
			//ri.Com_DPrintf("\tVERT[%d]\n", vx);
		}
		else
		{
			//ri.Com_DPrintf("\tVERT[%d] VALS[", vx);
			
			for(int n=0; n<(int)dim; n++)
			{
				value = readF4();
				
				if(type == ID_TXUV && dim == 2)
				{
					try
					{
						if(n==0)
							mesh->texcoords.at(vx)[n] = value;
						else
							mesh->texcoords.at(vx)[n] = 1.0f - value;
					}
					catch(...)
					{
						ri.Com_Error(ERR_DROP, "r_lwo_model_c::loadVmad: exception occured");
					}
				}
				
				/*		
				if(n+1 == dim)
					ri.Com_DPrintf("%f]\n", value);
				else
					ri.Com_DPrintf("%f, " , value);
				*/
			}
		}
	}
}

/*
void	r_lwo_model_c::readSurf(uint_t nbytes)
{
	uint_t		bytesread = 0, type, byteshold;
	ushort_t	size, u2[4];
	float		f4[4];
	index_t		vx[4];
	S0		name, source, s0;
	ID4		id;
	vec3_c		col;
	
	ri.Com_DPrintf("SURF [%d]\n", nbytes);
	
	readName(name);
	readName(source);
	
	ri.Com_DPrintf("[%s] [%s]\n", name, source);
	
	while(_readcount < (_readcount_old + nbytes))
	{
		if((nbytes - (_readcount - _readcount_old) < 6)
		{
			_readcount += (nbytes - (_readcount - _readcount_old)//seek_pad((nbytes - bytesread), file);
			return;
		}
		
		// Handle the various sub-chunks
		readID4(id);
		size = readU2();
		type = MAKE_ID(id[0], id[1], id[2], id[3]);
		
		byteshold = _readcount - _readcount_old;
		ri.Com_DPrintf("\t[%s] (%d) ", id, size);
		
		switch(type)
		{
			case ID_COLR:
			case ID_LCOL:
			readCOL12(col);
			vx[0] = readVX();
			ri.Com_DPrintf("<%f,%f,%f> <%d>\n", col[0], col[1], col[2], vx[0]);
			break;
		
			case ID_DIFF:
			case ID_LUMI:
			case ID_SPEC:
			case ID_REFL:
			case ID_TRAN:
			case ID_TRNL:
			case ID_GLOS:
			case ID_SHRP:
			case ID_BUMP:
			case ID_RSAN:
			case ID_RIND:
			case ID_CLRH:
			case ID_CLRF:
			case ID_ADTR:
			case ID_GVAL:
			case ID_LSIZ:
				f4[0] = readF4();
				vx[0] = readVX();
				ri.Com_DPrintf("<%f> <%d>\n", f4[0], vx[0]);
				break;
				
			case ID_SIDE:
			case ID_RFOP:
			case ID_TROP:
				u2[0] = readU2();
				_readcount += (size - sizeof(U2));
				ri.Com_DPrintf("<%d>\n", u2[0]);
				break;
				
			case ID_SMAN:
				f4[0] = readF4();
				ri.Com_DPrintf("<%f>\n", f4[0]);
				break;
			
			case ID_RIMG:
			case ID_TIMG:
				vx[0] = readVX();
				ri.Com_DPrintf("<%d>\n", vx[0]);
				break;
			
			case ID_GLOW:
				u2[0] = readU2();
				f4[0] = readF4();
				vx[0] = readVX();
				f4[1] = readF4();
				vx[1] = readVX();
				ri.Com_DPrintf("<%d> <%f> <%d> <%f> <%d>\n", u2[0], f4[0], vx[0], f4[1], vx[1]);
				break;
			
			case ID_LINE:
				u2[0] = readU2();
				
				if(size > 2)
				{
					f4[0] = readF4();
					vx[0] = readVX();
					
					if(size > 8)
					{
						readCOL12(col);
						vx[1] = readVX();
						
						ri.Com_DPrintf("<%d> <%f> <%d> <%f,%f,%f> <%d>\n", 
						u2[0], f4[0], vx[0], col[0], col[1], col[2], vx[1]);
					}
					else
					{
						ri.Com_DPrintf("<%d> <%f> <%d>\n", u2[0], f4[0], vx[0]);
					}
				}
				else
				{
					ri.Com_DPrintf("<%d>\n", u2[0]);
				}
				break;
				
			case ID_ALPH:
				u2[0] = readU2();
				f4[0] = readF4();
				ri.Com_DPrintf("<%d> <%f>\n", u2[0], f4[0]);
				break;
			
			case ID_AVAL:
				f4[0] = readF4();
				ri.Com_DPrintf("<%f>\n", f4[0]);
				break;
			
			//case ID_BLOK:
			//	ri.Com_DPrintf("\n");
				//TODO
				//bytesread += read_blok(size, file);
				//break;
				
			//case ID_CMNT:
			//	memset(s0, 0x00, sizeof(s0));
			//	bytesread += read_u1(s0, size, file);
			//	printf("<%s>\n", s0);
			//	break;
			//
			
			default:
		bytesread += seek_pad(size, file);
        printf("(%d bytes)\n", size);
      }
	  if ((size - bytesread + byteshold) > 0) {
		  bytesread += seek_pad((size - bytesread + byteshold), file);
	  }
    }

}
*/

