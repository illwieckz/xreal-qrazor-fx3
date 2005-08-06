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
#include "g_local.h"
#include "g_entity.h"

/*


pushmove objects do not obey gravity, and do not interact with each other or trigger fields, but block normal movement and push normal objects when they move.

onground is set for toss objects when they come to a complete rest.  it is set for steping or walking objects 

doors, plats, etc are SOLID_BSP, and MOVETYPE_PUSH
bonus items are SOLID_TRIGGER touch, and MOVETYPE_TOSS
corpses are SOLID_NOT and MOVETYPE_TOSS
crates are SOLID_BBOX and MOVETYPE_TOSS
walking monsters are SOLID_SLIDEBOX and MOVETYPE_STEP
flying/floating monsters are SOLID_SLIDEBOX and MOVETYPE_FLY

solid_edge items only clip against bsp models.

*/













/*
============
SV_FlyMove

The basic solid body movement clip that slides along multiple planes
Returns the clipflags if the velocity was modified (hit something solid)
1 = floor
2 = wall / step
4 = dead stop
============
*/

/*
#define	MAX_CLIP_PLANES	5
static int	SV_FlyMove(g_entity_c *ent, float time, int mask)
{
	g_entity_c		*hit;
	int			bumpcount, numbumps;
	vec3_c		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity, original_velocity, new_velocity;
	int			i, j;
	trace_t		trace;
	vec3_t		end;
	float		time_left;
	int			blocked;
	
	numbumps = 4;
	
	blocked = 0;
	Vector3_Copy (ent->_velocity, original_velocity);
	Vector3_Copy (ent->_velocity, primal_velocity);
	numplanes = 0;
	
	time_left = time;

	ent->_groundentity = NULL;
	for (bumpcount=0 ; bumpcount<numbumps ; bumpcount++)
	{
		for (i=0 ; i<3 ; i++)
			end[i] = ent->_s.origin[i] + time_left * ent->_velocity[i];

		trace = trap_SV_Trace (ent->_s.origin, ent->_r.bbox, end, ent, mask);

		if (trace.allsolid)
		{	// entity is trapped in another solid
			Vector3_Copy (vec3_origin, ent->_velocity);
			return 3;
		}

		if (trace.fraction > 0)
		{	// actually covered some distance
			Vector3_Copy (trace.endpos, ent->_s.origin);
			Vector3_Copy (ent->_velocity, original_velocity);
			numplanes = 0;
		}

		if (trace.fraction == 1)
			 break;		// moved the entire distance

		hit = (g_entity_c*)trace.ent;

		if (trace.plane._normal[2] > 0.7)
		{
			blocked |= 1;		// floor
			if ( hit->_r.solid == SOLID_BSP)
			{
				ent->_groundentity = hit;
				ent->_groundentity_linkcount = hit->_r.linkcount;
			}
		}
		if (!trace.plane._normal[2])
		{
			blocked |= 2;		// step
		}

		//
		// run the impact function
		//
		G_Impact(ent, &trace);
		if(!ent->_r.inuse)
			break;		// removed by the impact function

		
		time_left -= time_left * trace.fraction;
		
		// cliped to another plane
		if (numplanes >= MAX_CLIP_PLANES)
		{	// this shouldn't really happen
			Vector3_Copy (vec3_origin, ent->_velocity);
			return 3;
		}

		Vector3_Copy (trace.plane._normal, planes[numplanes]);
		numplanes++;

		
		//
		// modify original_velocity so it parallels all of the clip planes
		//
		for (i=0 ; i<numplanes ; i++)
		{
			ClipVelocity (original_velocity, planes[i], new_velocity, 1);

			for (j=0 ; j<numplanes ; j++)
				if ((j != i)  && !Vector3_Compare (planes[i], planes[j]))
				{
					if (Vector3_DotProduct (new_velocity, planes[j]) < 0)
						break;	// not ok
				}
			if (j == numplanes)
				break;
		}
		
		if (i != numplanes)
		{	// go along this plane
			Vector3_Copy (new_velocity, ent->_velocity);
		}
		else
		{	// go along the crease
			if (numplanes != 2)
			{
				//trap_dprintf ("clip velocity, numplanes == %i\n",numplanes);
				Vector3_Copy (vec3_origin, ent->_velocity);
				return 7;
			}
			dir.crossProduct(planes[0], planes[1]);
			d = Vector3_DotProduct (dir, ent->_velocity);
			Vector3_Scale (dir, d, ent->_velocity);
		}
		
		

		//
		// if original velocity is against the original velocity, stop dead
		// to avoid tiny occilations in sloping corners
		//
		if (Vector3_DotProduct (ent->_velocity, primal_velocity) <= 0)
		{
			Vector3_Copy (vec3_origin, ent->_velocity);
			return blocked;
		}
	}

	return blocked;
}
*/




