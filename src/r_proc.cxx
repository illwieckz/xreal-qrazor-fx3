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
#include 	"r_local.h"



r_areaportal_c::r_areaportal_c(const std::vector<vec3_c> &vertexes, int areas[2])
{
	if(vertexes.size() != 4)
	{
		ri.Com_Error(ERR_DROP, "r_areaportal_c::ctor: bad vertices number %i", vertexes.size());
	}
	else
	{
		_plane.fromThreePointForm(vertexes[0], vertexes[1], vertexes[2]);
	}

	_vertexes_original	= vertexes;
	_vertexes		= vertexes;
	_areas[0]		= areas[0];
	_areas[1]		= areas[1];

	_bbox.clear();
	
	for(std::vector<vec3_c>::const_iterator ir = vertexes.begin(); ir != vertexes.end(); ir++)
		_bbox.addPoint(*ir);
	
	//Com_Printf("iap: %s ", ap..toString());
	//Com_Printf("%i %i %i \n", points_num, ap.areas[SIDE_FRONT], ap.areas[SIDE_BACK]);
}

void	r_areaportal_c::adjustFrustum(const r_frustum_c &frustum)
{
	if(r_frustum[FRUSTUM_NEAR].onSide(_bbox) == SIDE_CROSS)
	{
		for(int i=0; i<FRUSTUM_PLANES; i++)
			_frustum[i] = r_frustum[i];
		return;
	}

	// left, right, down, up
	if(_plane.onSide(r_origin) == SIDE_BACK)
	{
		/*
		clipEdge(frustum, _vertexes_original[0], _vertexes_original[1], _vertexes[0], _vertexes[1]);
		clipEdge(frustum, _vertexes_original[1], _vertexes_original[2], _vertexes[1], _vertexes[2]);
		clipEdge(frustum, _vertexes_original[2], _vertexes_original[3], _vertexes[2], _vertexes[3]);
		clipEdge(frustum, _vertexes_original[3], _vertexes_original[0], _vertexes[3], _vertexes[0]);
		
		clipEdge(frustum, _vertexes_original[0], _vertexes_original[2], _vertexes[0], _vertexes[2]);
		clipEdge(frustum, _vertexes_original[1], _vertexes_original[3], _vertexes[1], _vertexes[3]);
		*/
	
		// build frustum's planes clockwise
		for(int i=0; i<4; i++)
		{
			/*
			if(frustum[i].onSide(_vertexes[i]) == SIDE_BACK && frustum[i].onSide(_vertexes[(i+1)%4]))
			{
				// edge behind plane, so use the clipping plane from previous frustum
				_frustum[i] = frustum[i];
			}
			else
			*/
			{
				//clipEdge(frustum, _vertexes_original[i], _vertexes_original[(i+1)%4], _vertexes[i], _vertexes[(i+1)%4]);
				_frustum[i].fromThreePointForm(r_origin, _vertexes[i], _vertexes[(i+1)%4]);
				_frustum[i]._type = PLANE_ANYZ;
				_frustum[i].setSignBits();
			}
			
				
			
		}
	}
	else
	{
		/*
		clipEdge(frustum, _vertexes_original[0], _vertexes_original[3], _vertexes[0], _vertexes[3]);
		clipEdge(frustum, _vertexes_original[1], _vertexes_original[1], _vertexes[1], _vertexes[1]);
		clipEdge(frustum, _vertexes_original[2], _vertexes_original[2], _vertexes[2], _vertexes[2]);
		clipEdge(frustum, _vertexes_original[3], _vertexes_original[3], _vertexes[3], _vertexes[3]);
			
		clipEdge(frustum, _vertexes_original[0], _vertexes_original[2], _vertexes[0], _vertexes[2]);
		clipEdge(frustum, _vertexes_original[1], _vertexes_original[3], _vertexes[1], _vertexes[3]);
		*/
	
		// build frustum's planes counter clockwise
		for(int i=0; i<4; i++)
		{
			/*
			if(frustum[i].onSide(_vertexes[i]) == SIDE_BACK && frustum[i].onSide(_vertexes[(i+3)%4]))
			{
				// edge behind plane, so use the clipping plane from previous frustum
				_frustum[i] = frustum[i];
			}
			else
			*/
			{
				//clipEdge(frustum, _vertexes_original[i], _vertexes_original[(i+3)%4], _vertexes[i], _vertexes[(i+3)%4]);
				_frustum[i].fromThreePointForm(r_origin, _vertexes[i], _vertexes[(i+3)%4]);
				_frustum[i]._type = PLANE_ANYZ;
				_frustum[i].setSignBits();
			}
		}
	}
	
	_frustum[FRUSTUM_NEAR] = frustum[FRUSTUM_NEAR];
	_frustum[FRUSTUM_FAR] = frustum[FRUSTUM_FAR];
}

