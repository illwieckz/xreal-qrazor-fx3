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
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"


r_bsptree_c::r_bsptree_c(const std::string &name)
{
	_viewcluster_old = _viewcluster = -1;

	byte*		buf = NULL;
//	char*		buf_p;
	std::string	name_full;
	
	if(!name.length())
		ri.Com_Error(ERR_DROP, "r_bsptree_c::ctor: empty name");
		
	_name = name;
	
	name_full = name + ".bsp";
	ri.Com_DPrintf("loading rendering BSP '%s' ...\n", name_full.c_str());
	
	ri.VFS_FLoad(name_full, (void**)&buf);
	if(!buf)
	{
		ri.Com_Error(ERR_DROP, "r_bsptree_c::ctor: '%s' not found", name_full.c_str());
	}
	
	
	bsp_dheader_t *header = (bsp_dheader_t*)buf;
	
	if(!X_strnequal((const char*)buf, BSP_IDENTSTRING, strlen(BSP_IDENTSTRING)))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::ctor: '%s' has wrong ident", name_full.c_str());

	int version = LittleLong(header->version);
	if(version != BSP_VERSION)
		ri.Com_Error(ERR_DROP, "r_bsptree_c:ctor: %s has wrong version number (%i should be %i)", name_full.c_str(), version, BSP_VERSION);

	for(unsigned int i=0; i<(int)sizeof(bsp_dheader_t)/4; i++)
		((int*)header)[i] = LittleLong(((int*)header)[i]);

	loadVisibility(buf, &header->lumps[BSP_LUMP_VISIBILITY]);
	loadVertexes(buf, &header->lumps[BSP_LUMP_VERTEXES]);
	loadIndexes(buf, &header->lumps[BSP_LUMP_INDEXES]);
	loadShaders(buf, &header->lumps[BSP_LUMP_SHADERS]);
	loadPlanes(buf, &header->lumps[BSP_LUMP_PLANES]);
	loadSurfaces(buf, &header->lumps[BSP_LUMP_SURFACES]);
	loadLeafSurfaces(buf, &header->lumps[BSP_LUMP_LEAFSURFACES]);
	loadLeafs(buf, &header->lumps[BSP_LUMP_LEAFS]);
	loadNodes(buf, &header->lumps[BSP_LUMP_NODES]);
	loadModels(buf, &header->lumps[BSP_LUMP_MODELS]);
	
	ri.VFS_FFree(buf);
	
	/*
	createAreas();
	
	name_full = name + ".prt";
	ri.Com_DPrintf("loading rendering BSP '%s' portal information ...\n", name_full.c_str());
	buf = NULL;
	ri.VFS_FLoad(name_full, (void**)&buf);
	if(buf)
	{
		buf_p = (char*)buf;
		loadPortals(&buf_p);
	
		ri.VFS_FFree(buf);
	}
	else
	{
		//ri.Com_Error(ERR_DROP, "r_bsptree_c::ctor: '%s' not found", name_full.c_str());
	}
	*/
	
	if(gl_config.arb_vertex_buffer_object)
	{
		//
		// count vertices and indices
		//
		uint_t	vertexes_num = 0;
		uint_t	indexes_num = 0;
		
		for(std::vector<r_surface_c*>::const_iterator ir = _surfaces.begin(); ir != _surfaces.end(); ir++)
		{	
			r_surface_c *surf = *ir;			
			
			if(surf->getMesh())
			{
				vertexes_num += surf->getMesh()->vertexes.size();
				indexes_num  += surf->getMesh()->indexes.size();
			}
		}
		
		xglGenBuffersARB(1, &_vbo_array_buffer);		RB_CheckForError();
		xglGenBuffersARB(1, &_vbo_element_array_buffer);	RB_CheckForError();
	
		
		uint_t		indexes_size = indexes_num * sizeof(index_t);
		byte*		indexes = (byte*)Com_Alloc(indexes_size);
		uint_t		indexes_ofs = 0;
		
		uint_t		data_size = vertexes_num * (sizeof(vec3_c)*5 + sizeof(vec2_c)*2 + sizeof(vec4_c));
		byte*		data = (byte*)Com_Alloc(data_size);
		uint_t		data_ofs = 0;
		
						
		// create vbo offsets
		for(std::vector<r_surface_c*>::const_iterator ir = _surfaces.begin(); ir != _surfaces.end(); ir++)
		{	
			r_surface_c *surf = *ir;			
			
			if(surf->getMesh())
			{
				surf->getMesh()->vbo_array_buffer = _vbo_array_buffer;
				//surf->getMesh()->vbo_element_array_buffer = _vbo_element_array_buffer;
			
				
				// setup vertices array
				surf->getMesh()->vbo_vertexes_ofs = data_ofs;
				for(std::vector<vec3_c>::const_iterator ir = surf->getMesh()->vertexes.begin(); ir != surf->getMesh()->vertexes.end(); ir++)
				{
					memcpy(data + data_ofs, (vec_t*)*ir, sizeof(vec3_c));
					data_ofs += sizeof(vec3_c);
				}
		
				surf->getMesh()->vbo_texcoords_ofs = data_ofs;
				for(std::vector<vec2_c>::const_iterator ir = surf->getMesh()->texcoords.begin(); ir != surf->getMesh()->texcoords.end(); ir++)
				{
					memcpy(data + data_ofs, (vec_t*)*ir, sizeof(vec2_c));
					data_ofs += sizeof(vec2_c);
				}
				
				surf->getMesh()->vbo_texcoords_lm_ofs = data_ofs;
				for(std::vector<vec2_c>::const_iterator ir = surf->getMesh()->texcoords_lm.begin(); ir != surf->getMesh()->texcoords_lm.end(); ir++)
				{
					memcpy(data + data_ofs, (vec_t*)*ir, sizeof(vec2_c));
					data_ofs += sizeof(vec2_c);
				}
				
				surf->getMesh()->vbo_tangents_ofs = data_ofs;
				for(std::vector<vec3_c>::const_iterator ir = surf->getMesh()->tangents.begin(); ir != surf->getMesh()->tangents.end(); ir++)
				{
					memcpy(data + data_ofs, (vec_t*)*ir, sizeof(vec3_c));
					data_ofs += sizeof(vec3_c);
				}
				
				surf->getMesh()->vbo_binormals_ofs = data_ofs;
				for(std::vector<vec3_c>::const_iterator ir = surf->getMesh()->binormals.begin(); ir != surf->getMesh()->binormals.end(); ir++)
				{
					memcpy(data + data_ofs, (vec_t*)*ir, sizeof(vec3_c));
					data_ofs += sizeof(vec3_c);
				}
				
				surf->getMesh()->vbo_normals_ofs = data_ofs;
				for(std::vector<vec3_c>::const_iterator ir = surf->getMesh()->normals.begin(); ir != surf->getMesh()->normals.end(); ir++)
				{
					memcpy(data + data_ofs, (vec_t*)*ir, sizeof(vec3_c));
					data_ofs += sizeof(vec3_c);
				}
				
				surf->getMesh()->vbo_lights_ofs = data_ofs;
				for(std::vector<vec3_c>::const_iterator ir = surf->getMesh()->lights.begin(); ir != surf->getMesh()->lights.end(); ir++)
				{
					memcpy(data + data_ofs, (vec_t*)*ir, sizeof(vec3_c));
					data_ofs += sizeof(vec3_c);
				}
				
				surf->getMesh()->vbo_colors_ofs = data_ofs;
				for(std::vector<vec4_c>::const_iterator ir = surf->getMesh()->colors.begin(); ir != surf->getMesh()->colors.end(); ir++)
				{
					memcpy(data + data_ofs, (vec_t*)*ir, sizeof(vec4_c));
					data_ofs += sizeof(vec4_c);
				}
								
				// setup indices array
				surf->getMesh()->vbo_indexes_ofs = indexes_ofs;
				for(std::vector<index_t>::const_iterator ir = surf->getMesh()->indexes.begin(); ir != surf->getMesh()->indexes.end(); ir++)
				{
					memcpy(indexes + indexes_ofs, &(*ir), sizeof(index_t));
					indexes_ofs += sizeof(index_t);
				}
			}
		}
		
		
		xglBindBufferARB(GL_ARRAY_BUFFER_ARB, _vbo_array_buffer);	RB_CheckForError();
		xglBufferDataARB(GL_ARRAY_BUFFER_ARB, data_size, data, GL_STATIC_DRAW_ARB);	RB_CheckForError();
		Com_Free(data);
				
		xglBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, _vbo_element_array_buffer);	RB_CheckForError();
		xglBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, indexes_size, indexes, GL_STATIC_DRAW_ARB);	RB_CheckForError();
		Com_Free(indexes);
	}
	else
	{
		_vbo_array_buffer = 0;
		_vbo_element_array_buffer = 0;
	}
}