/*
===============================================================================
				PUSHMOVE
===============================================================================
*/






/*
================
SV_Physics_Pusher

Bmodel objects don't interact with each other, but
push all box objects
================
*/
/*
static void	G_Physics_Pusher(g_entity_c *ent)
{
	vec3_t		move, amove;
	g_entity_c		*part, *mv;

	// if not a team captain, so movement will be handled elsewhere
	if(ent->_flags & FL_TEAMSLAVE)
		return;

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
//retry:
	pushed_p = pushed;
	for (part = ent ; part ; part=part->_teamchain)
	{
		if(part->_velocity[0] || part->_velocity[1] || part->_velocity[2] ||
			part->_avelocity[0] || part->_avelocity[1] || part->_avelocity[2]
			)
		{	// object is moving
			Vector3_Scale(part->_velocity, FRAMETIME, move);
			Vector3_Scale(part->_avelocity, FRAMETIME, amove);

			if(!G_Push(part, move, amove))
				break;	// move was blocked
		}
	}
	if(pushed_p > &pushed[MAX_ENTITIES])
		trap_Com_Error(ERR_FATAL, "pushed_p > &pushed[MAX_EDICTS], memory corrupted");

	if(part)
	{
		// the move failed, bump all nextthink times and back out moves
		for (mv = ent ; mv ; mv=mv->_teamchain)
		{
			if (mv->_nextthink > 0)
				mv->_nextthink += FRAMETIME;
		}

		// if the pusher has a "blocked" function, call it
		// otherwise, just stay in place until the obstacle is gone
		part->blocked(obstacle);
#if 0
		// if the pushed entity went away and the pusher is still there
		if (!obstacle->inuse && part->inuse)
			goto retry;
#endif
	}
	else
	{
		// the move succeeded, so call all think functions
		for (part = ent ; part ; part=part->_teamchain)
		{
			G_RunThink (part);
		}
	}
}
*/








/*
===============================================================================
				STEPPING MOVEMENT
===============================================================================
*/

/*
=============
SV_Physics_Step

Monsters freefall when they don't have a ground entity, otherwise
all movement is done with discrete steps.

This is also used for objects that have become still on the ground, but
will fall if the floor is pulled out from under them.
FIXME: is this true?
=============
*/

//FIXME: hacked in for E3 demo
/*
#define	sv_stopspeed		100
#define sv_friction			6
#define sv_waterfriction	1

static void 	G_AddRotationalFriction(g_entity_c *ent)
{
	int		n;
	float	adjustment;

	Vector3_MA (ent->_s.angles, FRAMETIME, ent->_avelocity, ent->_s.angles);
	adjustment = FRAMETIME * sv_stopspeed * sv_friction;
	for (n = 0; n < 3; n++)
	{
		if (ent->_avelocity[n] > 0)
		{
			ent->_avelocity[n] -= adjustment;
			if (ent->_avelocity[n] < 0)
				ent->_avelocity[n] = 0;
		}
		else
		{
			ent->_avelocity[n] += adjustment;
			if (ent->_avelocity[n] > 0)
				ent->_avelocity[n] = 0;
		}
	}
}
*/


/*
static void	G_Physics_Step(g_entity_c *ent)
{
	bool	wasonground;
	bool	hitsound = false;
	float		speed, newspeed, control;
	float		friction;
	g_entity_c*	groundentity;
	int		mask;

	
	groundentity = ent->_groundentity;

	G_CheckVelocity (ent);

	if (groundentity)
		wasonground = true;
	else
		wasonground = false;
		
	if (ent->_avelocity[0] || ent->_avelocity[1] || ent->_avelocity[2])
		G_AddRotationalFriction (ent);

	// add gravity except:
	//   flying monsters
	//   swimming monsters who are in the water
	if (! wasonground)
	{
		if (!(ent->_flags & FL_FLY))
		{
			if (!((ent->_flags & FL_SWIM) && (ent->_waterlevel > 2)))
			{
				if (ent->_velocity[2] < sv_gravity->value*-0.1)
					hitsound = true;
				
				if (ent->_waterlevel == 0)
					G_AddGravity (ent);
			}
		}
	}

	// friction for flying monsters that have been given vertical velocity
	if ((ent->_flags & FL_FLY) && (ent->_velocity[2] != 0))
	{
		speed = fabs(ent->_velocity[2]);
		control = speed < sv_stopspeed ? sv_stopspeed : speed;
		friction = sv_friction/3;
		newspeed = speed - (FRAMETIME * control * friction);
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;
		ent->_velocity[2] *= newspeed;
	}

	// friction for flying monsters that have been given vertical velocity
	if ((ent->_flags & FL_SWIM) && (ent->_velocity[2] != 0))
	{
		speed = fabs(ent->_velocity[2]);
		control = speed < sv_stopspeed ? sv_stopspeed : speed;
		newspeed = speed - (FRAMETIME * control * sv_waterfriction * ent->_waterlevel);
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;
		ent->_velocity[2] *= newspeed;
	}

	if (ent->_velocity[2] || ent->_velocity[1] || ent->_velocity[0])
	{
		mask = MASK_SOLID;
		
		SV_FlyMove (ent, FRAMETIME, mask);

		trap_SV_LinkEdict (ent);
		G_TouchTriggers (ent);
		
		if (!ent->_r.inuse)
			return;

		if (ent->_groundentity)
			if (!wasonground)
				if (hitsound)
					trap_SV_StartSound (NULL, ent, 0, trap_SV_SoundIndex("world/land.wav"), 1, 1, 0);
	}

	// regular thinking
	G_RunThink (ent);
}
*/