void	r_areaportal_c::clipEdge(const r_frustum_c &frustum, const vec3_c &v1, const vec3_c &v2, vec3_c &out1, vec3_c &out2)
{
	vec3_c		intersect;
	plane_side_e	side1, side2;

	// check edge against all frustum planes
	for(int i=0; i<FRUSTUM_PLANES; i++)
	{
		side1 = frustum[i].onSide(v1);
		side2 = frustum[i].onSide(v2);
		
		if(side1 == SIDE_BACK && side2 == SIDE_BACK)
			continue;	//edge behind plane
			
		if(side1 == SIDE_BACK || side2 == SIDE_BACK)
			intersect = r_frustum[i].intersect(v1, v2);
			
		if(side1 == SIDE_BACK)
			out1 = intersect;
		
		if(side2 == SIDE_BACK)
			out2 = intersect;
	}
}

void	r_areaportal_c::drawPortal()
{
	plane_side_e side = _plane.onSide(r_origin);

	if(!isVisible())
		xglColor4fv(color_red);
		
	else if(side == SIDE_FRONT)
		xglColor4fv(color_green);
		
	else
		xglColor4fv(color_blue);

	xglBegin(GL_QUADS);
	if(side == SIDE_FRONT)
	{
		for(int i=0; i<4; ++i)
		{
			//xglColor4f(0.0, 0.25*i, 0.0, 1.0);
			xglVertex3fv(_vertexes[i]);
		}
	}
	else
	{
		for(int i=3; i>=0; --i)
		{
			//xglColor4f(0.0, 0.0, 1.0-(0.25*i), 1.0);
			xglVertex3fv(_vertexes[i]);
		}
	}
	xglEnd();
}

void	r_areaportal_c::drawFrustum()
{
	if(isVisible())
	{
		xglColor4f(0.7, 0.7, 0.7, 0.3);
		xglBegin(GL_TRIANGLES);
		
		if(_plane.onSide(r_origin) == SIDE_FRONT)
		{
			for(int i=0; i<4; i++)
			{
				xglVertex3fv(r_origin + (r_forward * r_znear->getValue() + 1.0));
				xglVertex3fv(_vertexes[i]);
				xglVertex3fv(_vertexes[(i+1)%4]);
			}
		}
		else
		{
			for(int i=0; i<4; i++)
			{
				xglVertex3fv(r_origin + (r_forward * r_znear->getValue() + 1.0));
				xglVertex3fv(_vertexes[i]);
				xglVertex3fv(_vertexes[(i+3)%4]);
			}
		}
		xglEnd();
	}
}



r_proctree_c::r_proctree_c(const std::string &name)
{
	char*			buf;
	char*			buf_p;
	char*			token = NULL;
	int			length;
	std::string		fullgetName();


	//
	// load the file
	//
	length = ri.VFS_FLoad(name, (void **)&buf);
	if (!buf)
		ri.Com_Error(ERR_DROP, "R_LoadProc: Couldn't load %s", name.c_str());
	
	buf_p = buf;
	
	std::string ident = Com_Parse(&buf_p);
	if(ident != (std::string(PROC_IDENTSTRING + std::string(PROC_VERSION))))
		ri.Com_Error(ERR_DROP, "R_LoadProc: %s has wrong ident ('%s' should be '%s%s')", name.c_str(), ident.c_str(), PROC_IDENTSTRING, PROC_VERSION);


	ri.Com_DPrintf("loading '%s' ...\n", name.c_str());
	
	
	while(true)
	{
		token = Com_Parse(&buf_p);
		
		if(!token || !token[0])
			break;
			
		if(X_strcaseequal(token, "model"))
		{
			Com_Parse(&buf_p);	// skip '{'
			
			std::string modelname = Com_Parse(&buf_p);
			
			r_proc_model_c *model = new r_proc_model_c(modelname);
		
			model->load(&buf_p);
			
			model->setupMeshes();
			
			if(X_strnequal(model->getName(), "_area", 5))
			{
				r_proctree_area_c *area = new r_proctree_area_c();
				
				area->bbox = model->getBBox();
				area->surfaces = model->_surfaces;
				area->model = model;
				
				_areas.push_back(area);
			}
			else
				r_models.push_back(model);
		}
		
		if(X_strcaseequal(token, "nodes"))
		{
			loadNodes(&buf_p);
		}
		
		if(X_strcaseequal(token, "interAreaPortals"))
		{
			loadInterAreaPortals(&buf_p);
		}

	}
		
	ri.VFS_FFree(buf);
}

