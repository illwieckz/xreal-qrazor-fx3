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
	
	matrix_c		transform;
	cbbox_c			bbox;
	vec2_c			default_texcoord;
	int			default_texcoord_axis[2];
	vec2_c			default_texcoord_scale;
	
	std::vector<vec3_c>		vertexes;
	std::vector<lwo_vmap_t>		vmaps;
	std::vector<lwo_vmad_t>		vmads;
	std::vector<lwo_ptag_t>		ptags;
	std::vector<index_t>		indexes;

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
				readLayr(size, transform);
				break;
			
			case ID_PNTS:
				readPnts(size, vertexes);
				break;
				
			case ID_BBOX:
				readBbox(size, bbox);
				break;
			
			case ID_POLS:
				readPols(size, indexes);
				break;
			
			case ID_PTAG:
				readPtag(size, ptags);
				break;
				
			case ID_VMAP:
				readVmap(size, vmaps);
				break;

			case ID_VMAD:
				readVmad(size, vmads);
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
	
	for(std::vector<lwo_ptag_t>::const_iterator ir = ptags.begin(); ir != ptags.end(); ++ir)
	{
		const lwo_ptag_t& ptag = *ir;
	
		if(ptag.index_tag < 0 || ptag.index_tag >= _meshes.size())
			ri.Com_Error(ERR_DROP, "r_lwo_model_c::load: tag out of range");
			
		if(ptag.index_polygon < 0 || (ptag.index_polygon * 3) >= indexes.size())
			ri.Com_Error(ERR_DROP, "r_lwo_model_c::load: poly out of range");
			
		r_mesh_c *mesh = _meshes[ptag.index_tag];
		
		mesh->addTriangle(indexes[(ptag.index_polygon*3)+0], indexes[(ptag.index_polygon*3)+1], indexes[(ptag.index_polygon*3)+2], ptag.index_polygon);
	}
	
	default_texcoord.clear();
	default_texcoord_axis[0] = 0;
	default_texcoord_axis[1] = 1;
	for(int i=0; i<3; i++)
	{
		float min = bbox._mins[i];
		float max = bbox._maxs[i];
		float size = max - min;
		
		if(size > default_texcoord[0])
		{
			default_texcoord_axis[1] = default_texcoord_axis[0];
			default_texcoord_axis[0] = i;

			default_texcoord[1] = default_texcoord[0];
			default_texcoord[0] = size;
		}
		else if(size > default_texcoord[1])
		{
			default_texcoord_axis[1] = i;
			default_texcoord[1] = size;
		}
	}
	default_texcoord_scale[0] = 4.0f / default_texcoord[0];
	default_texcoord_scale[1] = 4.0f / default_texcoord[1];
	
	for(std::vector<r_mesh_c*>::const_iterator ir = _meshes.begin(); ir != _meshes.end(); ++ir)
	{
		r_mesh_c *mesh = *ir;
		
		//mesh->indexes = indexes;
				
		if(mesh->indexes.size())
		{
			mesh->fillVertexes(vertexes.size());
			mesh->vertexes = vertexes;
			
			for(uint_t i=0; i<mesh->vertexes.size(); i++)
			{
				mesh->texcoords[i][0] = mesh->vertexes[i][default_texcoord_axis[0]] * default_texcoord_scale[0];
				mesh->texcoords[i][1] = mesh->vertexes[i][default_texcoord_axis[1]] * default_texcoord_scale[1];
			}
			
			for(std::vector<lwo_vmap_t>::const_iterator ir = vmaps.begin(); ir != vmaps.end(); ++ir)
			{
				const lwo_vmap_t& vmap = *ir;
				
				mesh->texcoords[vmap.index_vertex] = vmap.texcoord;
			}
			
			for(std::vector<lwo_vmad_t>::const_iterator ir = vmads.begin(); ir != vmads.end(); ++ir)
			{
				const lwo_vmad_t& vmad = *ir;
				
				if(mesh->hasTriangle(vmad.index_polygon))
				{
					mesh->texcoords[vmad.index_vertex] = vmad.texcoord;
				}
			}
		}
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
		addMesh(new r_mesh_c());
	}
}

void	r_lwo_model_c::readLayr(uint_t nbytes, matrix_c &transform)
{
	ushort_t	flags[2], parent;
	vec3_c		pivot;
	char		name[255];
	
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
}

void	r_lwo_model_c::readPnts(uint_t nbytes, std::vector<vec3_c> &vertexes)
{
	uint_t points_num = nbytes/sizeof(VEC12);
	if(points_num <= 0)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readPnts: mesh has invalid vertices number %i", points_num);
	
	ri.Com_Printf("PNTS [%d] points_num [%d]\n", nbytes, points_num);
	
	vertexes.resize(points_num);
	
	for(uint_t i=0; i<points_num; i++)
	{
		readVEC12(vertexes[i]);
		//ri.Com_DPrintf("\t[%d] [%f,%f,%f]\n", i, mesh->vertexes[i][0], mesh->vertexes[i][1], mesh->vertexes[i][2]);
	}
}

void	r_lwo_model_c::readBbox(uint_t nbytes, cbbox_c &bbox)
{
	readVEC12(bbox._mins);
	readVEC12(bbox._maxs);
	
	ri.Com_DPrintf("BBOX [%d]\n", nbytes);
	ri.Com_DPrintf("\tMIN [%f,%f,%f]\n", bbox._mins[0], bbox._mins[1], bbox._mins[2]);
	ri.Com_DPrintf("\tMAX [%f,%f,%f]\n", bbox._maxs[0], bbox._maxs[1], bbox._maxs[2]);
}