#if defined(ODE)
void	G_InitDynamics()
{
	g_ode_world = new d_world_c();
	
	// apply real world gravity by default
	vec3_c gravity(0.0f, 0.0f, (-9.81 * 32.0) * g_gravity->getValue());

	g_ode_world->setGravity(gravity);
	g_ode_world->setCFM(1e-5);
	g_ode_world->setERP(0.4);
	
#if 1
	g_ode_world->setAutoDisableFlag(true);
	g_ode_world->setAutoDisableLinearThreshold(0.015);
	g_ode_world->setAutoDisableAngularThreshold(0.008);
	g_ode_world->setAutoDisableSteps(50);
//	g_ode_world->setAutoDisableTime(vec_t time);
#endif
	
//	g_ode_world->setContactMaxCorrectingVel(0.1);
//	g_ode_world->setContactSurfaceLayer(0.001);
	
	g_ode_space_toplevel = new d_simple_space_c();
//	g_ode_space_toplevel = new d_hash_space_c();
	
//	g_ode_space_world = new d_simple_space_c(g_ode_space_toplevel->getId());

//	g_ode_testplane = new d_plane_c(g_ode_space_toplevel->getId(), vec3_c(0.0, 0.0, 1.0), 64.0);
	
	g_ode_contact_group = new d_joint_group_c();
}

void	G_ShutdownDynamics()
{
	delete g_ode_contact_group;
	g_ode_contact_group = NULL;

	delete g_ode_space_toplevel;
	g_ode_space_toplevel = NULL;
	
//	delete g_ode_space_world;
//	g_ode_space_world = NULL;

	delete g_ode_world;
	g_ode_world = NULL;	
}


static int	G_SortByContactGeomDepthFunc(void const *a, void const *b)
{
	dContact* contact_a = (dContact*)a;
	dContact* contact_b = (dContact*)b;

	vec_t depth_a = contact_a->geom._depth;
	vec_t depth_b = contact_b->geom._depth;
	
	if(depth_a < depth_b)
		return 1;
	
	else if(depth_a > depth_b)
		return -1;
		
	else
		return 0;
}