r_proctree_c::~r_proctree_c()
{
	//TODO
}

void	r_proctree_c::precacheLight(r_light_c* light)
{
	light->setAreaNum(_areas.size());
		
	const std::vector<int>& areas = light->getAreas();
	
	for(std::vector<int>::const_iterator ir = areas.begin(); ir != areas.end(); ++ir)
	{
		int areanum = *ir;
		r_proctree_area_c *area = _areas[areanum];
		
		for(std::vector<r_surface_c*>::const_iterator ir = area->surfaces.begin(); ir != area->surfaces.end(); ++ir)
		{
			r_surface_c *surf = *ir;
		
			if(!light->getShared().radius_bbox.intersect(surf->getMesh()->bbox))
				continue;
		
			if(light->hasSurface(areanum, surf))
				continue;

			light->addSurface(areanum, surf);
		}
		
		//litArea_r(*ir, light, true);
		//_areas[*ir]->lights.push_back(light);
	}
	
	/*
	int area = pointInArea(light->getShared().origin);
				
	if(area != -1)
		litArea_r(area, light, true);
	*/
}

void	r_proctree_c::update()
{
	/*
	// current viewcluster
	if(!(r_newrefdef.rdflags & RDF_NOWORLDMODEL) && !r_mirrorview)
	{
		int	area = 0;
	
		_viewcluster_old = _viewcluster;
		
		//if(r_portal_view)
		//{
		//	r_viewcluster_old = -1;
		//	leaf = (r_bsptree_leaf_c*)pointInLeaf(r_portal_org);
		//}
		//else
		{
			_viewcluster_old = _viewcluster;
			//area = pointInArea(r_origin);
		}

		_viewcluster = area;
	}
	*/
}

void	r_proctree_c::updateArea_r(int areanum, const r_frustum_c &frustum)
{
	r_proctree_area_c *area = _areas[areanum];
	
//	if(area->visframecount == r_visframecount)
//		return;
		
//	if(area->framecount == r_framecount)
//		return;
	
	/*
	if(r_newrefdef.areabits)
	{
		if(!(r_newrefdef.areabits[areanum>>3] & (1<<(areanum&7))))
			return;		// not visible
	}
	*/
	
	if(frustum.cull(area->bbox))
		return;
		
	area->visframecount = r_visframecount;
	area->framecount = r_framecount;
	c_leafs++;
	
	// create general renderer commands
	for(std::vector<r_surface_c*>::const_iterator ir = area->surfaces.begin(); ir != area->surfaces.end(); ++ir)
	{
		r_surface_c *surf = *ir;
		
		if(surf->getFrameCount() == r_framecount)	// already added surface
			continue;
		
		if(!surf->getMesh())
		{
			//ri.Com_DPrintf("r_bsp_model_c::addSurfaceToList: surface of type %i has no mesh\n", surf->facetype);
			continue;
		}
	
		if(!surf->getShader())
		{
			ri.Com_DPrintf("r_proc_model_c::addModelToList: surface has no shader\n");
			continue;
		}
		
		if(!r_showinvisible->getValue() && surf->getShader()->hasFlags(SHADER_NODRAW))
			continue;
			
		if(r_envmap && surf->getShader()->hasFlags(SHADER_NOENVMAP))
			continue;
				
		if(frustum.cull(surf->getMesh()->bbox))
			continue;
		
		surf->setFrameCount();
	
		RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), NULL, NULL, -1, 0);
	}	
	
	// mark lights and create light commands
	if(r_lighting->getInteger())
	{
		for(std::vector<std::vector<r_light_c> >::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			std::vector<r_light_c>& lights = *ir;
			
			for(std::vector<r_light_c>::iterator ir = lights.begin(); ir != lights.end(); ++ir)
			{
				r_light_c& light = *ir;
			
				if(!light.hasArea(areanum))
					continue;
				
				//if(light.isVisible())
				//	continue;
			
				if(frustum.cull(light.getShared().radius_bbox))
					continue;
			
				light.setVisFrameCount();
				c_lights++;
			
				if(light.getShared().flags & RF_STATIC)
				{
					const std::map<const r_surface_c*, std::vector<index_t> >& surfaces = light.getAreaSurfaces(areanum);
			
					for(std::map<const r_surface_c*, std::vector<index_t> >::const_iterator ir = surfaces.begin(); ir != surfaces.end(); ++ir)
					{
						const r_surface_c* surf = ir->first;

						if(surf->getFrameCount() != r_framecount)
							continue;	// surface is not in this frame
					
						RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), &light, (std::vector<index_t>*)&ir->second, -1, 0);
					}
				}
				else
				{
					for(std::vector<r_surface_c*>::const_iterator ir = area->surfaces.begin(); ir != area->surfaces.end(); ++ir)
					{
						r_surface_c *surf = *ir;
			
						if(surf->getFrameCount() != r_framecount)
							continue;	// surface is not in this frame
				
						if(!light.getShared().radius_bbox.intersect(surf->getMesh()->bbox))
							continue;
				
						RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), &light, NULL, -1, 0);
					}
				}
			}
		}
	}
	
	//TODO: mark entities
	
	// go trough the area portals
	for(std::vector<r_areaportal_c*>::const_iterator ir = area->areaportals.begin(); ir != area->areaportals.end(); ++ir)
	{
		r_areaportal_c* iap = *ir;
		
		if(!iap->isVisible())
		{
			if(!frustum.cull(iap->getBBox()))
			{
				for(int i=0; i<2; i++)
				{
					// skip current area
					if(iap->getArea(i) == areanum)
						continue;
		
					// never go back to the source area
					if(iap->getArea(i) == _sourcearea)
						continue;
						
					iap->adjustFrustum(frustum);
					
					iap->setVisFrameCount();
					
					updateArea_r(iap->getArea(i), iap->getFrustum());
				}
			}
		}
	}
}

