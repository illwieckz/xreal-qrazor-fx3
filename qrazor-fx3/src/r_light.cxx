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
#include "r_local.h"

// xreal --------------------------------------------------------------------


r_light_c::r_light_c()
:r_occlusion_iface_a()
{
	_s.clear();
	_type = LIGHT_OMNI;
	
	_origin = _s.origin;
	
	setupTransform();
	setupProjection();
	setupAttenuation();
	setupFrustum();
	setupShadowMap();
	
	_cluster = -1;
}


r_light_c::r_light_c(const r_entity_t &shared, r_light_type_t type)
:r_occlusion_iface_a()
{
	update(shared, type);
}

r_light_c::~r_light_c()
{
//	if(_shadowmap)
//		delete _shadowmap;
}

void	r_light_c::update(const r_entity_t &shared, r_light_type_t type)
{
	_s = shared;
	_type = type;
	
	_origin = _s.origin + _s.quat * _s.center;
	
	if(_areasurfaces.empty())
		_areasurfaces = std::vector<std::map<const r_surface_c*, std::vector<index_t> > >(1);
	
	setupTransform();
	setupProjection();
	setupAttenuation();
	setupFrustum();
	setupShadowMap();
	
	if(_s.flags & RF_STATIC)
	{
		if(!r_world_tree && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
			ri.Com_Error(ERR_DROP, "r_light_c::ctor: NULL worldmodel");
	
#if 0
		updateVis(_s);
		if(_areas.size())
			ri.Com_DPrintf("light touches %i BSP areas\n", _areas.size());
		
		r_world_tree->precacheLight(this);
		
		int c=0;
		for(uint_t i=0; i<_areasurfaces.size(); i++)
		{
			c += _areasurfaces[i].size();
		}
		ri.Com_DPrintf("light has %i precached surface interactions\n", c);
#else
		updateVis(_s);
		
		r_world_tree->precacheLight(this);
		
		//if(_leafs.size())
		//	ri.Com_DPrintf("light touches %i BSP leaves\n", _leafs.size());
			
		//if(_areas.size())
		//	ri.Com_DPrintf("light touches %i BSP areas\n", _areas.size());
#endif
	}
	else
	{
		updateVis(_s);
	
		_cluster = -1;
	}
}

void 	r_light_c::setupTransform()
{
	_transform.setupTranslation(_s.origin);
	_transform.multiplyRotation(_s.quat);
	_transform.multiplyScale(_s.scale);
	
	_transform_inv = _transform.affineInverse();
}

void	r_light_c::setupAttenuation()
{		
	_attenuation.setupTranslation(0.5, 0.5, 0.5);			// bias
	_attenuation.multiplyScale(0.5, 0.5, 0.5);			// scale
	_attenuation.multiply(_projection);				// light projection (frustum)
//	_attenuation.multiply(_transform_inv);				// light view matrix
}

void	r_light_c::setupProjection()
{
	switch(_type)
	{
		case LIGHT_OMNI:
		{
			_projection.setupScale(1.0/_s.radius[0], 1.0/_s.radius[1], 1.0/_s.radius[2]);
			break;
		}
		
		case LIGHT_PROJ:
		{
#if 0
			vec_t fov_x = 90;
			vec_t fov_y = CalcFOV(fov_x, 100, 100);
		
			double n = r_znear->getValue();
			double f = r_zfar->getValue();
		
			double r = n * tan(fov_x * M_PI / 360.0);
			double l = -r;
			
			double t = n * tan(fov_y * M_PI / 360.0);
			double b = -t;
			
			RB_Frustum(_projection, l, r, b, t, n, f);
#else
			//matrix_c m;
			//m.fromVectorsFRU(_s.target, _s.right, _s.up);
			//m[3][0] = m[1][0];	m[3][1] = m[1][1];	m[3][2] = m[1][2];	m[3][3] = m[1][3];
			//_projection = m.affineInverse();
			
			//vec_t fov_x = acos(_s.target.dotProduct(lr) /(lt.length() * lr.length()));
			vec_t fov_x = 60;
			vec_t fov_y = CalcFOV(fov_x, _s.right.length() * 2, _s.up.length() * 2);
			
			//double n = r_znear->getValue();
			double n = 1.0;
			
			//double f = r_zfar->getValue();
			double f = _s.target.length();
		
			//double r = _s.target.dotProduct(_s.right);
			//double r = _s.right.length();
			double r = n * tan(fov_x * M_PI / 360.0);
			//double r = lr.length();
			double l = -r;
			
			//double t = _s.target.dotProduct(_s.up);
			//double t = _s.up.length();
			double t = n * tan(fov_y * M_PI / 360.0);
			//double t = lu.length();
			double b = -t;
			
			RB_OpenGLFrustum(_projection, l, r, b, t, n, f);
#endif
			break;
		}
		
		default:
			ri.Com_Error(ERR_DROP, "r_light_c::setupProjection: bad entity type %i", _type);
	}
}

void	r_light_c::setupFrustum()
{
	// TODO: figure out quake style matrix->frustum planes extraction
}

void	r_light_c::setupShadowMap()
{
#if 0
	switch(_type)
	{
		case LIGHT_OMNI:
		{
			_shadowmap = new r_image_c(GL_TEXTURE_CUBE_MAP_ARB, "_shadowmap", vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), IMAGE_NOMIPMAP | IMAGE_NOPICMIP, NULL, false);
			
			_shadowmap->bind();
			
			xglTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);
			
			xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, r_filter_max);
			xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, r_filter_max);
			
			xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			
			xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
			xglTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
				
			for(int i=0; i<6; i++)
			{
				xglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB+i, 0, r_depth_format, _shadowmap->getWidth(), _shadowmap->getHeight(), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 0);
			//	xglTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB+i, 0, GL_RGB, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger(), 0, GL_RGB, GL_FLOAT, 0);
			}
			break;
		}
		
		case LIGHT_PROJ:
		{
			// TODO:
			_shadowmap = NULL;
			break;
		}
	}
