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



void	r_areaportal_c::adjustFrustum()
{
	// left, right, down, up
	if(plane.onSide(r_origin) == SIDE_BACK)
	{
		// build frustum's planes clockwise
		for(int i=0; i<4; i++)
		{
			frustum[i].fromThreePointForm(points[(i+1)&3], r_origin, points[i]);
//			frustum[i]._dist = -frustum[i]._dist;
			frustum[i]._type = PLANE_ANYZ;
		}
	}
	else
	{
		// build frustum's planes counter clockwise
		for(int i=0; i<4; i++)
		{
			frustum[i].fromThreePointForm(points[i], r_origin, points[(i+1)&3]);
//			frustum[i]._dist = -frustum[i]._dist;
			frustum[i]._type = PLANE_ANYZ;
		}
	}
	
#if 0
	vec3_c x;
	
	// near
	x = r_origin;
	frustum[4]._normal	= bbox.origin() - r_origin;
	frustum[4]._dist	= x.dotProduct(frustum[4]._normal);
	frustum[4]._type	= PLANE_ANYZ;
	
	// far
//	x = bbox.origin();
//	frustum[5]._normal	= r_origin - bbox.origin();
//	frustum[5]._dist	= x.dotProduct(frustum[5]._normal);
//	frustum[5].normalize();
//	frustum[5]._type	= PLANE_ANYZ;
	frustum[5] = r_frustum[5];
#else
	frustum[4] = r_frustum[4];
	frustum[5] = r_frustum[5];
#endif
}
	
void	r_areaportal_c::draw()
{
	if(!r_showareaportals->getValue())
		return;

	//xglDisable(GL_DEPTH_TEST);
	xglPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	if(plane.distance(r_origin) >= 0)
		xglColor4fv(color_red);
	else
		xglColor4fv(color_blue);
	
	//xglBegin(GL_LINE_LOOP);
	//xglBegin(GL_LINES);
	xglBegin(GL_QUADS);
	
	for(unsigned int i=0; i<points.size(); i++)
		xglVertex3fv(points[i]);

	xglEnd();
	
	//xglEnable(GL_DEPTH_TEST);
	xglPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
	

void	r_proctree_c::update()
{
	// current viewcluster
	if(!(r_newrefdef.rdflags & RDF_NOWORLDMODEL)/* && !r_mirrorview*/)
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
	
	// cache lights
	for(std::map<int, r_light_c>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
	{
		r_light_c& light = ir->second;
		
		if(light.needsUpdate())
		{
			light.setAreaNum(_areas.size());
		
#if 0
			for(uint_t areanum=0; areanum<_areas.size(); areanum++)
			{
				r_proctree_area_c *area = _areas[areanum];
				
				for(std::vector<r_surface_c*>::const_iterator ir2 = area->model->_surfaces.begin(); ir2 != area->model->_surfaces.end(); ++ir2)
				{
					r_surface_c* surf = *ir2;
						
					if(!surf->getMesh()->bbox.intersect(light.getShared().radius_bbox))
						continue;
						
					light.addSurface(areanum, surf);
				}
			}
#else			
			int area = pointInArea(light.getShared().origin);
				
			if(area != -1)
				litArea_r(area, &light);
#endif
			
			light.needsUpdate(false);
		}
	}
}

void	r_proctree_c::draw()
{
	// clear old lights
	/*
	for(unsigned int i=0; i<_areas.size(); i++)
	{
		r_proctree_area_c* area = _areas[i];
			
		area->lights.clear();
	}
	*/
	
	if(r_lighting->getValue())	// don't draw lights if outside of the map
	{
		/*					
		// create new dynamic lights
		for(std::map<int, r_light_c>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c& light = ir->second;
			
			if(R_CullBBox(r_frustum, light.getShared().radius_bbox))
				continue;
			
			_visframecount++;
			
			int area = pointInArea(light.getShared().origin);
				
			if(area != -1)
				litArea_r(area, &light);
		}
		*/
	}
	
	_visframecount++;
	
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
		
			area->model->addModelToList(&r_world_entity);
		}
	}
	else
	{
		drawArea_r(_sourcearea, r_frustum, FRUSTUM_CLIPALL);
	}
}


