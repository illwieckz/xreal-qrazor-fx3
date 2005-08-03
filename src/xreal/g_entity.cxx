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
#include "g_entity.h"
#include "g_local.h"




g_entity_c::g_entity_c(bool create_rigid_body)
{
	//trap_Com_Printf ("g_entity_c::ctor:\n");

	//
	// create default values
	//
	_s.shaderparms[0] = 1.0;
	_s.shaderparms[1] = 1.0;
	_s.shaderparms[2] = 1.0;
	_s.shaderparms[3] = 1.0;
	_s.shaderparms[4] = 1.0;
	_s.shaderparms[5] = 1.0;
	_s.shaderparms[6] = 1.0;
// 	_s.shaderparms[7] = 1.0; 
	
	_r.inuse	= false;
	_r.headnode	= 0;			// unused if num_clusters != -1
	
	_r.area		= 0;
	_r.area2	= 0;
	_r.areaportal	= 0;

	_r.svflags	= SVF_NONE;		// SVF_NOCLIENT, SVF_DEADMONSTER, SVF_MONSTER, etc
	_r.bbox.zero();
	_r.size.clear();			// bbox size
	_r.solid	= SOLID_NOT;
	_r.clipmask	= 0;
	
	_r.owner	= NULL;
	
	_r.isclient	= false;
	_r.isbot	= false;
	_r.ps.clear();				// communicated by server to clients
	//_r.ping;
	
	_remove		= false;
	_spawntime	= level.time;
	_classname	= "noclass";
	_movetype	= MOVETYPE_NONE;
	_flags		= 0;
	_spawnflags	= 0;

	_angles.clear();
	_angle		= 0;				// set in qe3, -1 = up, -2 = down
	_rotation.identity();
	
	_target_ent	= NULL;

	_speed		= 0;
	_accel		= 0;
	_decel		= 0;
	_movedir.clear();
	
	_mass		= 0;
	_gravity	= 1.0;

	_goalentity	= 0;
	_yaw_speed	= 0;
	_ideal_yaw	= 0;

	_nextthink	= 0;			// time when this entity will think again	

	_touch_debounce_time	= 0;		// are all these legit?  do we need more/less of them?
	_pain_debounce_time	= 0;
	_damage_debounce_time	= 0;
	_fly_sound_debounce_time	= 0;
	_last_move_time	= 0;

	_health		= 0;
	_max_health	= 0;
	_deadflag	= 0;

	_takedamage	= 0;
	_dmg		= 0;
	_radius_dmg	= 0;
	_dmg_radius	= 0;
	_sounds		= 0;			// make this a spawntemp var?
	_count		= 0;

	_chain		= NULL;
	_enemy		= NULL;
	_activator	= NULL;
	_groundentity	= NULL;
	_groundentity_linkcount = 0;
	
	_teamchain	= NULL;
	_teammaster	= NULL;

	_noise_index	= 0;
	_noise_index2	= 0;
	_volume		= 0;
	_attenuation	= 0;
	
	_wait		= 0;
	_delay		= 0;
	_random		= 0;

	_watertype	= 0;
	_waterlevel	= 0;
		
	_item		= NULL;			// for bonus items

#if defined(ODE)		
	_body		= NULL;
	_space		= NULL;			// used only when we need a group of geoms
#endif

	//
	// find free entity slot
	//
	std::vector<sv_entity_c*>::iterator ir = std::find(g_entities.begin(), g_entities.end(), static_cast<sv_entity_c*>(NULL));
	
	if(ir != g_entities.end())
	{
		*ir = this;
			
		//trap_Com_Printf("g_entity_c::ctor: found free slot %i\n", s.number);
	}
	else
	{
		if(g_entities.size() == MAX_ENTITIES)
			trap_Com_Error(ERR_DROP, "g_entity_c::ctor: max entities limit reached, have to change network protocol for more");
		
		g_entities.push_back(this);	
	}
	

	//
	// setup fields
	//
	
	// network entity state
	addField(g_field_c("origin", &_s.origin, F_VECTOR));
	
	addField(g_field_c("angle", &_s.quat, F_ANGLEHACK_TO_QUATERNION));
	addField(g_field_c("angles",&_s.quat, F_ANGLES_TO_QUATERNION));
	addField(g_field_c("rotation", &_s.quat, F_ROTATION_TO_QUATERNION));
	
	addField(g_field_c("_color", &_s.shaderparms[0], F_VECTOR));

	addField(g_field_c("shaderparm0", &_s.shaderparms[0], F_FLOAT));
	addField(g_field_c("shaderparm1", &_s.shaderparms[1], F_FLOAT));
	addField(g_field_c("shaderparm2", &_s.shaderparms[2], F_FLOAT));
	addField(g_field_c("shaderparm3", &_s.shaderparms[3], F_FLOAT));
	addField(g_field_c("shaderparm4", &_s.shaderparms[4], F_FLOAT));
	addField(g_field_c("shaderparm5", &_s.shaderparms[5], F_FLOAT));
	addField(g_field_c("shaderparm6", &_s.shaderparms[6], F_FLOAT));
	addField(g_field_c("shaderparm7", &_s.shaderparms[7], F_FLOAT));

	// shared between server and game
//	addField(g_field_c("networksync", &_r.networksync, F_BOOL));
	addField(g_field_c("owner", &_r.owner, F_EDICT, FFL_NOSPAWN));
	
	// private to game
	addField(g_field_c("classname", &_classname, F_STRING));
	addField(g_field_c("model", &_model, F_STRING));
	addField(g_field_c("spawnflags", &_spawnflags, F_INT));
	addField(g_field_c("speed", &_speed, F_FLOAT));
	addField(g_field_c("accel", &_accel, F_FLOAT));
	addField(g_field_c("decel", &_decel, F_FLOAT));
//	addField(g_field_c("movedir", &_movedir, F_ANGLEHACK_TO_QUATERNION));
	addField(g_field_c("target", &_target, F_STRING));
	addField(g_field_c("target1", &_target1, F_STRING));
	addField(g_field_c("target2", &_target2, F_STRING));
	addField(g_field_c("name", &_targetname, F_STRING));
	addField(g_field_c("targetname", &_targetname, F_STRING));
	addField(g_field_c("pathtarget", &_pathtarget, F_STRING));
	addField(g_field_c("deathtarget", &_deathtarget, F_STRING));
	addField(g_field_c("killtarget", &_killtarget, F_STRING));
	addField(g_field_c("combattarget", &_combattarget, F_STRING));
	addField(g_field_c("message", &_message, F_STRING));
	addField(g_field_c("team", &_team, F_STRING));
	addField(g_field_c("wait", &_wait, F_FLOAT));
	addField(g_field_c("delay", &_delay, F_FLOAT));
	addField(g_field_c("random", &_random, F_FLOAT));
	
	addField(g_field_c("count", &_count, F_INT));
	addField(g_field_c("health", &_health, F_INT));
	addField(g_field_c("sounds", &_sounds, F_INT));
	addField(g_field_c("dmg", &_dmg, F_INT));
	addField(g_field_c("mass", &_mass, F_FLOAT));
	addField(g_field_c("gravity", &_gravity, F_FLOAT));
	addField(g_field_c("volume", &_volume, F_FLOAT));
	addField(g_field_c("attenuation", &_attenuation, F_FLOAT));
	
	addField(g_field_c("goalentity", &_goalentity, F_EDICT, FFL_NOSPAWN));
	addField(g_field_c("enemy", &_enemy, F_EDICT, FFL_NOSPAWN));
	addField(g_field_c("activator", &_activator, F_EDICT, FFL_NOSPAWN));
	addField(g_field_c("groundentity", &_groundentity, F_EDICT, FFL_NOSPAWN));
	addField(g_field_c("teamchain", &_teamchain, F_EDICT, FFL_NOSPAWN));
	addField(g_field_c("teammaster", &_teammaster, F_EDICT, FFL_NOSPAWN));
	addField(g_field_c("target_ent", &_target_ent, F_EDICT, FFL_NOSPAWN));
	addField(g_field_c("chain", &_chain, F_EDICT, FFL_NOSPAWN));
	
	//need for item field in edict struct, FFL_SPAWNTEMP item will be skipped on saves
	addField(g_field_c("item", &_item, F_ITEM, FFL_SPAWNTEMP));	
	
	
	//
	// set number
	//
	_s.number = G_GetNumForEntity(this);
	
	
	//
	// set rigid body
	//
#if defined(ODE)
	if(create_rigid_body)
	{
		_body = new d_body_c(g_ode_world->getId());
		_body->setData(this);
	}
#endif
}


