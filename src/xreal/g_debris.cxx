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
#include "g_debris.h"
#include "g_local.h"


/*
================================================================================
				DEBRIS
================================================================================
*/


g_debris_c::g_debris_c(g_entity_c *ent, const std::string &modelname, float speed, const vec3_c &origin)
{
	_r.inuse = true;
	
	G_SetModel(this, modelname);
	
	_s.origin = origin;
	
	_classname = "debris";
	
	vec3_c v(	100 * crandom(),
			100 * crandom(),
			100 + 100 * crandom());
	
	//FIXME
	
	Vector3_MA(ent->_s.velocity_linear, speed, v, _s.velocity_linear);
	_movetype = MOVETYPE_BOUNCE;
	_r.solid = SOLID_NOT;
	/*
	_avelocity[0] = random()*600;
	_avelocity[1] = random()*600;
	_avelocity[2] = random()*600;
	*/
	_nextthink = level.time + 5000 + (int)((random()*5)*1000);
	
	_takedamage = DAMAGE_YES;
}

void	g_debris_c::think()
{
	remove();
}

void	g_debris_c::die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point)
{
	remove();
}

void	G_ThrowDebris(g_entity_c *self, const std::string &modelname, float speed, const vec3_c &origin)
{
	new g_debris_c(self, modelname, speed, origin);
}

