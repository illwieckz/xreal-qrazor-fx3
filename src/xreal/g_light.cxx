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
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "g_light.h"
#include "g_local.h"


/*
================================================================================
				LIGHT
================================================================================
*/

/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/


#define START_OFF	1

g_light_c::g_light_c()
:g_entity_c(false)
{
	_s.effects = EF_DLIGHT;
	_s.renderfx = RF_STATIC;
	
	/*
	_s.shaderparms[0] = 1.0;
	_s.shaderparms[1] = 1.0;
	_s.shaderparms[2] = 1.0;
	_s.shaderparms[3] = 1.0;
	*/
	
	//_r.clipmask = MASK_SHOT;
	//_r.solid = SOLID_BBOX;
	//_r.bbox._mins.set(-32,-32, -24);
	//_r.bbox._maxs.set( 32, 32, -16);
	
	_light_target.clear();
	_light_right.clear();
	_light_up.clear();

	addField(g_field_c("light_origin", &_s.origin, F_VECTOR));
	addField(g_field_c("light_center", &_s.origin2, F_VECTOR));
	addField(g_field_c("light_rotation", &_s.quat, F_ROTATION_TO_QUATERNION));
	addField(g_field_c("light_radius", &_s.vectors[0], F_VECTOR));
//	addField(g_field_c("light", &_s.light, F_FLOAT));
	
	addField(g_field_c("texture", &_texture, F_STRING));
	
	addField(g_field_c("light_target", &_light_target, F_VECTOR));
	addField(g_field_c("light_right", &_light_right, F_VECTOR));
	addField(g_field_c("light_up", &_light_up, F_VECTOR));
	
	addField(g_field_c("falloff", &_falloff, F_INT));
	
	addField(g_field_c("nodiffuse", &_nodiffuse, F_INT));
	addField(g_field_c("nospecular", &_nospecular, F_INT));
	addField(g_field_c("noshadows", &_noshadows, F_INT));
}

void	g_light_c::think()
{
	// check for a direction
	if(_target.length())
	{
		g_entity_c *target = G_PickTarget(_target);
		if(!target)
		{
			gi.Com_Printf("g_light_c::think: can't find target %s\n", _target.c_str());
			remove();
			return;
		}
		
		vec3_c dir =  target->_s.origin - _s.origin;
		dir.normalize();
		Vector3_ToAngles(dir, _angles);
		_s.quat.fromAngles(_angles);
//		_body->setQuaternion(_s.quat);
	}

	// HACK HACK HACK
#if 0
	if(X_strcaseequal(_texture.c_str(), "lights/dotsCubeLight"))
//	if(X_strcaseequal(_texture.c_str(), "lights/fanlightgrate"))
	{
		_s.quat.multiplyRotation(0, 1, 0);
		_body->setQuaternion(_s.quat);
	}
#endif

	_nextthink = level.time + FRAMETIME;
}

/*
void	g_light_c::touch(g_entity_c *other, cplane_c *plane, csurface_c *surf)
{
	//_remove = true;
}
*/

void	g_light_c::die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point)
{
	gi.Com_Printf("g_light_c::die: inflictor '%s'\n", inflictor->getClassName());

	_health = _max_health;
	_takedamage = DAMAGE_NO;
	
	_s.shaderparms[6] = 1.0;
	_s.shaderparms[7] = 0.0;
}


/*
static void light_use (g_entity_c *self, g_entity_c *other, g_entity_c *activator)
{
	if (self->spawnflags & START_OFF)
	{
		gi.configstring (CS_LIGHTS+self->style, "m");
		self->spawnflags &= ~START_OFF;
	}
	else
	{
		gi.configstring (CS_LIGHTS+self->style, "a");
		self->spawnflags |= START_OFF;
	}
}
*/

void	g_light_c::activate()
{
	_r.inuse = true;
	
#if 0
	if(_model.length())
	{
		G_SetModel(this, _model);
		
		_s.origin = _r.bbox.origin();
		_s.vectors[0] = _r.bbox.size();
		_s.vectors[0].scale(1.5);
	}
#endif
	// update fields that may conflict with others
	updateField("light_origin");
	updateField("light_rotation");
	
	if(	_light_target != vec3_origin	||
		_light_right != vec3_origin	||
		_light_up != vec3_origin
	)
	{
		_s.type = ET_LIGHT_PROJ;
		
		if(_texture.length())
			_s.index_light = gi.SV_LightIndex(_texture);
		else
			_s.index_light = gi.SV_LightIndex("lights/defaultProjectedLight");
			
		_s.vectors[0] = _light_target;
		_s.vectors[1] = _light_right;
		_s.vectors[2] = _light_up;
	}
	else
	{
		_s.type = ET_LIGHT_OMNI;
		
		if(_texture.length())
			_s.index_light = gi.SV_LightIndex(_texture);
		else
			_s.index_light = gi.SV_LightIndex("lights/defaultPointLight");
	
		if(_s.vectors[0].isZero())
		{
			//if(_s.light)
			//	_s.vectors[0].set(_s.light, _s.light, _s.light);
			//else
				_s.vectors[0].set(300, 300, 300);
		}
	}
	
	if(_health)
	{
		_max_health = _health;
		_takedamage = DAMAGE_YES;
	}
	
	/*
		// setup ODE collision
		g_geom_info_c *geom_info = new g_geom_info_c(this, NULL, NULL);
	
		d_geom_c *geom = new d_box_c(g_ode_space->getId(), _s.vectors[0] * 0.5);
		geom->setPosition(_s.origin);
		geom->setQuaternion(_s.quat);
		geom->setData(geom_info);
	
		_geoms.insert(std::make_pair(geom, geom_info));
	*/
	
	
	
	
	// setup ODE rigid body
	/*
	_body->setPosition(_s.origin);
	_body->setQuaternion(_s.quat);
	_body->setGravityMode(0);
	
	dMass m;
	dMassSetSphereTotal(&m, 0.1, 8);
	_body->setMass(&m);
	*/
	
	_nextthink = level.time + FRAMETIME;
}

void	SP_light(g_entity_c **entity)
{
	g_entity_c *ent = new g_light_c();
	*entity = ent;
}