void	r_proctree_c::draw()
{
	r_visframecount++;
	
	// mark entities
	r_world_entity.setVisFrameCount();
	c_entities++;

	for(std::vector<std::vector<r_entity_c> >::iterator ir = r_entities.begin(); ir != r_entities.end(); ++ir)
	{
		std::vector<r_entity_c>& entities = *ir;
			
		for(std::vector<r_entity_c>::iterator ir = entities.begin(); ir != entities.end(); ++ir)
		{
			r_entity_c& ent = *ir;
		
			ent.setVisFrameCount();
			c_entities++;
		}
	}
	
	if(!r_drawworld->getValue())
		return;
	
	_sourcearea = pointInArea(r_origin);
	
	if(_sourcearea == -1)
	{
		for(unsigned i=0; i<_areas.size(); i++)
		{
			r_proctree_area_c *area = _areas[i];
			
			if(!area->model)
				continue;
			
			if(r_newrefdef.areabits)
			{
				if(!(r_newrefdef.areabits[i>>3] & (1<<(i&7))))
					continue;		// not visible
			}
			
			area->visframecount = r_visframecount;
			c_leafs++;
		}
	}
	else
	{
		updateArea_r(_sourcearea, r_frustum);
	}

	/*
	if(_sourcearea == -1)
	{
		for(unsigned i=0; i<_areas.size(); i++)
		{
			r_proctree_area_c *area = _areas[i];
			
			if(!area->model)
				continue;
			
			if(r_newrefdef.areabits)
			{
				if(!(r_newrefdef.areabits[i>>3] & (1<<(i&7))))
					continue;		// not visible
			}
		
			area->model->addModelToList(&r_world_entity);
		}
	}
	else
	{
		drawArea_r(_sourcearea);
	}
	*/
}

void	r_proctree_c::drawAreaPortals()
{
	if(!r_showareaportals->getValue())
		return;

	RB_SetupModelviewMatrix(matrix_identity);

	xglDisable(GL_DEPTH_TEST);
	
	xglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	for(std::vector<r_areaportal_c*>::const_iterator ir = _areaportals.begin(); ir != _areaportals.end(); ++ir)
	{
		r_areaportal_c* iap = *ir;
		iap->drawPortal();
	}
	xglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
#if 0
	xglEnable(GL_BLEND);
	xglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for(std::vector<r_areaportal_c*>::const_iterator ir = _areaportals.begin(); ir != _areaportals.end(); ++ir)
	{
		r_areaportal_c* iap = *ir;
		iap->drawFrustum();
	}
	xglDisable(GL_BLEND);
#endif
	
	xglEnable(GL_DEPTH_TEST);
	
	xglColor4fv(color_white);
}