r_bsptree_c::~r_bsptree_c()
{
	X_purge<std::vector<r_surface_c*> >(_surfaces);
	
	X_purge<std::vector<r_bsptree_leaf_c*> >(_leafs);
	
	X_purge<std::vector<r_bsptree_node_c*> >(_nodes);
	
//	X_purge<std::vector<r_bsptree_area_c*> >(_areas);
	
//	X_purge<std::vector<r_bsptree_portal_c*> >(_portals);
	
	if(_planes)
		delete [] _planes;
		
	if(_vbo_array_buffer)
		xglDeleteBuffersARB(1, &_vbo_array_buffer);
	
	if(_vbo_element_array_buffer)
		xglDeleteBuffersARB(1, &_vbo_element_array_buffer);
}

void	r_bsptree_c::update()
{
	// current viewcluster
	if(!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)/* && !r_mirrorview*/)
	{
		r_bsptree_leaf_c *leaf = NULL;
		
		_viewcluster_old = _viewcluster;
		
		//if(r_portal_view)
		//{
		//	r_viewcluster_old = -1;
		//	leaf = (r_bsptree_leaf_c*)pointInLeaf(r_portal_org);
		//}
		//else
		{
			_viewcluster_old = _viewcluster;
			leaf = (r_bsptree_leaf_c*)pointInLeaf(r_origin);
		}
		
		if(leaf)
			_viewcluster = leaf->cluster;
		else
			_viewcluster = -1;
	}
	
	markLeaves();
	markLights();
	markEntities();
}

void	r_bsptree_c::draw()
{
	if(!r_drawworld->getInteger())
		return;
		
	try
	{
		drawNode_r(_nodes.at(0), FRUSTUM_CLIPALL);
	}
	catch(...)
	{
		ri.Com_Error(ERR_DROP, "r_bsptree_c::draw: exception thrown");
	}
	
	if(r_lighting->getInteger() == 1)
	{
		for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c* light = *ir;
			
			if(!light)
				continue;
			
			if(!light->isVisible())
				continue;
					
			r_lightframecount++;
			
			if(!light->isStatic())
			{
				litNode_r(_nodes[0], light, LITNODE_MODE_DYNAMIC);
			}
			/*
			else
			{
				Tr3B - use instead static light interactions in drawNode_r
				litNode_r(_nodes[0], light, LITNODE_MODE_STATIC);
			}
			*/
		}
	}
	else if(r_lighting->getInteger() == 2)
	{
		for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c* light = *ir;
			
			if(!light)
				continue;
			
			if(!light->isVisible())
				continue;
					
			r_lightframecount++;
			
			if(!light->isStatic())
			{
				litNode_r(_nodes[0], light, LITNODE_MODE_DYNAMIC);
			}
		}
	}
}

r_bsptree_leaf_c*	r_bsptree_c::pointInLeaf(const vec3_c &p)
{
//	if(_nodes.empty())
//		ri.Com_Error(ERR_DROP, "r_bsp_model_c::pointInLeaf: bad model");

	r_tree_elem_c *elem = NULL;
	
	try
	{
		elem = (r_tree_elem_c*)_nodes[0];
	}
	catch(...)
	{
		ri.Com_Error(ERR_DROP, "r_bsp_model_c::exception thrown");
	}
	
	do
	{	
		if(elem->contents != -1)
			return (r_bsptree_leaf_c*)elem;
			
		r_bsptree_node_c* node = (r_bsptree_node_c*)elem;
		
		elem = node->children[node->plane->onSide(p)];
	} while(elem != NULL);
	
	return (r_bsptree_leaf_c*)elem;
}

int	r_bsptree_c::pointInCluster(const vec3_c &p)
{
	r_bsptree_leaf_c *leaf = pointInLeaf(p);
	
	if(leaf)
		return leaf->cluster;
	else
		return -1;
}

bool	r_bsptree_c::pointIsVisible(const vec3_c &p)
{
	//TODO
	return false;
}

void	r_bsptree_c::boxLeafs_r(const aabb_c &aabb, std::vector<r_bsptree_leaf_c*> &leafs, r_tree_elem_c *elem)
{
	if(elem->contents != -1)
	{
		r_bsptree_leaf_c* leaf = (r_bsptree_leaf_c*)elem;
		
		if(leaf->isChecked())
			return;
			
		leaf->setCheckCount();
			
		leafs.push_back(leaf);
		return;
	}
	
	r_bsptree_node_c *node = (r_bsptree_node_c*)elem;
		
	plane_side_e side = node->plane->onSide(aabb, true);
	
	switch(side)
	{
		case SIDE_FRONT:
		{
			boxLeafs_r(aabb, leafs, node->children[SIDE_FRONT]);
			break;
		}
		
		case SIDE_BACK:
		{
			boxLeafs_r(aabb, leafs, node->children[SIDE_BACK]);
			break;
		}
		
		case SIDE_CROSS:
		{
			// go down both
			boxLeafs_r(aabb, leafs, node->children[SIDE_FRONT]);
			boxLeafs_r(aabb, leafs, node->children[SIDE_BACK]);
			break;
		}
		
		default:
			break;
	}
}

void	r_bsptree_c::boxLeafs(const aabb_c &bbox, std::vector<r_bsptree_leaf_c*> &leafs)
{
	if(_nodes.empty())
		return;
		
	r_checkcount++;
		
	boxLeafs_r(bbox, leafs, _nodes[0]);
}

void	r_bsptree_c::precacheLight(r_light_c *light)
{
	r_lightframecount++;	

	litNode_r(_nodes[0], light, LITNODE_MODE_PRECACHE);
	
//	ri.Com_DPrintf("light has %i precached world entity interactions\n", count);
}

void	r_bsptree_c::loadVisibility(const byte *buffer, const bsp_lump_t *l)
{
	ri.Com_DPrintf("loading vis data ...\n");
	
	int pvs_size = l->filelen - BSP_PVS_HEADERSIZE;
	
	ri.Com_DPrintf("PVS data size: %i\n", pvs_size);
	
	if(pvs_size <= 0 || !r_vis->getInteger())
	{
		_pvs.clear();
		return;
	}
	
	_pvs = std::vector<byte>(pvs_size, 0);
	
	for(int i=0; i<pvs_size; i++)
	{
		_pvs[i] = *(buffer + (l->fileofs + BSP_PVS_HEADERSIZE + i));
	}
	
	_pvs_clusters_num  = LittleLong(((int*)(buffer + l->fileofs))[0]);
	_pvs_clusters_size = LittleLong(((int*)(buffer + l->fileofs))[1]);
	
	/*
	for(int i=0; i<_pvs_clusters_num; i++)
	{
		std::vector<boost::dynamic_bitset<byte> >	cluster(toBits(_pvs_cluster_size));
		
		try
		{
			bytesToBits(&_pvs.at(_pvs_clusters_size*i), _pvs_clusters_size);
		}
		catch(...)
		{
			ri.Com_Error(ERR_DROP, "r_bsptree_c::loadVisibility: exception occured");
		}
		
		_pvs_clusters.push_back(;
//		_pvs_clusters = std::vector<boost::dynamic_bitset<byte> >(_pvs_clusters_num);
	*/
	
	
	/*
	_vis = (bsp_dvis_t*)Com_Alloc(l->filelen);
	
	memcpy(_vis, ((byte*)buffer) + l->fileofs, l->filelen);

	_vis->clusters_num = LittleLong(_vis->clusters_num);
	_vis->cluster_size = LittleLong(_vis->cluster_size);
	*/
}