void	r_proctree_c::drawArea_r(int areanum, const r_frustum_t frustum, int clipflags)
{
	/*
	if(r_newrefdef.areabits)
	{
		if(!(r_newrefdef.areabits[areanum>>3] & (1<<(areanum&7))))
			return;		// not visible
	}
	*/
	
	r_proctree_area_c *area = _areas[areanum];
	
	if(area->visframe == r_framecount)
		return;
	
	if(r_cull->getValue() && clipflags)
	{
		for(int i=0; i<FRUSTUM_PLANES; i++)
		{
			if(!(clipflags & (1<<i)))
				continue;	// don't need to clip against it

			plane_side_e clipped = r_frustum[i].onSide(area->model->getBBox());
			
			if(clipped == SIDE_BACK)
				return;
				
			else if(clipped == SIDE_FRONT)
				clipflags &= ~(1<<i);	// node is entirely on screen
		}
	}
	
	area->visframe = r_framecount;
	r_leafs_counter++;

	for(std::vector<r_surface_c*>::const_iterator ir = area->model->_surfaces.begin(); ir != area->model->_surfaces.end(); ++ir)
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
				
		if(R_CullBBox(r_frustum, surf->getMesh()->bbox, clipflags))
			continue;
		
		surf->setFrameCount();
	
		RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), NULL, NULL, -1);
	}
	
	
	if(r_lighting->getValue() == 1)
	{
		for(std::vector<r_light_c*>::const_iterator ir = area->lights.begin(); ir != area->lights.end(); ++ir)
		{
			r_light_c *light = *ir;
			
			if(!light)
				continue;
				
			if(R_CullBBox(r_frustum, light->getShared().radius_bbox, clipflags))
				continue;
					
			const std::map<const r_surface_c*, std::vector<index_t> >& light_areasurfaces = light->getAreaSurfaces(areanum);
			
			for(std::map<const r_surface_c*, std::vector<index_t> >::const_iterator ir2 = light_areasurfaces.begin(); ir2 != light_areasurfaces.end(); ++ir2)
			{
				const r_surface_c* surf = ir2->first;

				if(surf->getFrameCount() != r_framecount)
					continue;
					
				RB_AddCommand(&r_world_entity, area->model, surf->getMesh(), surf->getShader(), light, (std::vector<index_t>*)&ir2->second, -1);
			}
		}
	}
	
	for(std::vector<r_areaportal_c*>::const_iterator ir = area->areaportals.begin(); ir != area->areaportals.end(); ++ir)
	{
		r_areaportal_c* iap = *ir;
		
		if(iap->visframe != r_framecount)
		{
			iap->visframe = r_framecount;
		
			if(r_cullportals->getValue() && !R_CullBBox(frustum, iap->bbox, clipflags))
			{
				for(int i=0; i<2; i++)
				{
					if(iap->areas[i] == areanum)
						continue;
		
					// never go back to the source area
					if(iap->areas[i] == _sourcearea)
						continue;
						
					iap->adjustFrustum();
					
					drawArea_r(iap->areas[i], iap->frustum, clipflags);
				}
			}
		}
	}
}

void	r_proctree_c::litArea_r(int areanum, r_light_c *light)
{
	//ri.Com_Printf("r_proctree_c::litArea_r %i\n", areanum);

	r_proctree_area_c *area = _areas[areanum];
		
	std::vector<r_light_c*>::iterator ir = find(area->lights.begin(), area->lights.end(), static_cast<r_light_c*>(light));
	if(ir != area->lights.end())
		return;
	else
		area->lights.push_back(light);
		
	
	for(std::vector<r_surface_c*>::const_iterator ir = area->model->_surfaces.begin(); ir != area->model->_surfaces.end(); ++ir)
	{
		r_surface_c *surf = *ir;
		
		if(!surf->getMesh()->bbox.intersect(light->getShared().radius_bbox))
			continue;
			
		if(light->hasSurface(areanum, surf))
			continue;
						
		light->addSurface(areanum, surf, false);
	}
		
	for(std::vector<r_areaportal_c*>::const_iterator ir = area->areaportals.begin(); ir != area->areaportals.end(); ++ir)
	{
		r_areaportal_c* iap = *ir;
		
		if(iap->bbox.intersect(light->getShared().radius_bbox))
		{
			for(int i=0; i<2; i++)
			{			
				litArea_r(iap->areas[i], light);
			}
		}
	}
}

int	r_proctree_c::pointInArea_r(const vec3_c &p, int num)
{
	if(num < 0 || num > (int)_nodes.size())
	{
		ri.Com_Error(ERR_DROP, "r_proctree_c::pointInArea_r: bad num %i", num);
	}
	
	r_proctree_node_c *node = _nodes[num];
		
	vec_t d = node->plane.distance(p);
		
	if(d >= 0)
		num = node->children[SIDE_FRONT];
	else
		num = node->children[SIDE_BACK];
		
	if(num < 0)
		return -1 -num;
			
	if(num == 0)
		return -1;
	
	return pointInArea_r(p, num);
}

