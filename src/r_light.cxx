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



r_interaction_c::r_interaction_c(r_entity_c* ent, r_light_c* light, const std::vector<index_t> &indexes)
{
	_entity		= ent;
	_light		= light;

	_attenuation	= light->getAttenuation() * (light->getView() * ent->getTransform());
	
	_indexes	= indexes;
}


/*
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
*/

r_light_c::r_light_c(const r_entity_t &shared, r_light_type_t type)
:r_occlusion_iface_a()
{
	update(shared, type);
}

r_light_c::~r_light_c()
{
	// TODO
}

void	r_light_c::update(const r_entity_t &shared, r_light_type_t type)
{
	_s = shared;
	_type = type;
	
	_origin = _s.origin + _s.quat * _s.center;
	
//	if(_areasurfaces.empty())
//		_areasurfaces = std::vector<std::map<const r_surface_c*, std::vector<index_t> > >(1);
	
	setupTransform();
	setupProjection();
	setupAttenuation();
	setupFrustum();
	
	setupShadowMapProjection();
	setupShadowMapView();
	
	if(_s.flags & RF_STATIC)
	{
		if(!r_world_tree && !(r_newrefdef.rdflags & RDF_NOWORLDMODEL))
			ri.Com_Error(ERR_DROP, "r_light_c::ctor: NULL worldmodel");
	
		updateVis(_s);
		
		r_world_tree->precacheLight(this);
		
		//if(_leafs.size())
		//	ri.Com_DPrintf("light touches %i BSP leaves\n", _leafs.size());
			
		//if(_areas.size())
		//	ri.Com_DPrintf("light touches %i BSP areas\n", _areas.size());
		
		int count = 0;
		
		// touch static entities
		for(std::vector<r_entity_c*>::iterator ir = r_entities.begin(); ir != r_entities.end(); ++ir)
		{
			r_entity_c* ent = *ir;
		
			if(!ent)
				continue;
				
			if(!(ent->getShared().flags & RF_STATIC))
				continue;
				
			const r_model_c* model = ent->getModel();
			if(model)
			{
				if(!ent->getAABB().intersect(_s.radius_aabb))
					continue;
					
				count += model->precacheLight(ent, this);
			}
		}
		
		//ri.Com_DPrintf("light has %i precached static entities interactions\n", count);
		
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
			//matrix_c m;
			//m.fromVectorsFRU(_s.target, _s.right, _s.up);
			//m[3][0] = m[1][0];	m[3][1] = m[1][1];	m[3][2] = m[1][2];	m[3][3] = m[1][3];
			//_projection = m.affineInverse();
			
			//vec_t fov_x = acos(_s.target.dotProduct(lr) /(lt.length() * lr.length()));
			vec_t fov_x = 60;
			vec_t fov_y = CalcFOV(fov_x, _s.right.length() * 2, _s.up.length() * 2);
			
			double n = 1.0;
			double f = _s.target.length();
		
			//double r = _s.target.dotProduct(_s.right);
			//double r = _s.right.length();
			double r = n * tan(fov_x * M_PI / 360.0);
			double l = -r;
			
			//double t = _s.target.dotProduct(_s.up);
			//double t = _s.up.length();
			double t = n * tan(fov_y * M_PI / 360.0);
			double b = -t;
			
			RB_OpenGLFrustum(_projection, l, r, b, t, n, f);
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

void	r_light_c::setupShadowMapProjection()
{
	switch(_type)
	{
		case LIGHT_OMNI:
			break;
		
		case LIGHT_PROJ:
		{
			double n = 1.0;
			double f = 65536.0;

			double r = n * tan(90 * M_PI / 360.0);
			double l = -r;
	
			double t = n * tan(CalcFOV(90, vid_pbuffer_width->getInteger(), vid_pbuffer_height->getInteger()) * M_PI / 360.0);
			double b = -t;
	
			RB_OpenGLFrustum(_shadowmap_projection, l, r, b, t, n, f);
			break;
		}
		
		default:
			ri.Com_Error(ERR_DROP, "r_light_c::setupShadowMapProjection: bad entity type %i", _type);
	}
}

void	r_light_c::setupShadowMapView()
{
	switch(_type)
	{
		case LIGHT_OMNI:
			break;
		
		case LIGHT_PROJ:
		{
			const vec3_c look_at = _s.target + _s.up;
			vec3_c angles(false);
			Vector3_ToAngles(look_at, angles);
			
			matrix_c m;
			m.setupTranslation(_s.origin);
			m.multiplyRotation(angles);
			
			_shadowmap_view = m.affineInverse();
			break;
		}
		
		default:
			ri.Com_Error(ERR_DROP, "r_light_c::setupShadowMapView: bad entity type %i", _type);
	}
}

r_interaction_c*	r_light_c::createInteraction(r_entity_c* ent, const r_mesh_c *mesh)
{
	if(!ent)
		return NULL;
			
	if(!mesh || mesh->isNotValid())
		return NULL;
		
	std::vector<index_t>	indexes;
	const matrix_c&		transform = ent->getTransform();
	
	for(unsigned int i=0; i<mesh->indexes.size(); i += 3)
	{
		vec3_c v0 = transform * mesh->vertexes[mesh->indexes[i+0]];
		vec3_c v1 = transform * mesh->vertexes[mesh->indexes[i+1]];
		vec3_c v2 = transform * mesh->vertexes[mesh->indexes[i+2]];
		
		v0[0] += transform[0][3];
		v0[1] += transform[1][3];
		v0[2] += transform[2][3];
		
		v1[0] += transform[0][3];
		v1[1] += transform[1][3];
		v1[2] += transform[2][3];
		
		v2[0] += transform[0][3];
		v2[1] += transform[1][3];
		v2[2] += transform[2][3];
	
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
		
		aabb_c aabb;
		aabb.clear();
		aabb.addPoint(v0);
		aabb.addPoint(v1);
		aabb.addPoint(v2);
		
		if(	p.onSide(_s.radius_aabb) == SIDE_CROSS		// check if aabb crosses triangle plane
			&& p.onSide(_origin) == SIDE_BACK		// check if light origin + light center is in positive half-space
			&& aabb.intersect(_s.radius_aabb)		// check if triangle aabb touches light aabb
		)
		{
			indexes.push_back(mesh->indexes[i+0]);
			indexes.push_back(mesh->indexes[i+1]);
			indexes.push_back(mesh->indexes[i+2]);
		}
#endif
	}
	
	if(!indexes.empty())
		return new r_interaction_c(ent, this, indexes);
	else
		return NULL;
}


void	R_InitLights()
{
	if(!r_lights.empty())
		R_ShutdownLights();

	r_lights = std::vector<r_light_c*>(MAX_ENTITIES, NULL);
}

void	R_ShutdownLights()
{
	X_purge<std::vector<r_light_c*> >(r_lights);
	r_lights.clear();
}


int	R_GetNumForLight(r_light_c *light)
{
	if(!light)
	{
		ri.Com_Error(ERR_DROP, "R_GetNumForLight: NULL parameter\n");
		return -1;
	}

	for(uint_t i=0; i<r_lights.size(); i++)
	{
		if(light == r_lights[i])
			return i;
	}
	
	ri.Com_Error(ERR_DROP, "R_GetNumForLight: bad pointer\n");
	return -1;
}


r_light_c*	R_GetLightByNum(int num)
{
	r_light_c* light = NULL;
	
	try
	{
		light = r_lights.at(num);
	}
	catch(...)
	{
		ri.Com_Error(ERR_DROP, "R_GetLightByNum: bad number %i\n", num);
	}

	return light;
}