#if 0
void	r_proctree_c::drawArea_r(int areanum)
{
	r_proctree_area_c *area = _areas[areanum];
	
	if(area->visframecount != r_visframecount)
		return;
		
	if(area->framecount == r_framecount)
		return;
	
	for(std::vector<r_surface_c*>::const_iterator ir = area->surfaces.begin(); ir != area->surfaces.end(); ++ir)
	{
		r_surface_c *surf = *ir;
		
		if(surf->getFrameCount() == r_framecount)	// already added surface
			continue;
		
		if(!surf->getMesh())
		{
			//ri.Com_DPrintf("r_bsp_model_c::addSurfaceToList: surface of type %i has no mesh\n", surf->facetype);
			continue;
		}
	
		if(!surf->getShader())
		{
			ri.Com_DPrintf("r_proc_model_c::addModelToList: surface has no shader\n");
			continue;
		}
				
		if(r_frustum.cull(surf->getMesh()->bbox, area->clipflags))
			continue;
		
		surf->setFrameCount();
	
		RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), NULL, NULL, -1, 0);
	}
	
	area->framecount = r_framecount;
	
	if(r_lighting->getValue() == 1)
	{
		for(std::map<int, r_light_c>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c& light = ir->second;
			
			if(!light.isVisible())
				continue;
				
			if(!light.hasArea(areanum))
				continue;
			
			if(light.getShared().flags & RF_STATIC)
			{
				const std::map<const r_surface_c*, std::vector<index_t> >& surfaces = light.getAreaSurfaces(areanum);
			
				for(std::map<const r_surface_c*, std::vector<index_t> >::const_iterator ir = surfaces.begin(); ir != surfaces.end(); ++ir)
				{
					const r_surface_c* surf = ir->first;

					if(surf->getFrameCount() != r_framecount)
						continue;	// surface is not in this frame
					
					RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), &light, (std::vector<index_t>*)&ir->second, -1, 0);
				}
			}
			else
			{
				for(std::vector<r_surface_c*>::const_iterator ir = area->surfaces.begin(); ir != area->surfaces.end(); ++ir)
				{
					r_surface_c *surf = *ir;
			
					if(surf->getFrameCount() != r_framecount)
						continue;	// surface is not in this frame
				
					if(!light.getShared().radius_bbox.intersect(surf->getMesh()->bbox))
						continue;
				
					RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), &light, NULL, -1, 0);
				}
			}
		}
	
		/*
		for(std::vector<r_light_c*>::const_iterator ir = area->lights.begin(); ir != area->lights.end(); ++ir)
		{
			r_light_c *light = *ir;
			
			if(!light)
				continue;
				
			if(!light->isVisible())
				continue;
			
			
			const std::map<const r_surface_c*, std::vector<index_t> >& surfaces = light->getAreaSurfaces(areanum);
			
			for(std::map<const r_surface_c*, std::vector<index_t> >::const_iterator ir = surfaces.begin(); ir != surfaces.end(); ir++)
			{
				const r_surface_c* surf = ir->first;

				if(surf->getFrameCount() != r_framecount)
					continue;
					
				RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), light, (std::vector<index_t>*)&ir->second, -1, 0);
			}
		}
		*/
	}
	
	for(std::vector<r_areaportal_c*>::const_iterator ir = area->areaportals.begin(); ir != area->areaportals.end(); ++ir)
	{
		r_areaportal_c* iap = *ir;
		
		if(iap->isVisible())
		{
			for(int i=0; i<2; i++)
			{
				drawArea_r(iap->getArea(i));
			}
		}
	}
}

