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
// xreal --------------------------------------------------------------------
#include "cg_local.h"



void	CG_InitDynamics()
{
	cg_ode_world = new d_world_c();
	
	// apply real world gravity by default
	vec3_c gravity(0.0f, 0.0f, (-9.81 * (32.0/1.0)) * cg_gravity->getValue());

	cg_ode_world->setGravity(gravity);
	cg_ode_world->setCFM(1e-5);
//	cg_ode_world->setAutoDisableFlag(true);
//	cg_ode_world->setContactMaxCorrectingVel(0.1);
//	cg_ode_world->setContactSurfaceLayer(0.001);
	
	cg_ode_space = new d_simple_space_c();
//	cg_ode_space = new d_hash_space_c();
	
	cg_ode_contact_group = new d_joint_group_c();
}

void	CG_ShutdownDynamics()
{
	delete cg_ode_contact_group;
	cg_ode_contact_group = NULL;

	delete cg_ode_space;
	cg_ode_space = NULL;

	delete cg_ode_world;
	cg_ode_world = NULL;	
}

#if 0
static void	CG_NearCallback(void *data, dGeomID o1, dGeomID o2)
{
	if(dGeomIsSpace(o1) || dGeomIsSpace(o2))
	{
		// colliding a space with something
 		dSpaceCollide2(o1, o2, data, &G_NearCallback);
		
		// collide all geoms internal to the space(s)
//		if(dGeomIsSpace(o1))
//			dSpaceCollide((dxSpace*)o1, data, &G_NearCallback);
		
//		if(dGeomIsSpace(o2))
//			dSpaceCollide((dxSpace*)o2, data, &G_NearCallback);
	}
	else
	{
		// colliding two non-space geoms, so generate contact
		// points between o1 and o2
		dBodyID b1 = dGeomGetBody(o1);
		dBodyID b2 = dGeomGetBody(o2);
	
		// exit without doing anything if the two bodies are connected by a joint
		if(b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
			return;
		
		// take care of different collision masks
//		if(!(dGeomGetCollideBits(o1) & dGeomGetCollideBits(o2)))
//			return;
		
		// take care of disabled bodies
		if(b1 && !b2 && !dBodyIsEnabled(b1))
			return;	// b1 is disabled and collides with no-body
	
		if(b2 && !b1 && !dBodyIsEnabled(b2))
			return; // b2 is disabled and collides with no-body
		
		if(b1 && b2 && !dBodyIsEnabled(b1) && !dBodyIsEnabled(b2))
			return; // both b1 and b2 are disabled
		

		// check for rays
//		if(dGeomGetClass(o1) == dRayClass || dGeomGetClass(o2) == dRayClass)
//		{
//			//TODO		
//			return;
//		}

		const int	contacts_max = 16;
		dContact	contacts[contacts_max]; 
		if(int contacts_num = dCollide(o1, o2, contacts_max, &contacts[0].geom, sizeof(dContact)))
		{
			// sort contacts by penetration depth
			//qsort(contacts, contacts_num, sizeof(dContact), G_ContactSortFunc);
		
			// two entities have touched so run their touch functions
			g_geom_info_c *g1 = (g_geom_info_c*)dGeomGetData(o1);
			g_geom_info_c *g2 = (g_geom_info_c*)dGeomGetData(o2);
			
			g_entity_c *e1 = g1->getEntity();
			g_entity_c *e2 = g2->getEntity();
		
			if(e1 && e2)
			{
				// test if both pointers are valid entities
				G_GetNumForEntity(e1);
				G_GetNumForEntity(e2);
			
				// set hit world surface if any
				csurface_c *surf = NULL;
				
				if(e1 == g_world)
					surf = g1->getSurface();
				
				else if(e2 == g_world)
					surf = g2->getSurface();
			
				// set collision plane
				vec3_c point = contacts[0].geom.pos;
				vec3_c normal = contacts[0].geom.normal;
								
				cplane_c plane(normal, -normal.dotProduct(point));
				
				if(!e1->touch(e2, plane, surf))
					return;
			
				if(!e2->touch(e1, plane, surf))
					return;
			}

			for(int i=0; i<contacts_num; i++)
			{
				contacts[i].surface.mode = 0;//dContactBounce | dContactSoftCFM;
				contacts[i].surface.mu = dInfinity;
				contacts[i].surface.mu2 = 0;
				contacts[i].surface.bounce = 0.1;
				contacts[i].surface.bounce_vel = 0.1;
				contacts[i].surface.soft_cfm = 0.0001;
		
				dJointID c = dJointCreateContact(g_ode_world->getId(), g_ode_contact_group->getId(), &contacts[i]);
				dJointAttach(c, b1, b2);
			}
		}
		
	}// dGeomIsSpace
}	

void	G_RunDynamics(float step_size)
{
	g_ode_space->collide(NULL, G_NearCallback);
	
//	g_ode_world->step(step_size);
//	g_ode_world->stepFast(step_size, 10);
	g_ode_world->stepQuick(step_size);
	
	g_ode_contact_group->empty();
}


static trace_t			g_ray_trace;
static std::vector<dContact>	g_ray_contacts;

static void	G_RayCallback(void *data, dGeomID o1, dGeomID o2)
{
	if(dGeomIsSpace(o1) || dGeomIsSpace(o2))
	{
		// colliding a space with something
 		dSpaceCollide2(o1, o2, data, &G_RayCallback);
		
		// collide all geoms internal to the space(s)
//		if(dGeomIsSpace(o1))
//			dSpaceCollide((dxSpace*)o1, data, &G_RayCallback);
		
//		if(dGeomIsSpace(o2))
//			dSpaceCollide((dxSpace*)o2, data, &G_RayCallback);
	}
	else
	{
		// check for rays
		if(dGeomGetClass(o1) != dRayClass && dGeomGetClass(o2) != dRayClass)
			return;
	
		int		contacts_max = 16;
		dContact	contacts[contacts_max];
		if(int contacts_num = dCollide(o1, o2, contacts_max, &contacts[0].geom, sizeof(dContact)))
		{
			g_ray_contacts.resize(g_ray_contacts.size() + contacts_num);
		
			for(int i=0; i<contacts_num; i++)
			{
				g_ray_contacts[g_ray_contacts.size() + i] = contacts[i];
			}
		}
	}
}

int	G_SortByContactGeomDepthFunc(void const *a, void const *b)
{
	dContact* contact_a = (dContact*)a;
	dContact* contact_b = (dContact*)b;

	vec_t depth_a = contact_a->geom.depth;
	vec_t depth_b = contact_b->geom.depth;
	
	if(depth_a < depth_b)
		return 1;
	
	else if(depth_a > depth_b)
		return -1;
		
	else
		return 0;
}

trace_t	G_RayTrace(const vec3_c &start, const vec3_c &end)
{
	return G_RayTrace(start, end - start, start.distance(end));
}


trace_t	G_RayTrace(const vec3_c &start, const vec3_c &dir, vec_t length)
{
	g_ray_contacts.clear();

	// create default trace
	g_ray_trace.nohit		= true;
	g_ray_trace.allsolid		= false;
	g_ray_trace.startsolid		= false;
	g_ray_trace.depth		= 1.0;
	g_ray_trace.pos			= start + dir * length;
	g_ray_trace.plane.set(0, 0, 1, 0);
	g_ray_trace.surface		= NULL;		
	g_ray_trace.pos_contents	= X_CONT_NONE;
	
	g_ray_trace.ent		= NULL;

	// create ray
	d_ray_c ray(g_ode_space->getId(), start, dir, length);
	
	// fire ray against everything
	g_ode_space->collide(NULL, G_RayCallback);
	
	if(g_ray_contacts.size())
	{
		//gi.Com_Printf("G_RayTrace: %i contacts\n", g_ray_contacts.size());
		
		// find nearest contact
		/*
		vec_t dist_old = 0;
	
		std::deque<dContact>::const_iterator nearest = g_ray_contacts.begin();
	
		for(std::deque<dContact>::const_iterator ir = g_ray_contacts.begin(); ir != g_ray_contacts.end(); ++ir)
		{
			const dContact& contact = *ir;
	
			if(contact.geom.depth < dist_old)
			{
				nearest = ir;
			}
		
			dist_old = contact.geom.depth;
		}
	
		const dContact& dnearest = *nearest;
		*/
		
		qsort(&g_ray_contacts[0], g_ray_contacts.size(), sizeof(dContact), G_SortByContactGeomDepthFunc);
		
		const dContact& dnearest = g_ray_contacts[0];
		
		// we hit something!
		g_ray_trace.nohit = false;
	
		// calc endpos
		//g_ray_trace.endpos = dnearest.geom.pos;
		g_ray_trace.pos = start + dir * dnearest.geom.depth;
		
		g_ray_trace.depth = dnearest.geom.depth;
	
		// calc plane
		vec3_c normal = dnearest.geom.normal;
		vec3_c x = dnearest.geom.pos;
	
		g_ray_trace.plane.set(normal, normal.dotProduct(x));
	}
	
	return g_ray_trace;
}


/*
=================
SV_SetModel

Also sets mins and maxs for inline bmodels
=================
*/
cmodel_c*	G_SetModel(g_entity_c *ent, const std::string &name)
{
	if(name.empty())
		Com_Error(ERR_DROP, "G_SetModel: empty name");
	
//	Com_Printf("G_SetModel: '%s'\n", name.c_str());

	ent->_s.index_model = gi.SV_ModelIndex(name);
	
	cmodel_c* model = gi.CM_RegisterModel(name);
		
//	ent->_geom = new d_trimesh_c(g_ode_space->getId(), model->vertexes, model->indexes);
//	ent->_geom->setBody(ent->_body->getId());
//	ent->_geom->setData(ent);
		
	ent->_r.bbox = model->getBBox();
	ent->_r.size = ent->_r.bbox.size();
	
	return model;
}

void		G_SetWorldModel(g_entity_c *ent, const std::string &name)
{
	if(name.empty())
		Com_Error(ERR_DROP, "G_SetWorldModel: empty name");
	
//	Com_Printf("G_SetWorldModel: '%s'\n", name.c_str());
	
	ent->_s.index_model = gi.SV_ModelIndex(name);
	
	cmodel_c* model = gi.CM_RegisterModel(name);
		
#if 0
	if(!model->surfaces.size())
	{
		Com_Error(ERR_DROP, "G_SetWorldModel: model has no surfaces");
		return;
	}

	ent->_space = new d_simple_space_c(g_ode_space->getId());
	
	for(std::vector<csurface_c>::iterator ir = model->surfaces.begin(); ir != model->surfaces.end(); ++ir)
	{
		csurface_c& surf = *ir;
		
		//if(surf.hasFlags(/*SURF_NOIMPACT |*/ X_SURF_NONSOLID))
		//	continue;
		
		g_geom_info_c *geom_info = new g_geom_info_c(ent, model, &surf);
		
		d_geom_c *geom = new d_trimesh_c(ent->_space->getId(), surf.getVertexes(), surf.getIndexes());
		geom->setData(geom_info);
	
		ent->_geoms.insert(std::make_pair(geom, geom_info));
	}
#else
	if(model->vertexes.size() && model->indexes.size())
	{
		g_geom_info_c *geom_info = new g_geom_info_c(ent, model, NULL);
		d_geom_c *geom = new d_trimesh_c(g_world->_space->getId(), model->vertexes, model->indexes);
		geom->setData(geom_info);
		ent->_geoms.insert(std::make_pair(geom, geom_info));
	}
#endif
		
	ent->_r.bbox = model->getBBox();
	ent->_r.size = ent->_r.bbox.size();
}
#endif
