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
#include <deque>
// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_entity.h"
#include "g_target.h"


void	G_ProjectSource(const vec3_c &point, const vec3_c &distance, const vec3_c &forward, const vec3_c &right, vec3_c &result)
{
	result[0] = point[0] + forward[0] * distance[0] + right[0] * distance[1];
	result[1] = point[1] + forward[1] * distance[0] + right[1] * distance[1];
	result[2] = point[2] + forward[2] * distance[0] + right[2] * distance[1] + distance[2];
}


bool 	G_AreasConnected(const vec3_c &p1, const vec3_c &p2)
{
	int area1 = trap_CM_PointAreanum(p1);

	int area2 = trap_CM_PointAreanum(p2);
	
	if(!trap_CM_AreasConnected(area1, area2))
		return false;		// a door blocks sight
	
	return true;
}


/*
=============
G_IsVisible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
/*
bool	G_IsVisible(g_entity_c *self, g_entity_c *other)
{
	vec3_c	spot1;
	vec3_c	spot2;
	trace_t	trace;
	cbbox_c	bbox;

	spot1 = self->_s.origin;
	//spot1[2] += self->_viewheight;
	
	spot2 = other->_s.origin;
	//spot2[2] += other->_viewheight;
	trace = trap_SV_Trace(spot1, bbox, spot2, self, MASK_OPAQUE);
	
	if(trace.fraction == 1.0)
		return true;
		
	return false;
}
*/

/*
=============
G_Find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
/*
g_entity_c*	G_Find(g_entity_c *from, int fieldofs, char *match)
{
	char	*s;
	unsigned int	i;

	if (!from)
		i = 0;	//g_entities[0];
	else
	{
		i = G_GetNumForEdict(from)+1;	//from++;	
	}

	for ( ; i < g_entities.size() ; i++)
	{
		from = g_entities[i];
	
		if (!from->r.inuse)
			continue;
			
		s = *(char **) ((byte *)from + fieldofs);

		if (!s)
			continue;
			
		if (!X_stricmp (s, match))
			return from;
	}

	return NULL;
}
*/

g_entity_c*	G_FindByClassName(g_entity_c *from, const std::string &match)
{
	unsigned int	i;

	if(!from)
		i = 0;	//g_entities[0];
	else
	{
		i = G_GetNumForEntity(from)+1;	//from++;	
	}

	for(; i < g_entities.size(); i++)
	{
		from = (g_entity_c*)g_entities[i];
		
		if(!from)
			continue;
		
		if(!from->_r.inuse)
			continue;

		if(!X_stricmp(from->getClassName(), match.c_str()))
			return from;
	}

	return NULL;
}


g_entity_c*	G_FindByTargetName(g_entity_c *from, const std::string &match)
{
	unsigned int	i;

	if(!from)
		i = 0;	//g_entities[0];
	else
	{
		i = G_GetNumForEntity(from)+1;	//from++;	
	}

	for(; i < g_entities.size(); i++)
	{
		from = (g_entity_c*)g_entities[i];
		
		if(!from)
			continue;
		
		if(!from->_r.inuse)
			continue;

		if(!X_stricmp(from->_targetname.c_str(), match.c_str()))
			return from;
	}

	return NULL;
}


g_entity_c*	G_FindByRadius(g_entity_c *from, vec3_t org, float rad)
{
	vec3_t	eorg;
	unsigned int		i, j;

	if (!from)
		i = 0;	//g_entities[0];
	else
	{
		i = G_GetNumForEntity(from)+1;	//from++;	
	}

	for(; i < g_entities.size(); i++)
	{
		from = (g_entity_c*)g_entities[i];
		
		if(!from)
			continue;
	
		if (!from->_r.inuse)
			continue;
			
		if (from->_r.solid == SOLID_NOT)
			continue;
			
		for (j=0 ; j<3 ; j++)
			eorg[j] = org[j] - (from->_s.origin[j] + (from->_r.bbox._mins[j] + from->_r.bbox._maxs[j])*0.5);
			
		if (Vector3_Length(eorg) > rad)
			continue;
			
		return from;
	}

	return NULL;
}


