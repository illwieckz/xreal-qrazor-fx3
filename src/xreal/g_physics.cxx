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
static g_entity_c*	G_TestEntityPosition(g_entity_c *ent)
{
	trace_t	trace;
	int		mask;

	if (ent->_r.clipmask)
		mask = ent->_r.clipmask;
	else
		mask = MASK_SOLID;
	trace = trap_SV_Trace (ent->_s.origin, ent->_r.bbox, ent->_s.origin, ent, mask);
	
	if (trace.startsolid)
		return (g_entity_c*)g_edicts[0];
		
	return NULL;
}
*/

/*
static void	G_CheckVelocity(g_entity_c *ent)
{
	float	scale;

	//
	// bound velocity
	//
	scale = ent->_velocity.length();
	
	if((scale > sv_maxvelocity->value) && (scale))
	{
		scale = sv_maxvelocity->value / scale;
			
		ent->_velocity.scale(scale);
	}
}
*/



/*
==================
SV_Impact

Two entities have touched, so run their touch functions
==================
*/
/*
static void	G_Impact(g_entity_c *e1, trace_t *trace)
{
	g_entity_c		*e2;
//	cplane_t	backplane;

	e2 = (g_entity_c*)trace->ent;

	//trap_Com_Printf("SV_Impact: %s %s\n", e1->_classname, e2->_classname);
	
	if(!e1 || !e2)
		return;
	
	if(e1->_r.solid != SOLID_NOT)
		e1->touch(e2, &trace->plane, trace->surface);
	
	if(e2->_r.solid != SOLID_NOT)
		e2->touch(e1, NULL, NULL);
}
*/


/*
==================
ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
/*
#define	STOP_EPSILON	0.1
static int	ClipVelocity(vec3_t in, vec3_t normal, vec3_t out, float overbounce)
{
	float	backoff;
	float	change;
	int		i, blocked;
	
	blocked = 0;
	
	if (normal[2] > 0)
		blocked |= 1;		// floor
		
	if (!normal[2])
		blocked |= 2;		// step
	
	backoff = Vector3_DotProduct (in, normal) * overbounce;

	for (i=0 ; i<3 ; i++)
	{
		change = normal[i]*backoff;
		
		out[i] = in[i] - change;
		
		if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}

	return blocked;
}
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
static void	G_AddGravity(g_entity_c *ent)
{
	ent->_velocity[2] -= ent->_gravity * g_gravity->value * FRAMETIME;
}
*/

/*
===============================================================================
				PUSHMOVE
===============================================================================
*/

/*
============
SV_PushEntity

Does not change the entities velocity at all
============
*/
/*
static trace_t	G_PushEntity(g_entity_c *ent, vec3_t push)
{
	trace_t	trace;
	vec3_c	start;
	vec3_c	end;
	int		mask;

	Vector3_Copy (ent->_s.origin, start);
	Vector3_Add (start, push, end);

retry:
	if (ent->_r.clipmask)
		mask = ent->_r.clipmask;
	else
		mask = MASK_SOLID;

	trace = trap_SV_Trace (start, ent->_r.bbox, end, ent, mask);
	
	Vector3_Copy (trace.endpos, ent->_s.origin);
	trap_SV_LinkEdict (ent);

	if (trace.fraction != 1.0)
	{
		G_Impact(ent, &trace);

		// if the pushed entity went away and the pusher is still there
		if (!trace.ent->_r.inuse && ent->_r.inuse)
		{
			// move the pusher back and try again
			Vector3_Copy (start, ent->_s.origin);
			trap_SV_LinkEdict (ent);
			goto retry;
		}
	}

	if (ent->_r.inuse)
		G_TouchTriggers (ent);

	return trace;
}					
*/

/*
typedef struct
{
	g_entity_c	*ent;
	vec3_t	origin;
	vec3_t	angles;
	float	deltayaw;
} pushed_t;
pushed_t	pushed[MAX_ENTITIES], *pushed_p;

g_entity_c	*obstacle;
*/

/*
============
SV_Push

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.

//FIXME rewrite this mess in a clean way
============
*/