void	r_proctree_c::litArea_r(int areanum, r_light_c *light, bool precache)
{
	//ri.Com_Printf("r_proctree_c::litArea_r %i\n", areanum);

	r_proctree_area_c *area = _areas[areanum];
	
	/*
	std::vector<r_light_c*>::iterator ir = find(area->lights.begin(), area->lights.end(), static_cast<r_light_c*>(light));
	if(ir != area->lights.end())
		return;
	else
		area->lights.push_back(light);
	*/
	
	for(std::vector<r_surface_c*>::const_iterator ir = area->surfaces.begin(); ir != area->surfaces.end(); ++ir)
	{
		r_surface_c *surf = *ir;
		
		if(!light->getShared().radius_bbox.intersect(surf->getMesh()->bbox))
			continue;
		
		if(light->hasSurface(areanum, surf))
			continue;

		light->addSurface(areanum, surf);
	}
	
	/*	
	for(std::vector<r_areaportal_c*>::const_iterator ir = area->areaportals.begin(); ir != area->areaportals.end(); ++ir)
	{
		r_areaportal_c* iap = *ir;
		
		if(iap->visframecount == r_visframecount)
		{
			if(iap->bbox.intersect(light->getShared().radius_bbox))
			{
				for(int i=0; i<2; i++)
				{			
					litArea_r(iap->areas[i], light, precache);
				}
			}
		
		}
	}
	*/
}
#endif

int	r_proctree_c::pointInArea_r(const vec3_c &p, int nodenum)
{
	if(nodenum < 0 || nodenum >= (int)_nodes.size())
	{
		ri.Com_Error(ERR_DROP, "r_proctree_c::pointInArea_r: bad num %i", nodenum);
	}
	
	r_proctree_node_c *node = _nodes[nodenum];

	plane_side_e side = node->plane.onSide(p);
	
	nodenum = node->children[side];
		
	if(nodenum < 0)
		return -1 -nodenum;
			
	if(nodenum == 0)
		return -1;
	
	return pointInArea_r(p, nodenum);
}

int	r_proctree_c::pointInArea(const vec3_c &p)
{
	if(_nodes.empty())
		return 0;
		
	return pointInArea_r(p, 0);
}

void	r_proctree_c::boxAreas_r(const cbbox_c &bbox, std::vector<int> &areas, int nodenum)
{
	if(nodenum < 0)
	{
		int areanum = -1 -nodenum;
		
		if(areanum < 0 || areanum >= (int)_areas.size())
		{
			ri.Com_Error(ERR_DROP, "r_proctree_c::boxAreas_r: bad areanum %i", areanum);
		}
		
		std::vector<int>::iterator ir = find(areas.begin(), areas.end(), areanum);
		if(ir == areas.end())
			areas.push_back(areanum);
			
		return;
	}

	r_proctree_node_c *node = _nodes[nodenum];
	
	plane_side_e side = node->plane.onSide(bbox);
	
	switch(side)
	{
		case SIDE_FRONT:
		{
			if(node->children[SIDE_FRONT])
				boxAreas_r(bbox, areas, node->children[SIDE_FRONT]);
			break;
		}
		
		case SIDE_BACK:
		{
			if(node->children[SIDE_BACK])
				boxAreas_r(bbox, areas, node->children[SIDE_BACK]);
			break;
		}
		
		case SIDE_CROSS:
		{
			// go down both
			if(node->children[SIDE_FRONT])
				boxAreas_r(bbox, areas, node->children[SIDE_FRONT]);
				
			if(node->children[SIDE_BACK])
				boxAreas_r(bbox, areas, node->children[SIDE_BACK]);
			break;
		}
		
		default:
			break;
	}
}
	
void	r_proctree_c::boxAreas(const cbbox_c &bbox, std::vector<int> &areas)
{
	areas.clear();

	if(_nodes.empty())
	{
		areas.push_back(0);
		return;
	}
	
	boxAreas_r(bbox, areas, 0);
}


void	r_proctree_c::loadNodes(char **buf_p)
{
	Com_Printf("loading nodes ... \n");

	Com_Parse(buf_p);	// skip '{'
	
	int count = Com_ParseInt(buf_p);

	for(int i=0; i<count; i++)
	{
		r_proctree_node_c * node = new r_proctree_node_c();
		
		// parse plane equation
		Com_Parse(buf_p);	// skip '('
		
		vec3_c normal;
		for(int i=0; i<3; i++)
		{
			normal[i] = Com_ParseFloat(buf_p);
		}						
		
		vec_t dist = -Com_ParseFloat(buf_p);
			
		Com_Parse(buf_p);	// skip ')'
			
		node->plane.set(normal, dist); 
		
		// parse children
		node->children[0] = Com_ParseInt(buf_p);
		node->children[1] = Com_ParseInt(buf_p);
		
		//ri.Com_Printf("node: %s ", node->plane.toString());
		//ri.Com_Printf("%i %i\n", node->children[0], node->children[1]);
		
		_nodes.push_back(node);
	}
	
	Com_Parse(buf_p);	// skip '}'
}