void	r_lwo_model_c::readPols(uint_t nbytes, std::vector<index_t> &indexes)
{
	ushort_t	numvert, flags;
	uint_t		nPols;
	index_t		vx;
	char		id[5];
	uint_t		type;
	
	ri.Com_DPrintf("POLS [%d]", nbytes);
	
	nPols = 0L;

	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
	if(type != ID_FACE)
		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readVmap: bad id '%s'", id);
	ri.Com_DPrintf(" [%s]\n", id);
	
	indexes.clear();

	while(_readcount < (_readcount_old + nbytes))
	{
		numvert = readU2();
		flags      = (0xFC00 & numvert) >> 10;
		numvert    =  0x03FF & numvert;
		
		if(numvert != 3)
			ri.Com_Error(ERR_DROP, "r_lwo_model_c::readPols: numvert %i != 3", numvert);
			
		//ri.Com_DPrintf("\t[%d] NVERT[%d] FLAG[%02x] <", nPols, numvert, flags);
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
				ri.Com_DPrintf("%d>\n", vx);
			else
				ri.Com_DPrintf("%d, ", vx);
			*/
		}
	}
}

int	LWO_SortByTagAndPolyFunc(void const *a, void const *b)
{
	lwo_ptag_t* ptag_a = (lwo_ptag_t*)a;
	lwo_ptag_t* ptag_b = (lwo_ptag_t*)b;
	
	// first sort by Tag
#if 1
	if(ptag_a->index_tag < ptag_b->index_tag)
		return 1;
		
	else if(ptag_a->index_tag > ptag_b->index_tag)
		return -1;
#endif
		
	// second sort by Poly
	if(ptag_a->index_polygon < ptag_b->index_polygon)
		return 1;
		
	else if(ptag_a->index_polygon > ptag_b->index_polygon)
		return -1;
		
	return 0;
}

void	r_lwo_model_c::readPtag(uint_t nbytes, std::vector<lwo_ptag_t> &ptags)
{
	uint_t		nTags;
	ID4		id;
	uint_t		type;
	
	ri.Com_DPrintf("PTAG [%d]", nbytes);
	
	nTags = 0L;
	
	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
//	if(type != ID_SURF)
//		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readPtag: bad id '%s'", id);
	ri.Com_DPrintf(" [%s]\n", id);
	
	ptags.clear();

	while(_readcount < (_readcount_old + nbytes))
	{
		lwo_ptag_t ptag;
	
		ptag.index_polygon = readVX();
		ptag.index_tag = readU2();
		
		//ri.Com_DPrintf("\tPOLY[%d] TAG[%d]\n", vx, tag);
		
		if(type == ID_SURF)
			ptags.push_back(ptag);
		
		nTags++;
	}
	
//	qsort(&ptags[0], ptags.size(), sizeof(lwo_ptag_t), LWO_SortByTagAndPolyFunc);
}

void	r_lwo_model_c::readVmap(uint_t nbytes, std::vector<lwo_vmap_t> &vmaps)
{
	ushort_t	dim;
	char		name[255];
	char		id[5];
	uint_t		type;
		
	ri.Com_DPrintf("VMAP [%d]", nbytes);

	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
//	if(type != ID_TXUV)
//		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readVmap: bad id '%s'", id);
	ri.Com_DPrintf(" [%s]", id);

	dim = readU2();
//	if(dim != 2)
//		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readVmap: texcoords size %i != 2", dim);
	readName(name);
	ri.Com_DPrintf(" DIM [%d] NAME [%s]\n", dim, name);
	
	if(type == ID_TXUV && dim == 2)
	{
		while(_readcount < (_readcount_old + nbytes))
		{
			lwo_vmap_t vmap;
		
			vmap.index_vertex = readVX();
		
			vmap.texcoord[0] = readF4();
			vmap.texcoord[1] = 1.0f - readF4();
				
			vmaps.push_back(vmap);
		}
	}
	else
	{
		_readcount += (nbytes - (_readcount - _readcount_old));
	}
}

void	r_lwo_model_c::readVmad(uint_t nbytes, std::vector<lwo_vmad_t> &vmads)
{
	ushort_t	dim;
	char		name[255];
	char		id[5];
	uint_t		type;
	vec2_c		texcoord;
		
	ri.Com_DPrintf("VMAD [%d]", nbytes);

	readID4(id);
	type = MAKE_ID(id[0], id[1], id[2], id[3]);
//	if(type != ID_TXUV)
//		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readVmad: bad id '%s'", id);
	ri.Com_DPrintf(" [%s]", id);

	dim = readU2();
//	if(dim != 2)
//		ri.Com_Error(ERR_DROP, "r_lwo_model_c::readVmad: texcoords size %i != 2", dim);
	readName(name);
	ri.Com_DPrintf(" DIM [%d] NAME [%s]\n", dim, name);
	
	if(type == ID_TXUV && dim == 2)
	{
		while(_readcount < (_readcount_old + nbytes))
		{
			lwo_vmad_t vmad;
		
			vmad.index_vertex = readVX();
			vmad.index_polygon = readVX();
			
			vmad.texcoord[0] = readF4();
			vmad.texcoord[1] = 1.0f - readF4();
				
			vmads.push_back(vmad);
		}
	}
	else
	{
		_readcount += (nbytes - (_readcount - _readcount_old));
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