/*
static bool	G_Push(g_entity_c *pusher, vec3_t move, vec3_t amove)
{
	int			i, e;
	g_entity_c		*check, *block;
	vec3_c		mins, maxs;
	pushed_t	*p;
	vec3_c		org, org2, move2, forward, right, up;

	// clamp the move to 1/8 units, so the position will
	// be accurate for client side prediction
	for (i=0 ; i<3 ; i++)
	{
		float	temp;
		temp = move[i]*8.0;
		if (temp > 0.0)
			temp += 0.5;
		else
			temp -= 0.5;
		move[i] = 0.125 * (int)temp;
	}

	// find the bounding box
	for (i=0 ; i<3 ; i++)
	{
		mins[i] = pusher->_r.bbox_abs._mins[i] + move[i];
		maxs[i] = pusher->_r.bbox_abs._maxs[i] + move[i];
	}

	// we need this for pushing things later
	Vector3_Subtract (vec3_origin, amove, org);
	Angles_ToVectors (org, forward, right, up);

	// save the pusher's original position
	pushed_p->ent = pusher;
	Vector3_Copy (pusher->_s.origin, pushed_p->origin);
	Vector3_Copy (pusher->_s.angles, pushed_p->angles);
	
	//if (pusher->getClient())
	//	pushed_p->deltayaw = pusher->getClient()->ps.pmove.delta_angles[YAW];
	pushed_p++;

	// move the pusher to it's final position
	Vector3_Add (pusher->_s.origin, move, pusher->_s.origin);
	Vector3_Add (pusher->_s.angles, amove, pusher->_s.angles);
	trap_SV_LinkEdict (pusher);

	// see if any solid entities are inside the final position
	for (e = 1; e < (int)g_edicts.size(); e++)
	{
		check = (g_entity_c*)g_edicts[e];
		
		if(!check)
			continue;
	
		if (!check->_r.inuse)
			continue;
			
		if (check->_movetype == MOVETYPE_PUSH
		|| check->_movetype == MOVETYPE_STOP
		|| check->_movetype == MOVETYPE_NONE
		|| check->_movetype == MOVETYPE_NOCLIP)
			continue;

		//if (!check->r.area.prev)
		if(!check->_r.islinked)
			continue;		// not linked in anywhere

		// if the entity is standing on the pusher, it will definitely be moved
		if (check->_groundentity != pusher)
		{
			// see if the ent needs to be tested
			if ( check->_r.bbox_abs._mins[0] >= maxs[0]
			|| check->_r.bbox_abs._mins[1] >= maxs[1]
			|| check->_r.bbox_abs._mins[2] >= maxs[2]
			|| check->_r.bbox_abs._maxs[0] <= mins[0]
			|| check->_r.bbox_abs._maxs[1] <= mins[1]
			|| check->_r.bbox_abs._maxs[2] <= mins[2] )
				continue;

			// see if the ent's bbox is inside the pusher's final position
			if (!G_TestEntityPosition (check))
				continue;
		}

		if ((pusher->_movetype == MOVETYPE_PUSH) || (check->_groundentity == pusher))
		{
			// move this entity
			pushed_p->ent = check;
			Vector3_Copy (check->_s.origin, pushed_p->origin);
			Vector3_Copy (check->_s.angles, pushed_p->angles);
			pushed_p++;

			// try moving the contacted entity 
			Vector3_Add (check->_s.origin, move, check->_s.origin);
			
			//if (check->getClient())
			//{	// FIXME: doesn't rotate monsters?
			//	check->getClient()->ps.pmove.delta_angles[YAW] += amove[YAW];
			//}

			// figure movement due to the pusher's amove
			Vector3_Subtract (check->_s.origin, pusher->_s.origin, org);
			org2[0] = Vector3_DotProduct (org, forward);
			org2[1] = -Vector3_DotProduct (org, right);
			org2[2] = Vector3_DotProduct (org, up);
			Vector3_Subtract (org2, org, move2);
			Vector3_Add (check->_s.origin, move2, check->_s.origin);

			// may have pushed them off an edge
			if (check->_groundentity != pusher)
				check->_groundentity = NULL;

			block = G_TestEntityPosition (check);
			if (!block)
			{	// pushed ok
				trap_SV_LinkEdict (check);
				// impact?
				continue;
			}

			// if it is ok to leave in the old position, do it
			// this is only relevent for riding entities, not pushed
			// FIXME: this doesn't acount for rotation
			Vector3_Subtract (check->_s.origin, move, check->_s.origin);
			block = G_TestEntityPosition (check);
			if (!block)
			{
				pushed_p--;
				continue;
			}
		}
		
		// save off the obstacle so we can call the block function
		obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for (p=pushed_p-1 ; p>=pushed ; p--)
		{
			Vector3_Copy (p->origin, p->ent->_s.origin);
			Vector3_Copy (p->angles, p->ent->_s.angles);
			
			//if (p->ent->getClient())
			//{
			//	p->ent->getClient()->ps.pmove.delta_angles[YAW] = p->deltayaw;
			//}
			trap_SV_LinkEdict (p->ent);
		}
		return false;
	}

//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
	for (p=pushed_p-1 ; p>=pushed ; p--)
		G_TouchTriggers (p->ent);

	return true;
}
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
=============
Non moving objects can only think
=============
*/
/*
static void	G_Physics_None(g_entity_c *ent)
{
	// regular thinking
	G_RunThink(ent);
}
*/