void	r_proctree_c::loadInterAreaPortals(char **buf_p)
{
	Com_Printf("loading inter area portals ... \n");

	Com_Parse(buf_p, true);	// skip '{'
	
	Com_ParseInt(buf_p, false);	// skip areas num
	int areaportals_num = Com_ParseInt(buf_p, false);
	
	//cm_areas = std::vector<carea_t>(areas_num);
	
	for(int i=0; i<areaportals_num; i++)
	{
		int points_num = Com_ParseInt(buf_p, true);
		std::vector<vec3_c> points(points_num);
		
		int areas[2];
		areas[SIDE_FRONT] = Com_ParseInt(buf_p, false);
		areas[SIDE_BACK] = Com_ParseInt(buf_p, false);
		
		// parse points
		for(int j=0; j<points_num; j++)
		{
			points[j] = Com_ParseVec3(buf_p);
		}
		
		r_areaportal_c* ap = new r_areaportal_c(points, areas);
		
		_areas[areas[SIDE_FRONT]]->areaportals.push_back(ap);
		_areas[areas[SIDE_BACK]]->areaportals.push_back(ap);
		_areaportals.push_back(ap);
	}
	
	Com_Parse(buf_p, true);	// skip '}'
}



r_proc_model_c::r_proc_model_c(const std::string &name)
:r_model_c(name, NULL, 0, MOD_PROC)
{
	ri.Com_DPrintf("loading '%s' ...\n", getName());
}

r_proc_model_c::~r_proc_model_c()
{
	for(std::vector<r_surface_c*>::iterator ir = _surfaces.begin(); ir != _surfaces.end(); ir++)
	{
		delete *ir;
	}
	
	_surfaces.clear();
}


void	r_proc_model_c::load(char **buf_p)
{
	//
	// parse and load the model data
	//
	int surfaces_num = Com_ParseInt(buf_p);
	
	for(int i=0; i<surfaces_num; i++)
	{
		Com_Parse(buf_p);	// skip '{'
	
		//
		// setup shader
		//
		std::string shadername = Com_Parse(buf_p);
				
		r_model_shader_c *shaderref = new r_model_shader_c(shadername, R_RegisterShader(shadername), X_SURF_NONE, X_CONT_NONE);
		
		_shaders.push_back(shaderref);
		
		
		//
		// setup mesh
		//
		r_mesh_c *mesh = new r_mesh_c();
		
		// parse numbers
		int vertexes_num = Com_ParseInt(buf_p);
		int indexes_num = Com_ParseInt(buf_p);
		
		mesh->fillVertexes(vertexes_num);
		
		// parse vertexes
		for(int j=0; j<vertexes_num; j++)
		{
			Com_Parse(buf_p);	// skip '('
			
			mesh->vertexes[j][0] = Com_ParseFloat(buf_p);
			mesh->vertexes[j][1] = Com_ParseFloat(buf_p);
			mesh->vertexes[j][2] = Com_ParseFloat(buf_p);
						
			mesh->texcoords[j][0] = Com_ParseFloat(buf_p);
			mesh->texcoords[j][1] = Com_ParseFloat(buf_p);
			
			mesh->normals[j][0] = Com_ParseFloat(buf_p);
			mesh->normals[j][1] = Com_ParseFloat(buf_p);
			mesh->normals[j][2] = Com_ParseFloat(buf_p);		
			
			Com_Parse(buf_p);	// skip ')'
			
			//Com_Printf("%s ", vertex->position.toString());
			//Com_Printf("%s ", vertex->texcoord.toString());
			//Com_Printf("%s\n", vertex->normal.toString());
		}
		
		// parse indices
		mesh->indexes = std::vector<index_t>(indexes_num);
		
		for(int j=0; j<indexes_num; j++)
		{
			mesh->indexes[j] = Com_ParseInt(buf_p);
			
			//Com_Printf("%i\n", *index);
		}
		
		//ri.Com_Printf("indices %i\n", mesh->indexes_num);
		
		
		// build triangle neighbours
		//mesh->neighbours = new int[mesh->indexes_num];
		//RB_BuildTriangleNeighbours(mesh->neighbours, mesh->indexes, mesh->indexes_num);
		
		
		_meshes.push_back(mesh);
		
		
		//
		// setup surface
		//
		r_surface_c *surf = new r_surface_c();
		
		surf->setShaderRef(shaderref);
		surf->setMesh(mesh);
		
		_surfaces.push_back(surf);
		
		char *token = Com_Parse(buf_p);	// skip '}'
		
		if(token[0] != '}')
			ri.Com_Error(ERR_DROP, "r_proc_model_c::load: model '%s' has bad end %s", getName(), token);
	}
	
	Com_Parse(buf_p);	// skip '}'
}

