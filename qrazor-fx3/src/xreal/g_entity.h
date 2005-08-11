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
#ifndef G_ENTITY_H
#define G_ENTITY_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "../x_shared.h"

#if defined(ODE)
#include "../x_ode.h"
#endif

// xreal --------------------------------------------------------------------
#include "g_public.h"
#include "g_field.h"


// edict->movetype values
enum movetype_t
{
	MOVETYPE_NONE,			// never moves
	MOVETYPE_NOCLIP,		// origin and angles change with no interaction
	MOVETYPE_PUSH,			// no clip to world, push on box contact
	MOVETYPE_STOP,			// no clip to world, stops on box contact

	MOVETYPE_WALK,			// gravity
	MOVETYPE_STEP,			// gravity, special edge handling
	MOVETYPE_FLY,
	MOVETYPE_TOSS,			// gravity
	MOVETYPE_FLYMISSILE,		// extra size to monsters
	MOVETYPE_BOUNCE,
//	MOVETYPE_ODE_TOSS
};


//class g_item_c;

typedef std::map<std::string, std::string, strcasecmp_c>	epairs_t;
typedef epairs_t::iterator					epairs_i;
typedef epairs_t::const_iterator				epairs_ci;

class g_entity_c :
public sv_entity_c, 
public g_field_iface_a
{
public:
	g_entity_c(bool create_rigid_body = true);
	virtual ~g_entity_c();
	
private:
	g_entity_c(const g_entity_c &ent);
	
	g_entity_c&	operator = (const g_entity_c &ent);
	
public:
	
	// think if necessary and update physical state in Q2/Q3A style
	virtual void	run();
	virtual bool	runThink();
	virtual void	runPhysics();

	virtual void	think() {};
	virtual void	blocked(g_entity_c *other) {};
	virtual bool	touch(g_entity_c *other, const plane_c *plane, const csurface_c *surf) {return true;}
	virtual void	use(g_entity_c *other, g_entity_c *activator) {};
	virtual void	pain(g_entity_c *other, float kick, int damage) {};
	virtual void	die(g_entity_c *inflictor, g_entity_c *attacker, int damage, const vec3_c &point) {};		//TODO rename to ::killed
	
	// called by G_SpawnEntities
	virtual void	activate() {};
	
	virtual std::string	clientTeam()	{return "";}
	virtual void	takeDamage(	g_entity_c *inflictor, 	g_entity_c *attacker, 
					vec3_t dir, vec3_t point, vec3_t normal, 
					int damage, int knockback, int dflags, int mod	);
	
	// destroy this entity at the end of the current or next frame
	void		remove();
	

	// dictionary functions
	void		setEPairs(const epairs_t &epairs);
	bool		hasKey(const std::string &key) const;
	// returns "" if not present
	const char*	getValueForKey(const std::string &key) const;
	// returns 0 if not present
	vec_t		getFloatForKey(const std::string &key) const;
	// assigns vec3_origin if not present
	void 		getVector3ForKey(const std::string &key, vec3_c &v) const;
	
	void		updateField(const std::string &key);
	
	// helper functions to update the entity network state
#if defined(ODE)
	void		updateOrigin();
	void		updateRotation();
	void		updateVelocity();
#endif
	
	// physics helper functions
	void		addGravity();
	void		applyLinearVelocity();
	void		applyAngularVelocity();
	g_entity_c*	checkPosition();
	void		checkVelocity();
	void		checkGround();
	int		clipVelocity(const vec3_c &in, const vec3_c &normal, vec3_c &out, float overbounce);
	void		impact(const trace_t &trace);
	trace_t		push(const vec3_c &push);
	bool		push2(vec3_c &move, vec3_c &amove);

	// physics controller types
	void		runPhysicsNone();
	void		runPhysicsNoclip();
	void		runPhysicsToss();
public:
	
	// Needs to be called any time an entity changes origin, mins, maxs,
	// or solid.  Automatically unlinks if needed.
	// sets _r.bbox_abs
	// sets ent->leafnums[] for pvs determination even if the entity is not solid
	void		link();

	// Call before removing an entity, and before trying to move one,
	// so it doesn't clip against itself
	void		unlink();
	
	// check if other entity is visible to this
	bool		inFront(const g_entity_c *other);
	void		touchTriggers();

	// Kills all entities that would touch the proposed new positioning of ent.
	// this entity should be unlinked before calling this!
	bool		killBox();
	void		setModel();
	void		setModel(const std::string &name);
	
	bool		getRemove() const	{return _remove;}	// called only by G_RemoveUnneededEntities
									// at the end of every frame
	float		getSpawnTime() const	{return _time_spawn;}
	const char*	getClassName() const	{return _classname.c_str();}

private:
	bool		_remove;			// don't kill objects using delete, just do _remove = true;
	float		_time_spawn;			// only set by constructor
	
	epairs_t	_epairs;

protected:
	std::string	_classname;

public:	
	int		_thinktype;
	movetype_t	_movetype;
	int		_flags;

	std::string	_model;
	
	std::string	_message;
	int		_spawnflags;

	vec3_c		_angles;
	float		_angle;				// set in qe3, -1 = up, -2 = down
	matrix_c	_rotation;
	
	// targeting variables
	std::string	_target;
	std::string	_target1;
	std::string	_target2;
	std::string	_targetname;
	std::string	_killtarget;
	std::string	_team;
	std::string	_pathtarget;
	std::string	_deathtarget;
	std::string	_combattarget;
	g_entity_c*	_target_ent;

	// mover variables
	float		_speed;
	float		_accel;
	float		_decel;
	vec3_c		_movedir;
	
	float		_mass;
	float		_gravity;			// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares
	// misc
	g_entity_c*	_goalentity;
	float		_yaw_speed;
	float		_ideal_yaw;

	// timing variables
	int		_time_nextthink;		// time when this entity will think again

	int		_time_wait;
	int		_time_delay;			// before firing targets
	int		_time_random;

	int		_time_touch_debounce;		// are all these legit?  do we need more/less of them?
	int		_time_pain_debounce;
	int		_time_damage_debounce;
	int		_time_fly_sound_debounce;	// move to clientinfo
	int		_time_last_move;

	int		_health;
	int		_max_health;
	int		_deadflag;

	int		_takedamage;
	int		_dmg;
	int		_radius_dmg;
	float		_dmg_radius;
	int		_sounds;			// make this a spawntemp var?
	int		_count;

	g_entity_c*	_chain;
	g_entity_c*	_enemy;
	g_entity_c*	_activator;

	// ground variables
	g_entity_c*	_groundentity;
	int		_groundentity_linkcount;
	
	g_entity_c*	_teamchain;
	g_entity_c*	_teammaster;

	// general sound stuff
	int		_noise_index;
	int		_noise_index2;
	float		_volume;
	float		_attenuation;

	// underwater variables
	int		_watertype;
	int		_waterlevel;

//	g_item_c*	_item;				// for bonus items

	// experimental Open Dynamics Engine stuff
#if defined(ODE)

	// rigid body
	d_body_c*		_body;

	// collision detection
	d_space_c*		_space;			// used only when we need a group of geoms
	std::vector<d_geom_c*>	_geoms;
	std::map<d_geom_c*, g_geom_info_c*>	_geoms;
#endif
};



#endif // G_ENTITY_H