g_entity_c::~g_entity_c()
{
	//trap_Com_Printf ("g_entity_c::dtor: %s %i\n", _classname, _s.number);
	
	//
	// reset entity's slot
	//
	std::vector<sv_entity_c*>::iterator ir = find(g_entities.begin(), g_entities.end(), static_cast<sv_entity_c*>(this));
	
	if(ir != g_entities.end())
		*ir = NULL;
	else
		trap_Com_Error(ERR_FATAL, "g_entity_c::dtor: entity '%s' %i not in list", _classname.c_str(), _s.number);
		
	
	//
	// clear dynamics
	//
#if defined(ODE)
	for(std::vector<d_geom_c*>::iterator ir = _geoms.begin(); ir != _geoms.end(); ++ir)
	{
		if((*ir)->isEnabled())
		{
			trap_Com_Error(ERR_FATAL, "g_entity_c::dtor: tried to delete entity '%s' %i which has enabled geom object", _classname.c_str(), _s.number);
		}
		
		delete *ir;
	}
	
	if(_body)
	{	
		delete _body;
		_body = NULL;
	}
#endif
}

void	g_entity_c::run()
{
	//trap_Com_Printf("G_RunEntity: %s\n", ent->classname);

	if(!_r.inuse)
		return;

	if(_r.isclient)
		return;

	// regular thinking
	runThink();

	switch(_movetype)
	{
		case MOVETYPE_PUSH:
		case MOVETYPE_STOP:
			//G_Physics_Pusher(ent);
			break;
			
		case MOVETYPE_NONE:
			runPhysicsNone();
			break;
			
		case MOVETYPE_NOCLIP:
			runPhysicsNoclip();
			break;
			
		case MOVETYPE_STEP:
			//G_Physics_Step(ent);
			break;
			
		case MOVETYPE_TOSS:
		case MOVETYPE_BOUNCE:
		case MOVETYPE_FLY:
		case MOVETYPE_FLYMISSILE:
			runPhysicsToss();
			break;
			
		//case MOVETYPE_ODE_TOSS:
			//G_ODE_Toss(ent);
			//break;
			
		default:
			trap_Com_Error(ERR_DROP, "runPhysics: bad movetype %i", _movetype);
	}
}