/*
=============
G_PickTarget

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the edict after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
#define MAXCHOICES	8

g_entity_c*	G_PickTarget(const std::string &targetname)
{
	g_entity_c	*ent = NULL;
	int		num_choices = 0;
	g_entity_c	*choice[MAXCHOICES];

	if(!targetname.length())
	{
		trap_Com_Printf("G_PickTarget called with NULL targetname\n");
		return NULL;
	}

	while(1)
	{
		ent = G_FindByTargetName(ent, targetname);
		if (!ent)
			break;
		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if(!num_choices)
	{
		trap_Com_Printf("G_PickTarget: target %s not found\n", targetname.c_str());
		return NULL;
	}

	return choice[rand() % num_choices];
}





/*
==============================
G_UseTargets

the global "activator" should be set to the entity that initiated the firing.

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Centerprints any self.message to the activator.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void	G_UseTargets(g_entity_c *ent, g_entity_c *activator)
{
	g_entity_c	*target;

	// check for a delay
	if(ent->_time_delay)
	{
		// create a temp object to fire at a later time
		new g_target_delay_c(ent, activator);
		return;
	}
	
	// print the message
	if(ent->_message.length())
	{
		trap_SV_CenterPrintf(activator, "%s", ent->_message.c_str());
		
		if(ent->_noise_index)
			trap_SV_StartSound (NULL, activator, CHAN_AUTO, ent->_noise_index, 1, ATTN_NORM, 0);
		else
			trap_SV_StartSound (NULL, activator, CHAN_AUTO, trap_SV_SoundIndex("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}


	// kill killtargets
	if(ent->_killtarget.length())
	{
		target = NULL;
		
		while((target = G_FindByTargetName(target, ent->_killtarget)))
		{
			target->remove();
			
			if(!ent->_r.inuse)
			{
				trap_Com_Printf("entity was removed while using killtargets\n");
				return;
			}
		}
	}

	// fire targets
	if(ent->_target.length())
	{
		target = NULL;
		
		while((target = G_FindByTargetName(target, ent->_target)))
		{
			// doors fire area portals in a specific way
			if(	!X_stricmp(target->getClassName(), "func_areaportal") && (!X_stricmp(ent->getClassName(), "func_door") || 
				!X_stricmp(ent->getClassName(), "func_door_rotating")))
				continue;

			if(target == ent)
			{
				trap_Com_Printf ("WARNING: Entity used itself.\n");
			}
			else
			{
				target->use(ent, activator);
			}
			
			if(!ent->_r.inuse)
			{
				trap_Com_Printf("entity was removed while using targets\n");
				return;
			}
		}
	}
}


/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
vec_t*	tv(float x, float y, float z)
{
	static	int		index;
	static	vec3_t	vecs[8];
	vec_t	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}




static const vec3_c VEC_UP(0, -1, 0);
static const vec3_c MOVEDIR_UP(0.0, 0, 1);
static const vec3_c VEC_DOWN(0.0, -2, 0);
static const vec3_c MOVEDIR_DOWN(0.0, 0, -1);

void	G_SetMovedir(quaternion_c &quat, vec3_c &movedir)
{
	/*
	if(angles == VEC_UP)
	{
		movedir = MOVEDIR_UP;
	}
	else if(angles == VEC_DOWN)
	{
		movedir = MOVEDIR_DOWN;
	}
	else
	{
		//move forward
		vec3_c	right, up;
		Angles_ToVectors(angles, movedir, right, up);
	}

	angles.clear();
	*/
	
	vec3_c		forward, right, up;
	matrix_c	matrix;
	
	matrix.fromQuaternion(quat);
	matrix.toVectorsFRU(forward, right, up);
	
	movedir = forward;
}


float	vectoyaw(vec3_t vec)
{
	float	yaw;
	
	if (/*vec[YAW] == 0 &&*/ vec[PITCH] == 0) 
	{
		yaw = 0;
		if (vec[YAW] > 0)
			yaw = 90;
		else if (vec[YAW] < 0)
			yaw = -90;
	} 
	else
	{
		yaw = (int) (atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);
		if (yaw < 0)
			yaw += 360;
	}

	return yaw;
}


void	vectoangles(vec3_t value1, vec3_t angles)
{
	float	forward;
	float	yaw, pitch;
	
	if (value1[1] == 0 && value1[0] == 0)
	{
		yaw = 0;
		if (value1[2] > 0)
			pitch = 90;
		else
			pitch = 270;
	}
	else
	{
		if (value1[0])
			yaw = (int) (atan2(value1[1], value1[0]) * 180 / M_PI);
		else if (value1[1] > 0)
			yaw = 90;
		else
			yaw = -90;
		if (yaw < 0)
			yaw += 360;

		forward = X_sqrt(value1[0]*value1[0] + value1[1]*value1[1]);
		pitch = (int) (atan2(value1[2], forward) * 180 / M_PI);
		if (pitch < 0)
			pitch += 360;
	}

	angles[PITCH] = -pitch;
	angles[YAW] = yaw;
	angles[ROLL] = 0;
}






/*
=================
KillBox

Kills all entities that would touch the proposed new positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
bool	G_KillBox(g_entity_c *ent)
{
#if 0
	trace_t		tr;
	
	trap_Com_Printf("G_KillBox: %s\n", ent->_classname.c_str());

	while(1)
	{
		tr = trap_SV_Trace(ent->_s.origin, ent->_r.bbox, ent->_s.origin, NULL, MASK_PLAYERSOLID);
		
		if(!tr.ent)
			break;

		// nail it
		((g_entity_c*)tr.ent)->takeDamage(ent, ent, vec3_origin, ent->_s.origin, vec3_origin, 100000, 0, DAMAGE_NO_PROTECTION, MOD_TELEFRAG);

		// if we didn't kill it, fail
		if(tr.ent->_r.solid)
			return false;
	}

	return true;		// all clear
#else
	return false;
#endif
}




g_entity_c*	G_GetEntityByNum(int num)
{
	if(num < 0 || num >= (int)g_entities.size())
	{
		trap_Com_Error(ERR_DROP, "G_GetEntityByNum: bad number %i\n", num);
		return NULL;
	}

	return (g_entity_c*)g_entities[num];
}

int	G_GetNumForEntity(g_entity_c *ent)
{
	if(!ent)
	{
		trap_Com_Error(ERR_DROP, "G_GetNumForEntity: NULL parameter\n");
		return -1;
	}

	for(unsigned int i=0; i<g_entities.size(); i++)
	{
		if(ent == g_entities[i])
			return i;
	}
	
	trap_Com_Error(ERR_DROP, "G_GetNumForEntity: bad pointer of entity %i\n", ent->_s.getNumber());
	return -1;
}