void	r_bsptree_c::loadVertexes(const byte *buffer, const bsp_lump_t *l)
{
	bsp_dvertex_t	*in;
		
	ri.Com_DPrintf("loading vertexes ...\n");
	
	in = (bsp_dvertex_t*)(buffer + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadVertexes: funny lump size in '%s'", _name.c_str());
	int count = l->filelen / sizeof(*in);
	
	_vertexes = std::vector<vec3_c>(count);
	_texcoords = std::vector<vec2_c>(count);
	_texcoords_lm = std::vector<vec2_c>(count);
	_tangents = std::vector<vec3_c>(count);
	_binormals = std::vector<vec3_c>(count);
	_normals = std::vector<vec3_c>(count);
	_lights = std::vector<vec3_c>(count);
	_colors = std::vector<vec4_c>(count, color_white);
	
	float div = X_recip(255.0);
	
	for(int i=0; i<count; i++, in++)
	{
		_vertexes[i][0] = LittleFloat(in->position[0]);
		_vertexes[i][1] = LittleFloat(in->position[1]);
		_vertexes[i][2] = LittleFloat(in->position[2]);
		
		_texcoords[i][0] = LittleFloat(in->texcoord_st[0]);
		_texcoords[i][1] = LittleFloat(in->texcoord_st[1]);
		
		_texcoords_lm[i][0] = LittleFloat(in->texcoord_lm[0]);
		_texcoords_lm[i][1] = LittleFloat(in->texcoord_lm[1]);
		
		_normals[i][0] = LittleFloat(in->normal[0]);
		_normals[i][1] = LittleFloat(in->normal[1]);
		_normals[i][2] = LittleFloat(in->normal[2]);
		_normals[i].normalize();
		
		_lights[i][0] = LittleFloat(in->light[0]);
		_lights[i][1] = LittleFloat(in->light[1]);
		_lights[i][2] = LittleFloat(in->light[2]);
		_lights[i].normalize();
		
		_colors[i][0] = in->color[0] * div;
		_colors[i][1] = in->color[1] * div;
		_colors[i][2] = in->color[2] * div;
	}
}


void	r_bsptree_c::loadModels(const byte *buffer, const bsp_lump_t *l)
{
	bsp_dmodel_t	*in;
	int			i, j, count;

	ri.Com_DPrintf("loading models ...\n");
	
	in = (bsp_dmodel_t*)(buffer + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadModels: funny lump size in '%s'", _name.c_str());
	count = l->filelen / sizeof(*in);	

	std::vector<r_bsptree_model_t>	models(count);

	for(i=0; i<count; i++, in++)
	{
		r_bsptree_model_t& out = models[i];
		
		for(j=0; j<3; j++)
		{	
			// spread the mins / maxs by a pixel
			out.bbox._mins[j] = LittleFloat(in->mins[j]) - 1;
			out.bbox._maxs[j] = LittleFloat(in->maxs[j]) + 1;
		}
		out.radius = out.bbox.radius();
		out.modelsurfaces_num = LittleLong(in->modelsurfaces_num);
		out.modelsurfaces_first = LittleLong(in->modelsurfaces_first);	
	}
	
	for(i=0; i<count; i++)
	{
		r_bsptree_model_t& model = models[i];
		
		r_bsp_model_c* starmod = new r_bsp_model_c(va("*%i", i), i ? true : false);

		for(int j=0; j<model.modelsurfaces_num; j++)
		{
			r_surface_c* surf = _surfaces[model.modelsurfaces_first + j];
			
			surf->_sub = j;
		
			starmod->_surfaces.push_back(surf);
		}
		
		starmod->_aabb = model.bbox;
		
		_models.push_back(starmod);
		r_models.push_back(starmod);
	}
}

void	r_bsptree_c::loadShaders(const byte *buffer, const bsp_lump_t *l)
{
	bsp_dshader_t		*in;
	int			i, count;

	ri.Com_DPrintf("loading shaders ...\n");
	
	in = (bsp_dshader_t*)(buffer + l->fileofs);
	if (l->filelen % sizeof(*in))
		ri.Com_Error (ERR_DROP, "r_bsptree_c::loadShaders: funny lump size in %s", _name.c_str());
	count = l->filelen / sizeof(*in);
		
	for(i=0; i<count; i++, in++)
	{
		_shaders.push_back(new r_model_shader_c(in->shader, R_RegisterShader(in->shader), LittleLong(in->flags), LittleLong(in->contents)));
	}
}

void	r_bsptree_c::loadSurfaces(const byte *buffer, const bsp_lump_t *l)
{
	bsp_dsurface_t	*in;
	r_surface_c 	*out;
	
	int			i, j, count;
	int			shadernum;
	
	ri.Com_Printf("loading surfaces ...\n");

	in = (bsp_dsurface_t*)(buffer + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadSurfaces: funny lump size in %s", _name.c_str());
	count = l->filelen / sizeof(*in);
	
	_surfaces = std::vector<r_surface_c*>(count);
	
	for(i=0; i<count; i++, in++)
	{
		_surfaces[i] = out = new r_surface_c;
	
		out->_mesh = NULL;
		
		out->_facetype = (bsp_surface_type_t)LittleLong(in->face_type);
		
		out->_lightmap = LittleLong(in->lm_num);
		
		switch(out->_facetype)
		{
			case BSPST_PLANAR:
			{
				out->_mesh = createMesh(in);
			
				//vec3_c	origin;
				vec3_c	normal;
			
				for(j=0; j<3; j++)
				{
					//origin[j] = LittleFloat(in->origin[j]);
					normal[j] = LittleFloat(in->normal[j]);
				}
				
				if(!normal.length() || out->_mesh->vertexes.empty())
				{
					out->_facetype = BSPST_PLANAR_NOCULL;
				}
				else
				{
					out->_plane.set(normal, normal.dotProduct(out->_mesh->vertexes[0]));
					/*
					out->_plane.fromThreePointForm(	out->_mesh->vertexes[0],
									out->_mesh->vertexes[1],
									out->_mesh->vertexes[2]	);
					*/
				}
				break;
			}
			
			case BSPST_MESH:
			{
				out->_mesh = createMesh(in);
				break;
			}
			
			case BSPST_BEZIER:
			{
				out->_mesh = createBezierMesh(in);
				break;
			}
			
			case BSPST_BAD:
			case BSPST_FLARE:
			default:
				ri.Com_Error(ERR_DROP, "r_bsptree_c::loadSurfaces: bad surface type %i", out->_facetype);
		}
		
		// load shader																			
		shadernum = LittleLong(in->shader_num);
		if(shadernum < 0 || shadernum >= (int)_shaders.size())
			ri.Com_Error(ERR_DROP, "r_bsptree_c::loadSurfaces: shadernum out of range %i\n", shadernum);
		
		out->_shaderref = _shaders[shadernum];
		
		// create bounding box on the fly
		out->_mesh->createBBoxFromVertexes();
			
		// setup Tangent Spaces
		out->_mesh->calcTangentSpaces();
	}
}


void	r_bsptree_c::setParent(r_tree_elem_c *elem, r_tree_elem_c *parent)
{
	elem->parent = parent;
	
	if(elem->contents != -1)	// is a leaf
		return;
		
	r_bsptree_node_c *node = (r_bsptree_node_c*)elem;
	
	setParent(node->children[0], elem);
	setParent(node->children[1], elem);
}

void	r_bsptree_c::loadNodes(const byte *buffer, const bsp_lump_t *l)
{
	int			j, count, p;
	bsp_dnode_t		*in;
	r_bsptree_node_c 	*out;

	ri.Com_DPrintf("loading nodes ...\n");
	
	in = (bsp_dnode_t*)(buffer + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadNodes: funny lump size in %s",_name.c_str());
	count = l->filelen / sizeof(*in);

	_nodes = std::vector<r_bsptree_node_c*>(count);
	
	// create nodes
	for(uint_t i=0; i<_nodes.size(); i++, in++)
	{
		_nodes[i] = out = new r_bsptree_node_c;
		
		out->parent = NULL;
		
		for(j=0; j<3; j++)
		{
			out->_aabb._mins[j] = (float)LittleLong(in->mins[j]);
			out->_aabb._maxs[j] = (float)LittleLong(in->maxs[j]);
		}
	
		out->plane = _planes + LittleLong(in->plane_num);
		out->contents = -1;	// differentiate from leafs
		
		out->children[SIDE_FRONT] = NULL;
		out->children[SIDE_BACK] = NULL;
	}
	
	// set childs
	in = (bsp_dnode_t*)(buffer + l->fileofs);	// reset in pointer
	
	for(uint_t i=0; i<_nodes.size(); i++, in++)
	{
		out = _nodes[i];
		
		for(j=0; j<2; j++)
		{
			p = LittleLong(in->children[j]);
			
			if(p >= 0)
			{
				if(p >= (int)_nodes.size())
					ri.Com_Error(ERR_FATAL, "r_bsptree_c::loadNodes: bad nodenum %i", p);
			
				out->children[j] = _nodes[p];
				
				if(!out->children[j])
					ri.Com_Error(ERR_FATAL, "r_bsptree_c::loadNodes: bad node pointer");
				
			}
			else
			{
				int leafnum = -1 - p;
				
				if(leafnum <= 0 || leafnum >= (int)_leafs.size())
					ri.Com_Error(ERR_FATAL, "r_bsptree_c::loadNodes: bad leafnum %i", leafnum);
			
				out->children[j] = (r_bsptree_node_c*)_leafs[leafnum];
				
				if(!out->children[j])
					ri.Com_Error(ERR_FATAL, "r_bsptree_c::loadNodes: bad leaf pointer");
			}
		}
	}
		
	
	// set parents
	setParent(_nodes[0], NULL);	// sets nodes and leafs
	
	// check parents
	/*
	for(std::vector<r_bsptree_leaf_c*>::iterator ir = _leafs.begin(); ir != _leafs.end(); ++ir)
	{
		r_bsptree_leaf_c *leaf = *ir;
		
		if(!leaf->parent)
			ri.Com_Error(ERR_DROP, "r_bsptree_c::loadNodes: leaf without parent");
	}
	*/
}

void	r_bsptree_c::loadLeafs(const byte *buffer, const bsp_lump_t *l)
{
	bsp_dleaf_t 		*in;
	r_bsptree_leaf_c 	*out;
	int			j, count;
	
	ri.Com_DPrintf("loading leafs ...\n");
	
	in = (bsp_dleaf_t*)(buffer + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadLeafs: funny lump size in %s",_name.c_str());
	count = l->filelen / sizeof(*in);

	_leafs = std::vector<r_bsptree_leaf_c*>(count);
	
	for(uint_t i=0; i<_leafs.size(); i++, in++)
	{
		_leafs[i] = out = new r_bsptree_leaf_c;
		
		out->parent = NULL;
		out->surfaces.clear();
		out->lights.clear();
	
		for(j=0; j<3; j++)
		{
			out->_aabb._mins[j] = (float)LittleLong(in->mins[j]);
			out->_aabb._maxs[j] = (float)LittleLong(in->maxs[j]);
		}

		out->contents = 0;
		
		out->cluster = LittleLong(in->cluster);
		
		if(_pvs.size())
		{
			if(out->cluster >= _pvs_clusters_num)
				ri.Com_Error(ERR_DROP, "r_bsptree_c::loadLeafs: leaf cluster > number of clusters");
		}
		
		out->area = LittleLong(in->area);
		//if(out->area < 0)
		//	ri.Com_Error(ERR_DROP, "r_bsptree_c::loadLeafs: leaf area < 0");
	
		int first = LittleLong(in->leafsurfaces_first);
		if(first < 0 || first >= (int)_surfaces_leaf.size())
		{
			ri.Com_DPrintf("r_bsptree_c::loadLeafs: bad firstleafface %i\n", first);
			continue;
		}
		
		try
		{
			for(j=0; j<LittleLong(in->leafsurfaces_num); j++)
			{
				out->surfaces.push_back(_surfaces_leaf.at(first + j));
			}
		}
		catch(...)
		{
			ri.Com_Error(ERR_DROP, "r_bsptree_c::loadLeafs: exception occured");
		}
	}
}

void	r_bsptree_c::loadLeafSurfaces(const byte *buffer, const bsp_lump_t *l)
{	
	ri.Com_DPrintf("loading leaf surfaces ...\n");
	
	bsp_dleafsurface_t *in = (int*)(buffer + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadLeafSurfaces: funny lump size in %s", _name.c_str());
	int count = l->filelen / sizeof(*in);

	_surfaces_leaf = std::vector<r_surface_c*>(count);
	
	for(int i=0; i<count; i++)
	{
		int num = LittleLong(in[i]);
		
		if(num < 0 ||  num >= (int)_surfaces.size())
			ri.Com_Error(ERR_DROP, "r_bsptree_c::loadLeafSurfaces: bad surface number %i", num);
		
		_surfaces_leaf[i] = _surfaces[num];
	}
}


void	r_bsptree_c::loadIndexes(const byte *buffer, const bsp_lump_t *l)
{	
	int		i, count;
	bsp_dindex_t	*in;
	
	ri.Com_DPrintf("loading indexes ...\n");
	
	in = (int*)(buffer + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadIndexes: funny lump size in %s", _name.c_str());
	count = l->filelen / sizeof(*in);
	
	if(count < 1 || count >= MAX_BSP_INDEXES)
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadIndexes: bad surfindexes count in %s: %i", _name.c_str(), count);

	_indexes = std::vector<index_t>(count);
	
	for(i=0; i<count; i++)
		_indexes[i] = LittleLong(in[i]);
}

void	r_bsptree_c::loadPlanes(const byte *buffer, const bsp_lump_t *l)
{
	int			i, j;

	cplane_c		*out;
	bsp_dplane_t 	*in;
	int			count;
	
	vec3_c		normal;
	float		dist;
	
	
	ri.Com_DPrintf("loading planes ...\n");
	
	in = (bsp_dplane_t*)(buffer + l->fileofs);
	if(l->filelen % sizeof(*in))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadPlanes: funny lump size in %s", _name.c_str());
	count = l->filelen / sizeof(*in);
	out = new cplane_c[count];	
	
	_planes_num = count;
	_planes = out;

	for(i=0; i<count; i++, in++, out++)
	{
		for(j=0; j<3; j++)
			normal[j] = LittleFloat(in->normal[j]);
		
		dist = LittleFloat(in->dist);
			
		out->set(normal, dist);
	}
}

/*
void	r_bsptree_c::loadPortals(char **buf_p)
{
	char *token = Com_Parse(buf_p, true);
	
	if(!X_strnequal(token, "PRT1", 4))
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadPortals: wrong ident");
		
	Com_ParseInt(buf_p, true);	// nodes_num
	int portals_num = Com_ParseInt(buf_p, true);
	Com_ParseInt(buf_p, true);	// faces_num
	
	if(!portals_num)
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadPortals: bad portals number %i", portals_num);
	
	for(int i=0; i<portals_num; i++)
	{
		loadPortal(buf_p);
	}
	
	int areaportals_num = 0;
	for(std::vector<r_bsptree_portal_c*>::const_iterator ir = _portals.begin(); ir != _portals.end(); ++ir)
	{
		if((*ir)->areaportal)
			areaportals_num++;
	}
	
	ri.Com_Printf("loaded %i portals ...\n", portals_num);
	ri.Com_Printf("loaded %i area portals ...\n", areaportals_num);
}

void	r_bsptree_c::loadPortal(char **buf_p)
{
	char *token = NULL;
	
	int points_num = Com_ParseInt(buf_p, true);
	if(points_num < 3)
		ri.Com_Error(ERR_DROP, "r_bsptree_c::loadPortal: bad points number %i", points_num);
	
	r_bsptree_portal_c* portal = new r_bsptree_portal_c();
	
	int leaves[2];
	leaves[0] = Com_ParseInt(buf_p, false);
	leaves[1] = Com_ParseInt(buf_p, false);
	
	for(int i=0; i<2; i++)
	{
		if(leaves[i] < 0 || leaves[i] >= (int)_leafs.size())
			ri.Com_Error(ERR_DROP, "r_bsptree_c::loadPortal: portal has bad leaf number %i", leaves[i]);
			
		portal->leafs[i] = _leafs[leaves[i]];
	}
	
	if(portal->leafs[0]->area != portal->leafs[1]->area)
	{
		if(portal->leafs[0]->area == -1 || portal->leafs[1]->area == -1)
			portal->areaportal = false;
		else
			portal->areaportal = true;
	}
	
	Com_ParseInt(buf_p, false);	// skip antiportal
	
	portal->points = std::vector<vec3_c>(points_num);
	portal->points_inner = std::vector<vec3_c>(points_num);
	
	portal->bbox.clear();
	
	for(int i=0; i<points_num; i++)
	{
		token = Com_Parse(buf_p, false);
		if(token[0] != '(')
			ri.Com_Error(ERR_DROP, "r_bsptree_c::loadPortal: found '%s' instead of '('", token);
	
		portal->points[i][0] = atof(token + 1);
		portal->points[i][1] = Com_ParseFloat(buf_p, false);
		portal->points[i][2] = Com_ParseFloat(buf_p, false);
		
		portal->center += portal->points[i];
		portal->bbox.addPoint(portal->points[i]);
		
		//ri.Com_Printf("%s ", ap->points[i].toString());
		
		token = Com_Parse(buf_p, true);
		if(!X_strequal(token, ")"))
			ri.Com_Error(ERR_DROP, "r_bsptree_c::loadPortal: found '%s' instead of ')'", token);
	}
	//ri.Com_Printf("\n");
	
	portal->center *= (1.0/points_num);
	
	for(int i=0; i<points_num; i++)
	{
		portal->points_inner[i] = (portal->center * 0.01) + (portal->points[i] * 0.99);
	}
	
	_portals.push_back(portal);
}

void	r_bsptree_c::createAreas()
{
	// count areas from leaves
	int area_max = 0;
	
	for(std::vector<r_bsptree_leaf_c*>::const_iterator ir = _leafs.begin(); ir != _leafs.end(); ++ir)
	{
		r_bsptree_leaf_c* leaf = *ir;
	
		if(leaf->area == -1)
			continue;
	
		if(leaf->surfaces.empty())
			continue;
	
		if(leaf->area > area_max)
			area_max = leaf->area;
	}
	
	// create empty areas
	_areas = std::vector<r_bsptree_area_c*>(area_max + 1);
	
	for(int i=0; i<(int)_areas.size(); i++)
	{
		_areas[i] = new r_bsptree_area_c();
	}
	
	// add area surfaces
	for(std::vector<r_bsptree_leaf_c*>::const_iterator ir = _leafs.begin(); ir != _leafs.end(); ++ir)
	{
		r_bsptree_leaf_c* leaf = *ir;
		
		if(leaf->area == -1)
			continue;
			
		r_bsptree_area_c* area = _areas[leaf->area];
		
		for(std::vector<r_surface_c*>::iterator ir2 = leaf->surfaces.begin(); ir2 != leaf->surfaces.end(); ++ir2)
		{
			r_surface_c *surf = *ir2;
			
			// check if surface alreay added
			std::vector<r_surface_c*>::iterator ir3 = std::find(area->surfaces.begin(), area->surfaces.end(), static_cast<r_surface_c*>(surf));
			if(ir3 != area->surfaces.end())
				continue;
			
			area->surfaces.push_back(surf);
		}
	}
	
	for(int i=0; i<(int)_areas.size(); i++)
	{
		if(_areas[i]->surfaces.empty())
			ri.Com_DPrintf("area %i is empty\n", i);
			
		_areas[i] = new r_bsptree_area_c();
	}
	
	ri.Com_DPrintf("created %i areas\n", _areas.size());
}
*/


r_mesh_c*	r_bsptree_c::createMesh(const bsp_dsurface_t *in)
{
	r_mesh_c* mesh = new r_mesh_c();
			
	int vertexes_first = LittleLong(in->vertexes_first);
	int vertexes_num = LittleLong(in->vertexes_num);
				
	mesh->fillVertexes(vertexes_num, true, true);
			
	for(int j=0; j<vertexes_num; j++)
	{
		mesh->vertexes[j] = _vertexes[vertexes_first + j];
		mesh->texcoords[j] = _texcoords[vertexes_first + j];
		mesh->texcoords_lm[j] = _texcoords_lm[vertexes_first + j];
		mesh->normals[j] = _normals[vertexes_first + j];
		mesh->lights[j] = _lights[vertexes_first + j];
		mesh->colors[j] = _colors[vertexes_first + j];
	}
				
	int indexes_num = LittleLong(in->indexes_num);
	int indexes_first = LittleLong(in->indexes_first);
				
	mesh->indexes = std::vector<index_t>(indexes_num);
	for(int j=0; j<indexes_num; j++)
	{
		mesh->indexes[j] = _indexes[indexes_first + j];
	}
	
	return mesh;
}

r_mesh_c*	r_bsptree_c::createBezierMesh(const bsp_dsurface_t *in)
{
	int		step[2];
	int 		size[2];
	int		flat[2];
	int		mesh_cp[2];
	int		i, p, u, v;
	
	int		vertexes_first;
	int		vertexes_num;
	
	r_mesh_c*	mesh;
	
	int		subdivlevel;
	
	mesh_cp[0] = LittleLong(in->mesh_cp[0]);
	mesh_cp[1] = LittleLong(in->mesh_cp[1]);
	
	if(!mesh_cp[0] || !mesh_cp[1])
		return NULL;
	
	subdivlevel = r_subdivisions->getInteger();
	if(subdivlevel < 1)
		subdivlevel = 1;
	
	vertexes_first = LittleLong(in->vertexes_first);
	vertexes_num = LittleLong(in->vertexes_num);
	
	std::vector<vec4_c>	vertexes(vertexes_num);
	std::vector<vec4_c>	normals(vertexes_num);
	std::vector<vec4_c>	texcoords(vertexes_num);
	std::vector<vec4_c>	texcoords_lm(vertexes_num);
	std::vector<vec4_c>	lights(vertexes_num);
	std::vector<vec4_c>	colors(vertexes_num);
	
	for(i=0; i<vertexes_num; i++)
	{
		_vertexes[vertexes_first +i].copyTo(vertexes[i]);
		_normals[vertexes_first +i].copyTo(normals[i]);
		_texcoords[vertexes_first +i].copyTo(texcoords[i]);
		_texcoords_lm[vertexes_first +i].copyTo(texcoords_lm[i]);
		_lights[vertexes_first +i].copyTo(lights[i]);
		_colors[vertexes_first +i].copyTo(colors[i]);
	}
	
	// find degree of subdivision
	Curve_GetFlatness(subdivlevel, &(vertexes[0]), mesh_cp, flat);
	
	// allocate space for mesh
	step[0] = (1 << flat[0]);		//step u
	step[1] = (1 << flat[1]);		//step v
	
	size[0] = (mesh_cp[0] / 2) * step[0] + 1;
	size[1] = (mesh_cp[1] / 2) * step[1] + 1;
	vertexes_num = size[0] * size[1];

	mesh = new r_mesh_c();
	
	mesh->fillVertexes(vertexes_num, true, true);
	
	// allocate and fill index table
	int indexes_num = (size[0]-1) * (size[1]-1) * 6;
	mesh->indexes = std::vector<index_t>(indexes_num);
	
	for(v=0, i=0; v<size[1]-1; v++)
	{
		for(u=0; u<size[0]-1; u++, i+=6)
		{	
			mesh->indexes[i+0] = p = v * size[0] + u;
			mesh->indexes[i+1] = p + size[0];
			mesh->indexes[i+2] = p + 1;
			mesh->indexes[i+3] = p + 1;
			mesh->indexes[i+4] = p + size[0];
			mesh->indexes[i+5] = p + size[0] + 1;
		}
	}
	
	std::vector<vec4_c>	vertexes2(vertexes_num);
	std::vector<vec4_c>	normals2(vertexes_num);
	std::vector<vec4_c>	texcoords2(vertexes_num);
	std::vector<vec4_c>	texcoords2_lm(vertexes_num);
	std::vector<vec4_c>	lights2(vertexes_num);
	std::vector<vec4_c>	colors2(vertexes_num);
		
	// fill in
	Curve_EvalQuadricBezierPatch(&(vertexes[0]), mesh_cp, step, &(vertexes2[0]));
	Curve_EvalQuadricBezierPatch(&(texcoords[0]), mesh_cp, step, &(texcoords2[0]));
	Curve_EvalQuadricBezierPatch(&(texcoords_lm[0]), mesh_cp, step, &(texcoords2_lm[0]));
	Curve_EvalQuadricBezierPatch(&(normals[0]), mesh_cp, step, &(normals2[0]));
	Curve_EvalQuadricBezierPatch(&(lights[0]), mesh_cp, step, &(lights2[0]));
	Curve_EvalQuadricBezierPatch(&(colors[0]), mesh_cp, step, &(colors2[0]));
	
	for(i=0; i<(int)mesh->vertexes.size(); i++)
	{
		mesh->vertexes[i] = vertexes2[i];
		mesh->texcoords[i] = texcoords2[i];
		mesh->texcoords_lm[i] = texcoords2_lm[i];
		mesh->normals[i] = normals2[i];
		mesh->lights[i] = lights2[i];
		mesh->colors[i] = colors2[i];
	}
	
	return mesh;
}



void	r_bsptree_c::drawNode_r(r_tree_elem_c *elem, int clipflags)
{
	if(!elem->isVisFramed())
		return;
		
	if(clipflags > 0)
	{
		for(int i=0; i<FRUSTUM_PLANES; i++)
		{
			if(!(clipflags & (1<<i)))
				continue;	// don't need to clip against it

			plane_side_e clipped = r_frustum[i].onSide(elem->_aabb, true);
			
			if(clipped == SIDE_BACK)
				return;
				
			else if(clipped == SIDE_FRONT)
				clipflags &= ~(1<<i);	// node is entirely on screen
		}
	}
	
	// if a leaf node, draw stuff
	if(elem->contents != -1)
	{
		r_bsptree_leaf_c *leaf = (r_bsptree_leaf_c*)elem;
		
		//if(leaf->area <= 0)
		//	return;
		
		if(leaf->isFramed())	// already added surface
			return;
			
		leaf->setFrameCount();
		
		if(leaf->surfaces.empty())
			return;

		// check for door connected areas
		if((leaf->area >= 0) && (leaf->area < (int)r_newrefdef.areabits.size()) && !r_newrefdef.areabits[leaf->area])
			return;		// not visible
		
		for(std::vector<r_surface_c*>::const_iterator ir = leaf->surfaces.begin(); ir != leaf->surfaces.end(); ++ir)
		{
			r_surface_c* surf = *ir;
			
			if(surf == NULL)
				continue;
		
			if(surf->isFramed())	// already added surface
				continue;
			
			if(!surf->getMesh())
			{
				ri.Com_DPrintf("r_bsptree_c::drawNode_r: surface of type %i has no mesh\n", surf->getFaceType());
				continue;
			}
			
			if(!surf->getShader())
			{
				ri.Com_DPrintf("r_bsptree_c::drawNode_r: surface has no shader\n");
				continue;
			}
			
			if(!r_showinvisible->getInteger() && surf->getShaderRef()->hasFlags(X_SURF_NODRAW))
				continue;
				
			if(r_envmap && surf->getShader()->hasFlags(SHADER_NOENVMAP))
				continue;
				
			//if(!r_showareaportals->getInteger() && surf->getShader()->hasFlags(SHADER_AREAPORTAL))
			//	continue;
				
			switch(surf->getFaceType())
			{
				case BSPST_PLANAR:
				{
					if(!surf->getShader()->hasFlags(SHADER_TWOSIDED))
					{
						if(surf->getPlane().distance(r_origin) <= -0.01)			
						//if(surf->getPlane().onSide(r_origin) == SIDE_BACK)
							continue;
					}
					break;
				}
			
				case BSPST_BEZIER:
				case BSPST_MESH:
				{
					if(r_frustum.cull(surf->getMesh()->bbox, clipflags))
						continue;
				}
		
				default:
					break;
			}
			
			surf->setFrameCount();
			
			#if DEBUG
			try
			{
			#endif
				RB_AddCommand(&r_world_entity, _models.at(0), surf->getMesh(), surf->getShader(), NULL, NULL, surf->getLightMapNum(), X_infinity);
			#if DEBUG
			}
			catch(...)
			{
				ri.Com_DPrintf("r_bsptree_c::drawNode_r: exception thrown while creating command\n");
			}
			#endif
			
			if(r_lighting->getInteger() == 1)
			{
				const r_entity_sub_c& sub = r_world_entity.getSubEntity(surf->getSubEntityNum());
					
				const std::vector<r_interaction_c*>& interactions = sub.getInteractions();
					
				for(std::vector<r_interaction_c*>::const_iterator ir = interactions.begin(); ir != interactions.end(); ++ir)
				{
					const r_interaction_c* ia = *ir;
					
					if(ia->getIndexes().empty())
						continue;
					
					#if DEBUG
					try
					{
					#endif
						RB_AddCommand(&r_world_entity, _models.at(0), surf->getMesh(), surf->getShader(), ia->getLight(), (std::vector<index_t>*)&ia->getIndexes(), -1, 0, ia->getAttenuation());
					
					#if DEBUG
					}
					catch(...)
					{
						ri.Com_DPrintf("r_bsptree_c::drawNode_r: exception thrown while creating light command\n");
					}
					#endif
				}
			}
		}
		
		c_leafs++;
	}
	else
	{
		r_bsptree_node_c	*node = (r_bsptree_node_c*)elem;
		
		drawNode_r(node->children[0], clipflags);
		drawNode_r(node->children[1], clipflags);
	}
}



int	r_bsptree_c::litNode_r(r_tree_elem_c *elem, r_light_c *light, r_litnode_mode_e mode, int count)
{
	if(mode == LITNODE_MODE_DYNAMIC && !elem->isVisFramed())
		return count;
		
	if(elem->contents != -1)
	{
		// mark the polygons
		r_bsptree_leaf_c *leaf = (r_bsptree_leaf_c*)elem;
		
		if(leaf->isLightFramed())
			return count;
			
		leaf->setLightFrameCount();
		
		// check for door connected areas
		if(mode == LITNODE_MODE_DYNAMIC)
		{
			if((leaf->area >= 0) && (leaf->area < (int)r_newrefdef.areabits.size()) && !r_newrefdef.areabits[leaf->area])
				return count;		// not visible
		}
		
		//if(!leaf->_aabb.intersect(light->getShared().radius_aabb))
		//	return count;
					
		for(std::vector<r_surface_c*>::const_iterator ir = leaf->surfaces.begin(); ir != leaf->surfaces.end(); ++ir)
		{
			r_surface_c *surf = *ir;

			if(!surf->getMesh())
				continue;
				
			if(!surf->getShader() || !surf->getShader()->stage_diffusemap)
				continue;	
				
			switch(mode)
			{
				case LITNODE_MODE_PRECACHE:
				{
					if(surf->isLightFramed())
						continue;
						
					surf->setLightFrameCount();
				
					if(!surf->getMesh()->bbox.intersect(light->getShared().radius_aabb))
						continue;
						
					r_interaction_c* ia = light->createInteraction(&r_world_entity, surf->getMesh());
					
					if(!ia)
						continue;
						
					r_world_entity.addInteractionToSubEntity(surf->getSubEntityNum(), ia);
					count++;
					break;
				}
				
				case LITNODE_MODE_STATIC:
				{
					if(surf->isLightFramed())
						continue;
						
					surf->setLightFrameCount();
				
					if(!surf->getMesh()->bbox.intersect(light->getShared().radius_aabb))
						continue;
				
					const r_entity_sub_c& sub = r_world_entity.getSubEntity(surf->getSubEntityNum());
					
					const std::vector<r_interaction_c*>& interactions = sub.getInteractions();
					
					for(std::vector<r_interaction_c*>::const_iterator ir = interactions.begin(); ir != interactions.end(); ++ir)
					{
						const r_interaction_c* ia = *ir;
						
						if(light != ia->getLight())
							continue;
							
						if(ia->getIndexes().empty())
							continue;
					
						RB_AddCommand(&r_world_entity, _models[0], surf->getMesh(), surf->getShader(), light, (std::vector<index_t>*)&ia->getIndexes(), -1, 0, ia->getAttenuation());
					}
					break;
				}
				
				case LITNODE_MODE_DYNAMIC:
				{
					if(!surf->isFramed())
						continue;
						
					if(surf->isLightFramed())
						continue;
						
					surf->setLightFrameCount();
				
					if(!surf->getMesh()->bbox.intersect(light->getShared().radius_aabb))
						continue;
					
					#if DEBUG
					try
					{
					#endif
						RB_AddCommand(&r_world_entity, _models.at(0), surf->getMesh(), surf->getShader(), light, NULL, -1, 0);
					#if DEBUG
					}
					catch(...)
					{
						ri.Com_DPrintf("r_bsptree_c::litNode_r: exception thrown\n");
					}
					#endif
					break;
				}
			}
		}
	}
	else
	{
		r_bsptree_node_c *node = (r_bsptree_node_c*)elem;
		
		plane_side_e side = node->plane->onSide(light->getShared().radius_aabb, true);
		
		if(side == SIDE_FRONT)
		{
			count = litNode_r(node->children[SIDE_FRONT], light, mode, count);
		}
		else if(side == SIDE_BACK)
		{
			count = litNode_r(node->children[SIDE_BACK], light, mode, count);
		}
		else
		{	// go down both
			count = litNode_r(node->children[SIDE_FRONT], light, mode, count);
			count = litNode_r(node->children[SIDE_BACK], light, mode, count);
		}
	}
	
	return count;
}


byte*	r_bsptree_c::clusterPVS(int cluster)
{
	if(cluster < 0 || cluster >= _pvs_clusters_num || _pvs.empty())
		return NULL;
		
	//if(cluster < 0 || cluster >= (_pvs_clusters_num))
	//	return NULL;
	
	byte *data = NULL;
	
	#if DEBUG
	try
	{
	#endif
		data =  &(_pvs.at(cluster * _pvs_clusters_size));
	#if DEBUG
	}
	catch(...)
	{
		ri.Com_Error(ERR_DROP, "r_bsptree_c::clusterPVS: exception occured");
	}
	#endif
	
	return data;
}




/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
void 	r_bsptree_c::markLeaves()
{
	if(_viewcluster_old == _viewcluster && r_vis->getInteger() && _viewcluster != -1)
		return;

	// development aid to let you run around and see exactly where
	// the PVS ends
	if(r_lockpvs->getInteger())
		return;

	r_visframecount++;
	
	byte *vis = clusterPVS(_viewcluster);
	
	if(!r_vis->getInteger() || _viewcluster == -1 || !vis || _pvs.empty())
	{
		// mark everything
		for(std::vector<r_bsptree_node_c*>::iterator ir = _nodes.begin(); ir != _nodes.end(); ++ir)
		{
			r_bsptree_node_c *node = *ir;
			node->setVisFrameCount();
		}
		
		for(std::vector<r_bsptree_leaf_c*>::iterator ir = _leafs.begin(); ir != _leafs.end(); ++ir)
		{
			r_bsptree_leaf_c *leaf = *ir;
		
			if(leaf->surfaces.empty())
				continue;
				
			//if(leaf->area <= 0)
			//	continue;
		
			leaf->setVisFrameCount();
		}
		
		/*
		for(std::vector<r_bsptree_area_c*>::iterator ir = _areas.begin(); ir != _areas.end(); ++ir)
		{
			r_bsptree_area_c *area = *ir;
			area->visframe = _visframecount;
		}
		*/
	}
	else
	{
		#if DEBUG
		try
		{
		#endif
			// mark only leafs that are in the PVS
			for(std::vector<r_bsptree_leaf_c*>::iterator ir = _leafs.begin(); ir != _leafs.end(); ++ir)
			{
				r_bsptree_leaf_c *leaf = *ir;
			
				if(leaf->cluster == -1)
					continue;
					
				if(leaf->area < 0)
					continue;
		
				if(vis[leaf->cluster >> 3] & (1 << (leaf->cluster & 7)))
				{
					r_tree_elem_c *elem = (r_tree_elem_c*)leaf;
			
					do
					{
						if(elem->isVisFramed())
							break;
				
						elem->setVisFrameCount();
						elem = elem->parent;
					}while(elem != NULL);
					
					
					//if(leaf->area != -1)
					//	_areas.at(leaf->area)->visframe = _visframecount;
				}
				
			}
		#if DEBUG
		}
		catch(...)
		{
			ri.Com_Error(ERR_DROP, "r_bsptree_c::markLeaves: exception occured");
		}
		#endif
	}
}

void 	r_bsptree_c::markLights()
{
	if(_viewcluster_old == _viewcluster && r_vis->getInteger() && _viewcluster != -1)
		return;

	if(r_lockpvs->getInteger())
		return;
	
	byte *vis = clusterPVS(_viewcluster);
	
	if(!r_vis->getInteger() || _viewcluster == -1 || !vis || _pvs.empty())
	{
		for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c* light = *ir;
			
			if(!light)
				continue;
			
			light->setVisFrameCount();
		}
	}
	else
	{
		for(std::vector<r_light_c*>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c* light = *ir;
			
			if(!light)
				continue;
			
			if(!light->isStatic())
			{
				//if(light->getCluster() < 0)
				//	continue;
				
				light->setVisFrameCount();
				continue;
			}
			
			/*
			if(r_newrefdef.areabits)
				{
					TODO cull by unset areas
				
					if(!r_newrefdef.areabits[light->getArea()])
						continue;
					
					const std::vector<int>& areas = light->getAreas();
					
					for(std::vector<int>::const_iterator ir = areas.begin(); ir != areas.end(); ++ir)
					{
						//_areas[*ir]->lights.push_back(light);
					}
				}
				*/
				
			const std::vector<r_bsptree_leaf_c*>& leafs = light->getLeafs();
			
			for(std::vector<r_bsptree_leaf_c*>::const_iterator ir = leafs.begin(); ir != leafs.end(); ++ir)
			{
				r_bsptree_leaf_c* leaf = *ir;
				
				if(leaf->isVisFramed())
				{
					light->setVisFrameCount();
					break;
				}
			}
		}
	}
}

void 	r_bsptree_c::markEntities()
{
	r_world_entity.setFrameCount();
	r_world_entity.setVisFrameCount();
	c_entities++;

//	if(_viewcluster_old == _viewcluster && r_vis->getInteger() && _viewcluster != -1)
//		return;

	if(r_lockpvs->getInteger())
		return;
	
	byte *vis = clusterPVS(_viewcluster);
	
	if(!r_vis->getInteger() || _viewcluster == -1 || !vis || _pvs.empty())
	{
		for(std::vector<r_entity_c*>::iterator ir = r_entities.begin(); ir != r_entities.end(); ++ir)
		{
			r_entity_c* ent = *ir;
			
			if(!ent)
				continue;
				
			ent->setVisFrameCount();
		}
	}
	else
	{
		for(std::vector<r_entity_c*>::iterator ir = r_entities.begin(); ir != r_entities.end(); ++ir)
		{
			r_entity_c* ent = *ir;
			
			if(!ent)
				continue;
				
			if(!ent->isStatic())
			{
				// entity is always visible
				ent->setVisFrameCount();
				continue;
			}
			else
			{
				// check if entity has shared leaves with the PVS
				const std::vector<r_bsptree_leaf_c*>& leafs = ent->getLeafs();
			
				for(std::vector<r_bsptree_leaf_c*>::const_iterator ir = leafs.begin(); ir != leafs.end(); ++ir)
				{
					r_bsptree_leaf_c* leaf = *ir;
			
					if(leaf->isVisFramed())
					{
						// leaf is visible
						ent->setVisFrameCount();
						break;
					}
				}
			}
		}
	}
}


r_bsp_model_c::r_bsp_model_c(const std::string &name, bool inline_model)
:r_model_c(name, NULL, 0, MOD_BSP)
{	
	_inline = inline_model;
}

r_bsp_model_c::~r_bsp_model_c()
{
}

const aabb_c	r_bsp_model_c::createAABB(r_entity_c *ent) const
{
	return _aabb;
}

void	r_bsp_model_c::addModelToList(r_entity_c *ent)
{
	//ri.Com_DPrintf("r_bsp_model_c::addModelToList:\n");
	
	if(ent->isVisFramed() && r_frustum.cull(ent->getAABB(), FRUSTUM_CLIPALL))
	{
		return;
	}
	else
	{
		ent->setFrameCount();
		c_entities++;
	}

	for(std::vector<r_surface_c*>::iterator ir = _surfaces.begin(); ir != _surfaces.end(); ++ir)
	{
		r_surface_c *surf = *ir;
		
		if(surf->isFramed())	// already added surface
			continue;
	
		if(!surf->getMesh())
		{
			ri.Com_DPrintf("r_bsp_model_c::addModelToList: surface of type %i has no mesh\n", surf->getFaceType());
			continue;
		}
		
		if(!surf->getShader())
		{
			ri.Com_DPrintf("r_bsp_model_c::addModelToList: surface has no shader\n");
			continue;
		}
		
		if(!r_showinvisible->getInteger() && surf->getShaderRef()->hasFlags(X_SURF_NODRAW))
			continue;
			
		if(r_envmap && surf->getShader()->hasFlags(SHADER_NOENVMAP))
			continue;
			
		surf->setFrameCount();
		
		/*
		switch(surf->facetype)
		{
			case BSPST_BEZIER:
			case BSPST_MESH:
			{
				if(R_CullBSphere(r_frustum, ent->getShared().origin, surf->getMesh()->bbox.radius(), FRUSTUM_CLIPALL))
					return;
			}
			
			default:
				break;
		}
		*/
		
		RB_AddCommand(ent, this, surf->getMesh(), surf->getShader(), NULL, NULL, surf->getLightMapNum(), r_origin.distance(ent->getShared().origin));
		
		if(r_lighting->getInteger())
		{
			// create static interaction light commands
			const r_entity_sub_c& sub = ent->getSubEntity(surf->getSubEntityNum());
					
			const std::vector<r_interaction_c*>& interactions = sub.getInteractions();
					
			for(std::vector<r_interaction_c*>::const_iterator ir = interactions.begin(); ir != interactions.end(); ++ir)
			{
				const r_interaction_c* ia = *ir;
				
				if(ia->getIndexes().empty())
					continue;
				
				RB_AddCommand(ent, this, surf->getMesh(), surf->getShader(), ia->getLight(), (std::vector<index_t>*)&ia->getIndexes(), -1, 0, ia->getAttenuation());
			}
			
			#if 0
			// create dynamic interaction light commands
			
			if(!surf->getShader() || !surf->getShader()->stage_diffusemap)
				continue;
		
			for(std::vector<std::vector<r_light_c*> >::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
			{
				r_light_c* light = *ir;
				
				if(!light)
					continue;
					
				if(!(light->getShared().flags & RF_STATIC))
					continue;
					
				if(!light->isVisible())
					continue;
						
				if(r_lighting->getInteger() == 2)
				{
					if((light->getShared().flags & RF_STATIC) && surf->getLightMapNum())
						continue;
				}
						
			
				if(light->getShared().radius_aabb.intersect(ent->getShared().origin, surf->getMesh()->bbox.radius()))
					RB_AddCommand(ent, this, surf->getMesh(), surf->getShader(), &light, NULL, -1, 0);
			}
			#endif
		}
	}
}

int	r_bsp_model_c::precacheLight(r_entity_c *ent, r_light_c *light) const
{
	int count = 0;

	for(std::vector<r_surface_c*>::const_iterator ir = _surfaces.begin(); ir != _surfaces.end(); ++ir)
	{
		const r_surface_c *surf = *ir;
		
		if(!surf->getMesh())
		{
			ri.Com_DPrintf("r_bsp_model_c::precacheLight: surface of type %i has no mesh\n", surf->getFaceType());
			continue;
		}
		
		if(!surf->getShader())
		{
			ri.Com_DPrintf("r_bsp_model_c::precacheLight: surface has no shader\n");
			continue;
		}
		
		if(!r_showinvisible->getInteger() && surf->getShaderRef()->hasFlags(X_SURF_NODRAW))
			continue;
			
		if(!surf->getShader()->stage_diffusemap)
			continue;
			
		//if(!surf->getMesh()->bbox.intersect(light->getShared().radius_aabb))
		//	continue;
		
		r_interaction_c* ia = light->createInteraction(ent, surf->getMesh());
		
		if(!ia)
			continue;
				
		ent->addInteractionToSubEntity(surf->getSubEntityNum(), ia);
		count++;
	}
	
	return count;
}

void	r_bsp_model_c::draw(const r_command_t *cmd, r_render_type_e type)
{
	RB_SetupModelviewMatrix(cmd->getEntity()->getTransform());

	RB_RenderCommand(cmd, type);
}