int	r_proctree_c::pointInArea(const vec3_c &p)
{
	if(!_nodes.size())
	{
		//ri.Com_Error(ERR_DROP, "r_proctree_c::pointInArea: bad tree");
		return 0;
	}
	
	int area = pointInArea_r(p, 0);
	
	//ri.Com_Printf("r_proc_tree_c::pointInArea: %i\n", area);
	
	return area;
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
		r_areaportal_c* ap = new r_areaportal_c();
		
		int points_num = Com_ParseInt(buf_p, true);
		
		ap->areas[SIDE_FRONT] = Com_ParseInt(buf_p, false);
		ap->areas[SIDE_BACK] = Com_ParseInt(buf_p, false);
		
		_areas[ap->areas[SIDE_FRONT]]->areaportals.push_back(ap);
		_areas[ap->areas[SIDE_BACK]]->areaportals.push_back(ap);
		
		ap->bbox.clear();
	
		// parse points
		for(int j=0; j<points_num; j++)
		{
			vec3_c p = Com_ParseVec3(buf_p);
		
			ap->bbox.addPoint(p);
			
			ap->points.push_back(p);
		}
		
		if(points_num >= 3)
		{		
			ap->plane.fromThreePointForm(ap->points[0], ap->points[1], ap->points[2]);
		}
		else
		{
			ri.Com_Error(ERR_DROP, "r_proctree_c::loadInterAreaPortals: bad points num %i", points_num);
		}
		
		//Com_Printf("iap: %s ", ap..toString());
		//Com_Printf("%i %i %i \n", points_num, ap.areas[SIDE_FRONT], ap.areas[SIDE_BACK]);
		
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
	
	_bbox.clear();
	
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
		
		
		// create bounding box on the fly	
		mesh->bbox.clear();
		for(std::vector<vec3_c>::const_iterator ir = mesh->vertexes.begin(); ir != mesh->vertexes.end(); ir++)
		{
			mesh->bbox.addPoint(*ir);
			_bbox.addPoint(*ir);
		}
		
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
	
	
	
	//
	// setup VBO support
	//
	if(gl_config.arb_vertex_buffer_object)
	{
		for(std::vector<r_surface_c*>::const_iterator ir = _surfaces.begin(); ir != _surfaces.end(); ir++)
		{	
			r_surface_c *surf = *ir;
			
			if(surf->getMesh())
				RB_PushMesh(surf->getMesh(), true, true);
		}
	}
}

void	r_proc_model_c::addModelToList(r_entity_c *ent)
{
	if(R_CullBSphere(r_frustum, ent->getShared().origin, _bbox.radius(), FRUSTUM_CLIPALL))
		return;

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
				
		//if(R_CullBBox(r_frustum, surf->getMesh()->bbox))
		//	continue;
	
		RB_AddCommand(ent, this, surf->getMesh(), surf->getShader(), NULL, NULL, -1);
	}
	
	/*
	if(r_lighting->getValue() == 1)
	{
		for(std::map<int, r_light_c>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c& light = ir->second;
			
			if(R_CullBBox(r_frustum, light.getShared().radius_bbox))
					continue;
			
			for(std::vector<r_surface_c*>::const_iterator ir2 = _surfaces.begin(); ir2 != _surfaces.end(); ++ir2)
			{
				r_surface_c* surf = *ir2;
				
				if(surf->culled)
					continue;				
						
				if(!surf->getMesh()->bbox.intersect(light.getShared().radius_bbox))
					continue;
					
				//if(light.hasSurface(surf))
				//	continue;
								
				RB_AddCommand(ent, this, surf->getMesh(), surf->getShader(), &light, surf, -1);
			}
			
			//ri.Com_Printf("light in area %i intersects with %i surfaces\n", areanum, light->surfaces.size());
		}
	}
	*/
}

void	r_proc_model_c::draw(const r_command_t *cmd, r_render_type_e type)
{
	if(!cmd)
		return;

	if(type == RENDER_TYPE_SHADOWING)
		return;
				
	RB_SetupModelviewMatrix(cmd->getEntity()->getTransform());

	RB_RenderCommand(cmd, type);
}

void	r_proc_model_c::setupMeshes()
{	
	for(std::vector<r_surface_c*>::const_iterator ir = _surfaces.begin(); ir != _surfaces.end(); ir++)
	{
		r_surface_c *surf = *ir;
		
		if(!surf->getMesh())
			continue;
		
		surf->getMesh()->calcTangentSpaces();
//		surf->getMesh()->calcEdges();
	}
}



