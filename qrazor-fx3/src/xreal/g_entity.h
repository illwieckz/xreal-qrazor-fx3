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
#include "../shared/x_shared.h"
#include "../shared/x_ode.h"

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
	MOVETYPE_ODE_TOSS
};


//
// forward declarations
//
class g_item_c;


class g_geom_info_c
{
public:
	g_geom_info_c(g_entity_c *ent, cmodel_c *model, csurface_c *surf)
	{
		_ent	= ent;
		_model	= model;
		_surf	= surf;
	}
	
	g_entity_c*	getEntity() const	{return _ent;}
	cmodel_c*	getModel() const	{return _model;}
	csurface_c*	getSurface() const	{return _surf;}
	
private:
	g_entity_c*	_ent;
	cmodel_c*	_model;
	csurface_c*	_surf;
};


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
	
	virtual void	think() {};
	virtual void	blocked(g_entity_c *other) {};
	virtual bool	touch(g_entity_c *other, const cplane_c &plane, csurface_c *surf) {return true;}
	virtual void	use(g_entity_c *other, g_entity_c *activator) {};
	virtual void	pain(g_entity_c *other, float kick, int damage) {};
	virtual void	die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point) {};
	
	virtual void	activate() {};
	
	virtual std::string	clientTeam()	{return "";}
	virtual void	takeDamage(	g_entity_c *inflictor, 	g_entity_c *attacker, 
					vec3_t dir, vec3_t point, vec3_t normal, 
					int damage, int knockback, int dflags, int mod	);
						
	void		remove();					// mark for removal from world
	bool		hasEPair(const std::string &key);
	const char*	valueForKey(const std::string &key);
	
	// helper functions to update the entity network state
	void		updateOrigin();
	void		updateRotation();
	void		updateVelocity();
	
	// check if other entity is visible to this
	bool		inFront(const g_entity_c *other);
	
	bool		getRemove() const	{return _remove;}	// called only by G_RemoveUnneededEntities
									// at the end of every frame
									
	float		getSpawnTime() const	{return _spawntime;}
	
	const char*	getClassName() const	{return _classname.c_str();}
	
	//TODO	

private:
	bool		_remove;			// don't kill objects using delete, just do _remove = true;
	float		_spawntime;			// only set by constructor
	
	std::map<std::string, std::string>	_epairs;

protected:
	std::string	_classname;

public:	
	movetype_t	_movetype;
	int		_flags;

	std::string	_model;
	
	std::string	_message;
	int		_spawnflags;

	vec3_c		_angles;
	float		_angle;				// set in qe3, -1 = up, -2 = down
	matrix_c	_rotation;
	
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

	float		_speed;
	float		_accel;
	float		_decel;
	vec3_c		_movedir;
	
	float		_mass;
	float		_gravity;			// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	g_entity_c*	_goalentity;
	float		_yaw_speed;
	float		_ideal_yaw;

	float		_nextthink;			// time when this entity will think again	

	float		_touch_debounce_time;		// are all these legit?  do we need more/less of them?
	float		_pain_debounce_time;
	float		_damage_debounce_time;
	float		_fly_sound_debounce_time;	// move to clientinfo
	float		_last_move_time;

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
	g_entity_c*	_groundentity;
	
	g_entity_c*	_teamchain;
	g_entity_c*	_teammaster;

	

	// general sound stuff
	int		_noise_index;
	int		_noise_index2;
	float		_volume;
	float		_attenuation;

	// timing variables
	float		_wait;
	float		_delay;				// before firing targets
	float		_random;

	int		_watertype;
	int		_waterlevel;


	g_item_c*	_item;				// for bonus items

		
	// rigid body
	d_body_c*		_body;

	// collision detection
	d_space_c*				_space;			// used only when we need a group of geoms
	std::map<d_geom_c*, g_geom_info_c*>	_geoms;
};



#endif // G_ENTITY_H
