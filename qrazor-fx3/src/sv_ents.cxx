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
#include "sv_local.h"


sv_entity_c*	SV_GetEntityByNum(int num)
{
	if(num < 0 || num >= (int)ge->entities->size())
	{
		Com_Error(ERR_FATAL, "SV_GetEntityByNum: bad number %i\n", num);
		return NULL;
	}

	return (*ge->entities)[num];

}

int	SV_GetNumForEntity(sv_entity_c *ent)
{
	if(!ent)
	{
		Com_Error(ERR_FATAL, "SV_GetNumForEntity: NULL parameter\n");
		return -1;
	}


	for(unsigned int i=0; i<ge->entities->size(); i++)
	{
		if(ent == (*ge->entities)[i])
			return i;
	}
	
	
	//if(num == -1)
	//	Com_Error(ERR_FATAL, "SV_GetNumForEdict: bad pointer of edict %i\n", ent->s.number);
		
	return -1;
}