void	r_proc_model_c::addModelToList(r_entity_c *ent)
{
	if(r_frustum.cull(ent->getShared().origin, _bbox.radius(), FRUSTUM_CLIPALL))
	{
		c_entities--;
		return;
	}

	for(std::vector<r_surface_c*>::const_iterator ir = _surfaces.begin(); ir != _surfaces.end(); ++ir)
	{
		r_surface_c *surf = *ir;
		
		if(!surf->getMesh())
		{
			//ri.Com_DPrintf ("r_bsp_model_c::addSurfaceToList: surface of type %i has no mesh\n", surf->facetype);
			continue;
		}
	
		if(!surf->getShader())
		{
			ri.Com_DPrintf("r_proc_model_c::addModelToList: surface has no shader\n");
			continue;
		}	
		
		if(!r_showinvisible->getValue() && surf->getShader()->hasFlags(SHADER_NODRAW))
			continue;
			
		if(r_envmap && surf->getShader()->hasFlags(SHADER_NOENVMAP))
			continue;
	
		RB_AddCommand(ent, this, surf->getMesh(), surf->getShader(), NULL, NULL, -1, r_origin.distance(ent->getShared().origin));
	
		if(r_lighting->getValue() == 1)
		{
			for(std::vector<std::vector<r_light_c> >::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
			{
				std::vector<r_light_c>& lights = *ir;
			
				for(std::vector<r_light_c>::iterator ir = lights.begin(); ir != lights.end(); ++ir)
				{
					r_light_c& light = *ir;
					
					if(!light.isVisible())
						continue;
			
					if(light.getShared().radius_bbox.intersect(ent->getShared().origin, surf->getMesh()->bbox.radius()))
						RB_AddCommand(ent, this, surf->getMesh(), surf->getShader(), &light, NULL, -1, 0);
				}
			}
		}
	}
}

void	r_proc_model_c::draw(const r_command_t *cmd, r_render_type_e type)
{
	if(!cmd)
		return;
				
	RB_SetupModelviewMatrix(cmd->getEntity()->getTransform());

	RB_RenderCommand(cmd, type);
}

/*
void	r_proc_model_c::setupMeshes()
{
	_bbox.clear();
	
	for(std::vector<r_surface_c*>::const_iterator ir = _surfaces.begin(); ir != _surfaces.end(); ++ir)
	{
		r_surface_c *surf = *ir;
		
		if(!surf->getMesh())
			continue;
		
		surf->getMesh()->calcTangentSpaces();
//		surf->getMesh()->calcEdges();
		surf->getMesh()->createBBoxFromVertexes();
		
		_bbox.mergeWith(surf->getMesh()->bbox);
	}
}
*/


void	r_proc_model_c::setupVBO()
{
	if(gl_config.arb_vertex_buffer_object)
	{
		// count vertices and indices
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
		
		if(!vertexes_num || !indexes_num)
		{
			_vbo_array_buffer = 0;
			_vbo_element_array_buffer = 0;
			return;
		}
		
		xglGenBuffersARB(1, &_vbo_array_buffer);		RB_CheckForError();
		xglGenBuffersARB(1, &_vbo_element_array_buffer);	RB_CheckForError();
	
		
		uint_t		indexes_size = indexes_num * sizeof(index_t);
		byte*		indexes = (byte*)Com_Alloc(indexes_size);
		uint_t		indexes_ofs = 0;
		
		uint_t		data_size = vertexes_num * (sizeof(vec3_c)*4 + sizeof(vec2_c));
		byte*		data = (byte*)Com_Alloc(data_size);
		uint_t		data_ofs = 0;
		
						
		// create vbo offsets
		for(std::vector<r_surface_c*>::const_iterator ir = _surfaces.begin(); ir != _surfaces.end(); ir++)
		{	
			r_surface_c *surf = *ir;			
			
			if(surf->getMesh())
			{
				surf->getMesh()->vbo_array_buffer = _vbo_array_buffer;
				surf->getMesh()->vbo_element_array_buffer = _vbo_element_array_buffer;
			
				
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