void 	g_entity_c::takeDamage(g_entity_c *inflictor, g_entity_c *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	int			take;
	int			save;
	int			te_sparks;

	if(!_takedamage)
		return;

	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if((this != attacker) && ((deathmatch->getInteger() && (dmflags->getInteger() & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->getInteger()))
	{
		if(G_OnSameTeam(this, attacker))
		{
			if(dmflags->getInteger() & DF_NO_FRIENDLY_FIRE)
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
		}
	}
	meansOfDeath = mod;

	
	if(dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	Vector3_Normalize(dir);

	
	if(_flags & FL_NO_KNOCKBACK)
		knockback = 0;

	// figure momentum add
	if(!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if((knockback) && (_movetype != MOVETYPE_NONE) && (_movetype != MOVETYPE_BOUNCE) && (_movetype != MOVETYPE_PUSH) && (_movetype != MOVETYPE_STOP))
		{
			vec3_c	kvel;
			float	mass;

			if(_mass < 50)
				mass = 50;
			else
				mass = _mass;

			Vector3_Scale(dir, 500.0 * (float)knockback / mass, kvel);

			#if defined(ODE)
			if(_body)
				_body->addForce(kvel);
			#endif
		}
	}

	take = damage;
	save = 0;

	// team damage avoidance
	//if(!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(this, attacker))
	//	return;

	// do the damage
	if(take)
	{
		G_SpawnDamage(te_sparks, point, normal, take);

		_health -= take;
			
		if(_health <= 0)
		{
			G_Killed(this, inflictor, attacker, take, point);
			return;
		}
		
		pain(attacker, knockback, take);
	}
}

void	g_entity_c::remove()
{
	_r.inuse = false;
	
#if defined(ODE)	
	for(std::vector<d_geom_c*>::iterator ir = _geoms.begin(); ir != _geoms.end(); ++ir)
	{
		(*ir)->disable();
	}
		
	if(_body)
		_body->disable();
#endif
		
	_remove = true;	// ready to destroy
}

void	g_entity_c::setEPairs(const epairs_t &epairs)
{
	_epairs = epairs;
}

bool	g_entity_c::hasEPair(const std::string &key)
{
	epairs_ci i = _epairs.find(key);
	
	if(i != _epairs.end())
		return true;
		
	return false;
}

const char*	g_entity_c::valueForKey(const std::string &key)
{
	epairs_ci i = _epairs.find(key);
	
	if(i != _epairs.end())
		return i->second.c_str();
	
	return "";
}

void	g_entity_c::updateField(const std::string &key)
{
	const char* value = valueForKey(key);
	
	if(value[0] != '\0')
		setField(key, value);
}

#if defined(ODE)
void	g_entity_c::updateOrigin()
{
	if(_body)
		_s.origin = _body->getPosition();
}

void	g_entity_c::updateRotation()
{
	if(_body) 
		_s.quat = _body->getQuaternion();
}

void	g_entity_c::updateVelocity()
{
	if(_body)
		_s.velocity_linear = _body->getLinearVel();
}
#endif


/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
bool	g_entity_c::runThink()
{
	float thinktime = _nextthink;
	
	if(thinktime <= 0)
		return true;
		
	if(thinktime > level.time + 0.001)
		return true;
	
	_nextthink = 0;
	
	think();

	return false;
}

void	g_entity_c::addGravity()
{
	_s.velocity_linear[2] -= _gravity * g_gravity->getValue() * FRAMETIME;
}

void	g_entity_c::applyLinearVelocity()
{
	_s.origin += _s.velocity_linear * FRAMETIME;
}

void	g_entity_c::applyAngularVelocity()
{
	//TODO
	//Vector3_MA(ent->_s.angles, FRAMETIME, ent->_avelocity, ent->_s.angles);
}

g_entity_c*	g_entity_c::checkPosition()
{
	int	mask;
	if(_r.clipmask)
		mask = _r.clipmask;
	else
		mask = MASK_SOLID;
	
	trace_t trace = G_Trace(_s.origin, _r.bbox, _s.origin, this, mask);
	
	if(trace.startsolid)
	{
		return (g_entity_c*)g_world;
	}
		
	return NULL;
}

void	g_entity_c::checkVelocity()
{
	// bound velocity
	vec_t scale = _s.velocity_linear.length();
	
	if((scale > sv_maxvelocity->getValue()) && (scale))
	{
		scale = sv_maxvelocity->getValue() / scale;
			
		_s.velocity_linear *= scale;
	}
}

/*
==================
ClipVelocity

Slide off of the impacting object
returns the blocked flags (1 = floor, 2 = step / wall)
==================
*/
#define	STOP_EPSILON	0.1
int	g_entity_c::clipVelocity(const vec3_c &in, const vec3_c &normal, vec3_c &out, float overbounce)
{
	float	backoff;
	float	change;
	int		i, blocked;
	
	blocked = 0;
	
	if(normal[2] > 0)
		blocked |= 1;		// floor
		
	if(!normal[2])
		blocked |= 2;		// step
	
	backoff = normal.dotProduct(in) * overbounce;

	for(i=0; i<3; i++)
	{
		change = normal[i]*backoff;
		
		out[i] = in[i] - change;
		
		if(out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			out[i] = 0;
	}

	return blocked;
}


/*
==================
SV_Impact

Two entities have touched, so run their touch functions
==================
*/
void	g_entity_c::impact(const trace_t &trace)
{
	
//	cplane_t	backplane;

	g_entity_c *other = (g_entity_c*)trace.ent;

	//trap_Com_Printf("SV_Impact: %s %s\n", e1->_classname, e2->_classname);
	
	if(!other)
		return;
	
	if(_r.solid != SOLID_NOT)
		touch(other, trace.plane, trace.surface);

	if(other->_r.solid != SOLID_NOT)
		other->touch(this, -trace.plane, NULL);
}

/*
============
SV_PushEntity

Does not change the entities velocity at all
============
*/
trace_t	g_entity_c::push(const vec3_c &push)
{
	trace_t	trace;
	int	mask;

	vec3_c start = _s.origin;
	vec3_c end = start + push;;

retry:
	if(_r.clipmask)
		mask = _r.clipmask;
	else
		mask = MASK_SOLID;

	trace = G_Trace(start, _r.bbox, end, this, mask);
	
	_s.origin = trace.pos;
	link();

	if(trace.fraction != 1.0)
	{
		impact(trace);

		// if the pushed entity went away and the pusher is still there
		if(!((g_entity_c*)trace.ent)->_r.inuse && _r.inuse)
		{
			// move the pusher back and try again
			_s.origin = start;
			link();
			goto retry;
		}
	}

//	if(_r.inuse)
//		G_TouchTriggers(this);

	return trace;
}					


struct pushed_t
{
	g_entity_c	*ent;
	vec3_c		origin;
	quaternion_c	quat;
	vec_t		deltayaw;
};
pushed_t	g_pushed[MAX_ENTITIES], *pushed_p;
g_entity_c	*obstacle;

/*
============
SV_Push

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.

//FIXME rewrite this mess in a clean way
============
*/
bool	g_entity_c::push2(vec3_c &move, vec3_c &amove)
{
	g_entity_c		*check, *block;
	vec3_c		mins, maxs;
	pushed_t	*p;
	vec3_c		org, org2, move2, forward, right, up;

	// clamp the move to 1/8 units, so the position will
	// be accurate for client side prediction
	for(int i=0; i<3; i++)
	{
		float	temp;
		temp = move[i]*8.0;
		if(temp > 0.0)
			temp += 0.5;
		else
			temp -= 0.5;
		move[i] = 0.125 * (int)temp;
	}

	// find the bounding box
	for(int i=0; i<3; i++)
	{
		mins[i] = _r.bbox_abs._mins[i] + move[i];
		maxs[i] = _r.bbox_abs._maxs[i] + move[i];
	}

	// we need this for pushing things later
	Vector3_Subtract(vec3_origin, amove, org);
	Angles_ToVectors(org, forward, right, up);

	// save the pusher's original position
	pushed_p->ent = this;
	pushed_p->origin = _s.origin;
	pushed_p->quat = _s.quat;
	
	//if (pusher->getClient())
	//	pushed_p->deltayaw = pusher->getClient()->ps.pmove.delta_angles[YAW];
	pushed_p++;

	// move the pusher to it's final position
	_s.origin += move;
	_s.quat.multiplyRotation(amove);
	link();

	// see if any solid entities are inside the final position
	for(int e = 1; e < (int)g_entities.size(); e++)
	{
		check = (g_entity_c*)g_entities[e];
		
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
		if (check->_groundentity != this)
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
			if(!check->checkPosition())
				continue;
		}

		if((_movetype == MOVETYPE_PUSH) || (check->_groundentity == this))
		{
			// move this entity
			pushed_p->ent = check;
			pushed_p->origin = check->_s.origin;
			pushed_p->quat = check->_s.quat;
			pushed_p++;

			// try moving the contacted entity 
			check->_s.origin += move;
			
			if(_r.isclient)
			{	
				// FIXME: doesn't rotate monsters?
				//check->_ps.pmove.delta_angles[YAW] += amove[YAW];
			}

			// figure movement due to the pusher's amove
			Vector3_Subtract(check->_s.origin, _s.origin, org);
			org2[0] = Vector3_DotProduct(org, forward);
			org2[1] = -Vector3_DotProduct(org, right);
			org2[2] = Vector3_DotProduct(org, up);
			Vector3_Subtract(org2, org, move2);
			Vector3_Add(check->_s.origin, move2, check->_s.origin);

			// may have pushed them off an edge
			if(check->_groundentity != this)
				check->_groundentity = NULL;

			block = check->checkPosition();
			if(!block)
			{
				// pushed ok
				check->link();
				// impact?
				continue;
			}

			// if it is ok to leave in the old position, do it
			// this is only relevent for riding entities, not pushed
			// FIXME: this doesn't acount for rotation
			check->_s.origin -= move;
			block = check->checkPosition();
			if(!block)
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
		for(p=pushed_p-1; p>=g_pushed; p--)
		{
			p->ent->_s.origin = p->origin;
			p->ent->_s.quat = p->quat;
			
			//if (p->ent->getClient())
			//{
			//	p->ent->getClient()->ps.pmove.delta_angles[YAW] = p->deltayaw;
			//}
			p->ent->link();
		}
		return false;
	}

//FIXME: is there a better way to handle this?
	// see if anything we moved has touched a trigger
//	for (p=pushed_p-1 ; p>=pushed ; p--)
//		G_TouchTriggers (p->ent);

	return true;
}

/*
=============
Non moving objects can only think
=============
*/
void	g_entity_c::runPhysicsNone()
{
	// DO NOTHING
}


/*
=============
A moving object that doesn't obey physics
=============
*/
void	g_entity_c::runPhysicsNoclip()
{
	applyAngularVelocity();
	applyLinearVelocity();

	link();
}

/*
=============
SV_Physics_Toss

Toss, bounce, and fly movement.  When onground, do nothing.
=============
*/
void	g_entity_c::runPhysicsToss()
{
	// if not a team captain, so movement will be handled elsewhere
	if(_flags & FL_TEAMSLAVE)
		return;

	if(_s.velocity_linear[2] > 0)
		_groundentity = NULL;

	// check for the groundentity going away
	if(_groundentity)
	{
		if(!_groundentity->_r.inuse)
			_groundentity = NULL;
	}


	// if onground, return without moving
	if(_groundentity)
		return;

	vec3_c old_origin = _s.origin;

	checkVelocity();

	// add gravity
	if(_movetype != MOVETYPE_FLY && _movetype != MOVETYPE_FLYMISSILE)
		addGravity();

	// move angles
	applyAngularVelocity();

	// move origin
	vec3_c move = _s.velocity_linear * FRAMETIME;
	trace_t trace = push(move);
	if(!_r.inuse)
		return;

	if(trace.fraction < 1)
	{
		vec_t backoff;

		if(_movetype == MOVETYPE_BOUNCE)
			backoff = 1.5;
		else
			backoff = 1;

		clipVelocity(_s.velocity_linear, trace.plane._normal, _s.velocity_linear, backoff);

		// stop if on ground
		if(trace.plane._normal[2] > 0.7)
		{		
			if(_s.velocity_linear[2] < 60 || _movetype != MOVETYPE_BOUNCE)
			{
				_groundentity = (g_entity_c*)trace.ent;
				_groundentity_linkcount = ((g_entity_c*)trace.ent)->_r.linkcount;
				_s.velocity_linear.clear();
				_s.velocity_angular.clear();
			}
		}

		//if (ent->touch)
		//	ent->touch (ent, trace.ent, &trace.plane, trace.surface);
	}
	
	// check for water transition
	bool wasinwater = (_watertype & MASK_WATER);
	_watertype = trap_CM_PointContents(_s.origin, 0);
	bool isinwater = _watertype & MASK_WATER;

	if(isinwater)
		_waterlevel = 1;
	else
		_waterlevel = 0;

	if(!wasinwater && isinwater)
		trap_SV_StartSound(old_origin, g_world, CHAN_AUTO, trap_SV_SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);
		
	else if(wasinwater && !isinwater)
		trap_SV_StartSound(_s.origin, g_world, CHAN_AUTO, trap_SV_SoundIndex("misc/h2ohit1.wav"), 1, 1, 0);

	// move teamslaves
	for(g_entity_c* slave = _teamchain; slave; slave = slave->_teamchain)
	{
		slave->_s.origin = _s.origin;
		slave->link();
	}
}




void	g_entity_c::link()
{
	G_LinkEntity(this);
}

void	g_entity_c::unlink()
{
	G_UnlinkEntity(this);
}

bool	g_entity_c::inFront(const g_entity_c *other)
{
	vec3_c		forward, right, up;
	matrix_c	matrix;
	
	matrix.fromQuaternion(_s.quat);
	matrix.toVectorsFRU(forward, right, up);
	
	vec3_c vec(other->_s.origin - _s.origin);
	vec.normalize();	
	
	vec_t dot = vec.dotProduct(forward);
	if(dot > 0.3)
		return true;
	else
		return false;
}

void	G_ShutdownEntities()
{
	trap_Com_Printf("G_ShutdownEntities: %i edict slots left\n", g_entities.size());
	
	for(std::vector<sv_entity_c*>::iterator ir = g_entities.begin(); ir != g_entities.end(); ir++)
	{
		g_entity_c *ent = (g_entity_c*) *ir;
		
		if(ent)
		{
			ent->remove();	// get rid of enabled bodies and geoms
		
			//trap_Com_Printf("killing %s %i ...\n", ent->_classname, ent->_s.number);
			delete ent;
		}
	}
	
	g_entities.clear();
}