/*
=============
A moving object that doesn't obey physics
=============
*/
/*
static void	G_Physics_Noclip(g_entity_c *ent)
{
	// regular thinking
	if(!G_RunThink(ent))
		return;
	
	//Vector3_MA(ent->_s.angles, FRAMETIME, ent->_avelocity, ent->_s.angles);
	//Vector3_MA(ent->_s.origin, FRAMETIME, ent->_velocity, ent->_s.origin);
	
	ent->_s.origin = ent->_body->getPosition();

	//trap_SV_LinkEdict(ent);
}
*/

/*
==============================================================================
				TOSS / BOUNCE
==============================================================================
*/

/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
/*
static void	G_Physics_Toss(g_entity_c *ent)
{
	trace_t		trace;
	vec3_t		move;
	float		backoff;
	g_entity_c		*slave;
	bool	wasinwater;
	bool	isinwater;
	vec3_t		old_origin;

	// regular thinking
	G_RunThink (ent);

	// if not a team captain, so movement will be handled elsewhere
	if(ent->_flags & FL_TEAMSLAVE)
		return;

	if(ent->_velocity[2] > 0)
		ent->_groundentity = NULL;

	// check for the groundentity going away
	if(ent->_groundentity)
		if(!ent->_groundentity->_r.inuse)
			ent->_groundentity = NULL;


	// if onground, return without moving
	if(ent->_groundentity)
		return;

	Vector3_Copy (ent->_s.origin, old_origin);

	G_CheckVelocity(ent);

	// add gravity
	if(ent->_movetype != MOVETYPE_FLY && ent->_movetype != MOVETYPE_FLYMISSILE)
		G_AddGravity(ent);

	// move angles
	Vector3_MA(ent->_s.angles, FRAMETIME, ent->_avelocity, ent->_s.angles);

	// move origin
	Vector3_Scale(ent->_velocity, FRAMETIME, move);
	trace = G_PushEntity(ent, move);
	if(!ent->_r.inuse)
		return;

	if(trace.fraction < 1)
	{
		if(ent->_movetype == MOVETYPE_BOUNCE)
			backoff = 1.5;
		else
			backoff = 1;

		ClipVelocity(ent->_velocity, trace.plane._normal, ent->_velocity, backoff);

		// stop if on ground
		if(trace.plane._normal[2] > 0.7)
		{		
			if (ent->_velocity[2] < 60 || ent->_movetype != MOVETYPE_BOUNCE)
			{
				ent->_groundentity = (g_entity_c*)trace.ent;
				ent->_groundentity_linkcount = trace.ent->_r.linkcount;
				ent->_velocity.clear();
				ent->_avelocity.clear();
			}
		}

		//if (ent->touch)
		//	ent->touch (ent, trace.ent, &trace.plane, trace.surface);
	}
	
	// check for water transition
	wasinwater = (ent->_watertype & MASK_WATER);
	ent->_watertype = trap_SV_PointContents (ent->_s.origin);
	isinwater = ent->_watertype & MASK_WATER;

	if (isinwater)
		ent->_waterlevel = 1;
	else
		ent->_waterlevel = 0;

	if (!wasinwater && isinwater)
		trap_SV_StartSound (old_origin, g_edicts[0], CHAN_AUTO, trap_SV_SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);
		
	else if (wasinwater && !isinwater)
		trap_SV_StartSound (ent->_s.origin, g_edicts[0], CHAN_AUTO, trap_SV_SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);

	// move teamslaves
	for (slave = ent->_teamchain; slave; slave = slave->_teamchain)
	{
		Vector3_Copy (ent->_s.origin, slave->_s.origin);
		trap_SV_LinkEdict (slave);
	}
}
*/