#endif
}

bool	r_light_c::hasSurface(int areanum, const r_surface_c *surf)
{
	if(areanum < 0 || areanum >= (int)_areasurfaces.size())
	{
		ri.Com_Error(ERR_DROP, "r_light_c::hasSurface: %i areanum\n", areanum);
		return false;
	}

	std::map<const r_surface_c*, std::vector<index_t> >::iterator ir = _areasurfaces[areanum].find(surf);
	
	if(ir != _areasurfaces[areanum].end())
		return true;
	
	return false;
}

void	r_light_c::addSurface(int areanum, const r_surface_c *surf)
{
	r_mesh_c* mesh;
	
	if(!(mesh = surf->getMesh()))
		return;
	
	if(mesh->isNotValid())
		return;
	
	if(_areasurfaces.empty())
	{
		ri.Com_Error(ERR_DROP, "r_light_c::addSurface: 0 surfaces\n");
		return;
	}
	
	if(areanum < 0 || areanum >= (int)_areasurfaces.size())
	{
		ri.Com_Error(ERR_DROP, "r_light_c::addSurface: %i areanum\n", areanum);
		return;
	}

	std::vector<index_t> indexes;

	for(unsigned int i=0; i<mesh->indexes.size(); i += 3)
	{
		const vec3_c &v0 = mesh->vertexes[mesh->indexes[i+0]];
		const vec3_c &v1 = mesh->vertexes[mesh->indexes[i+1]];
		const vec3_c &v2 = mesh->vertexes[mesh->indexes[i+2]];
	
#if 0
		// triangle visibility test
		cplane_c p;
		p.fromThreePointForm(v0, v1, v2);
		
		if(p.onSide(_s.origin) == SIDE_FRONT)
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
#elif 0
		// triangle vertices interaction test		
		if()
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
#elif 0
		// triangle edges interaction test
		if(	_s.radius_bbox.intersect(v0, v1)	||
			_s.radius_bbox.intersect(v1, v2)	||
			_s.radius_bbox.intersect(v2, v0)
		)
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
		
#else
		// triangle planes interaction test
		cplane_c p;
		p.fromThreePointForm(v0, v1, v2);
		
		cbbox_c b;
		b.clear();
		b.addPoint(v0);
		b.addPoint(v1);
		b.addPoint(v2);
		
		if(	p.onSide(_s.radius_bbox) == SIDE_CROSS		// check if aabb crosses triangle plane
			&& p.onSide(_origin) == SIDE_BACK		// check if light origin + light center is in positive half-space
			&& b.intersect(_s.radius_bbox)			// check if triangle aabb touches light aabb
		)
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
#endif
	}
	
	if(indexes.size())
		_areasurfaces[areanum].insert(std::make_pair(surf, indexes));
}


