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


#if defined(ODE)
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
#endif