static void	G_TopLevelCollisionCallback(void *data, dGeomID o1, dGeomID o2)
{
	if(dGeomIsSpace(o1) || dGeomIsSpace(o2))
	{
		// colliding a space with something
 		dSpaceCollide2(o1, o2, data, &G_TopLevelCollisionCallback);
		
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

		std::vector<dContact> contacts;
		if(int contacts_num = dCollide(o1, o2, 32, contacts))
		{
			// sort contacts by penetration depth
			//qsort(contacts, contacts_num, sizeof(dContact), G_SortByContactGeomDepthFunc);
		
			// two entities have touched so run their touch functions
			g_entity_c *e1 = (g_entity_c*)dGeomGetData(o1);
			g_entity_c *e2 = (g_entity_c*)dGeomGetData(o2);
				
				
			if(e1 && e2)
			{
				// test if both pointers are valid entities
				G_GetNumForEntity(e1);
				G_GetNumForEntity(e2);
			
				// set hit world surface if any
				csurface_c *surf = NULL;	//FIXME use contact geom flags
					
				/*
				if(e1 == g_world)
					surf = g1->getSurface();
				
				else if(e2 == g_world)
					surf = g2->getSurface();
				*/
			
				// set collision plane
				const vec3_c& point = contacts[0].geom._origin;
				const vec3_c& normal = contacts[0].geom._normal;
								
				plane_c plane(normal, normal.dotProduct(point));
				
				if(!e1->touch(e2, plane, surf))
					return;
			
				if(!e2->touch(e1, plane, surf))
					return;
			}

			for(int i=0; i<contacts_num; i++)
			{
				contacts[i].surface.mode = 0;//dContactBounce | dContactSoftCFM;
				contacts[i].surface.mu = X_infinity;
				contacts[i].surface.mu2 = 0;
				contacts[i].surface.bounce = 0.1;
				contacts[i].surface.bounce_vel = 0.1;
				contacts[i].surface.soft_cfm = 0.0001;
				
				#if 0
				trap_SV_WriteBits(SVC_TEMP_ENTITY, svc_bitcount);
				trap_SV_WriteByte(TE_CONTACT);
				trap_SV_WritePosition(contacts[i].geom._origin);
				trap_SV_WriteDir(contacts[i].geom._normal);
				trap_SV_WriteFloat(contacts[i].geom._depth);
				trap_SV_Multicast(contacts[i].geom._origin, MULTICAST_ALL);
				#endif
		
				dJointID c = dJointCreateContact(g_ode_world->getId(), g_ode_contact_group->getId(), &contacts[i]);
				dJointAttach(c, b1, b2);
			}
		}
		
	}// dGeomIsSpace
}	

void	G_RunDynamics(float step_size)
{
	g_ode_space_toplevel->collide(NULL, G_TopLevelCollisionCallback);
	
//	g_ode_world->step(step_size);
//	g_ode_world->stepFast(step_size, 10);
	g_ode_world->stepQuick(step_size);
	
	g_ode_contact_group->empty();
}


static trace_t			g_ray_trace;
static std::vector<dContact>	g_ray_contacts;

static void	G_RayCollisionCallback(void *data, dGeomID o1, dGeomID o2)
{
	if(dGeomIsSpace(o1) || dGeomIsSpace(o2))
	{
		// colliding a space with something
 		dSpaceCollide2(o1, o2, data, &G_RayCollisionCallback);
		
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
		std::vector<dContact>	contacts;//[contacts_max];
		if(int contacts_num = dCollide(o1, o2, contacts_max, contacts))
		{
			g_ray_contacts.resize(g_ray_contacts.size() + contacts_num);
		
			for(int i=0; i<contacts_num; i++)
			{
				g_ray_contacts[g_ray_contacts.size() + i] = contacts[i];
			}
		}
	}
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
	d_ray_c ray(g_ode_space_toplevel->getId(), start, dir, length);
	
	// fire ray against everything
	g_ode_space_toplevel->collide(NULL, G_RayCollisionCallback);
	
	if(g_ray_contacts.size())
	{
		//trap_Com_Printf("G_RayTrace: %i contacts\n", g_ray_contacts.size());
		
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
		g_ray_trace.pos = start + dir * dnearest.geom._depth;
		
		g_ray_trace.depth = dnearest.geom._depth;
	
		// calc plane
		const vec3_c& normal = dnearest.geom._normal;
		const vec3_c& x = dnearest.geom._origin;
	
		g_ray_trace.plane.set(normal, normal.dotProduct(x));
	}
	
	return g_ray_trace;
}
#endif // defined(ODE)



/*
void		G_SetWorldModel(g_entity_c *ent, const std::string &name)
{
	if(name.empty())
		Com_Error(ERR_DROP, "G_SetWorldModel: empty name");
	
	Com_Printf("G_SetWorldModel: '%s'\n", name.c_str());
	
	ent->_s.index_model = trap_SV_ModelIndex(name);
	
	cmodel_c* model = trap_CM_RegisterModel(name);
		
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
		
		//if(surf.hasFlags(SURF_NOIMPACT | X_SURF_NONSOLID))
		//	continue;
		
		g_geom_info_c *geom_info = new g_geom_info_c(ent, model, &surf);
		
		d_geom_c *geom = new d_trimesh_c(ent->_space->getId(), surf.getVertexes(), surf.getIndexes());
		geom->setData(geom_info);
	
		ent->_geoms.insert(std::make_pair(geom, geom_info));
	}
#else

#if defined(ODE)
	if(X_strequal("*0", name.c_str()) && g_ode_bsp)
	{
		g_ode_bsp->setData(ent);
		g_ode_bsp->setCollideBits(X_CONT_SOLID);
		ent->_geoms.push_back(g_ode_bsp);
	}

#endif
	
#endif
		
	ent->_r.bbox = model->getAABB();
	ent->_r.size = ent->_r.bbox.size();
}
*/