/*
=============
G_ODE_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
/*
static void	G_ODE_Toss(g_entity_c *ent)
{
	// regular thinking
	G_RunThink(ent);
	
	//
	//ent->_s.origin = ent->_geom->getPosition();
	ent->_s.origin = ent->_body->getPosition();
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

/*
void	G_RunEntity(g_entity_c *ent)
{
	//trap_Com_Printf("G_RunEntity: %s\n", ent->classname);

	switch(ent->_movetype)
	{
		case MOVETYPE_PUSH:
		case MOVETYPE_STOP:
			//G_Physics_Pusher(ent);
			break;
			
		case MOVETYPE_NONE:
			//G_Physics_None(ent);
			break;
			
		case MOVETYPE_NOCLIP:
			//G_Physics_Noclip(ent);
			break;
			
		case MOVETYPE_STEP:
			//G_Physics_Step(ent);
			break;
			
		case MOVETYPE_TOSS:
		case MOVETYPE_BOUNCE:
		case MOVETYPE_FLY:
		case MOVETYPE_FLYMISSILE:
			//G_Physics_Toss(ent);
			break;
			
		case MOVETYPE_ODE_TOSS:
			//G_ODE_Toss(ent);
			break;
			
		default:
			trap_Com_Error (ERR_DROP, "G_RunEntity: bad movetype %i", ent->_movetype);
	}
}
*/


void	G_InitDynamics()
{
	g_ode_world = new d_world_c();
	
	// apply real world gravity by default
	vec3_c gravity(0, 0, (-9.81 * (32.0/1.0)) * g_gravity->getValue());

	g_ode_world->setGravity(gravity);
	g_ode_world->setCFM(1e-5);
//	g_ode_world->setAutoDisableFlag(true);
//	g_ode_world->setContactMaxCorrectingVel(0.1);
//	g_ode_world->setContactSurfaceLayer(0.001);
	
	g_ode_space_toplevel = new d_simple_space_c();
//	g_ode_space_toplevel = new d_hash_space_c();
	
//	g_ode_space_world = new d_simple_space_c(g_ode_space_toplevel->getId());

	g_ode_testplane = new d_plane_c(g_ode_space_toplevel->getId(), vec3_c(0, 0, 1), 0);
	
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

	vec_t depth_a = contact_a->geom.depth;
	vec_t depth_b = contact_b->geom.depth;
	
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

		const int	contacts_max = 16;
		dContact	contacts[contacts_max]; 
		if(int contacts_num = dCollide(o1, o2, contacts_max, &contacts[0].geom, sizeof(dContact)))
		{
			// sort contacts by penetration depth
			qsort(contacts, contacts_num, sizeof(dContact), G_SortByContactGeomDepthFunc);
		
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
				contacts[i].surface.mu = X_infinity;
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

	ent->_s.index_model = trap_SV_ModelIndex(name);
	
	cmodel_c* model = trap_CM_RegisterModel(name);
		
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
		
		//if(surf.hasFlags(/*SURF_NOIMPACT |*/ X_SURF_NONSOLID))
		//	continue;
		
		g_geom_info_c *geom_info = new g_geom_info_c(ent, model, &surf);
		
		d_geom_c *geom = new d_trimesh_c(ent->_space->getId(), surf.getVertexes(), surf.getIndexes());
		geom->setData(geom_info);
	
		ent->_geoms.insert(std::make_pair(geom, geom_info));
	}
#else

#if 0
	if(X_strequal("*0", name.c_str()) && g_ode_bsp)
	{
		g_ode_bsp->setData(ent);
		ent->_geoms.push_back(g_ode_bsp);
	}
	else if(model->vertexes.size() && model->indexes.size())
	{
		d_geom_c *geom = new d_trimesh_c(g_ode_space_toplevel->getId(), model->vertexes, model->indexes);
		//d_geom_c *geom = new d_box_c(g_ode_space_toplevel->getId(), model->getBBox().size());
		geom->setData(ent);
		ent->_geoms.push_back(geom);
	}
#endif
	
#endif
		
	ent->_r.bbox = model->getBBox();
	ent->_r.size = ent->_r.bbox.size();
}

