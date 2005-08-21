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

#include "cm.h"


/*
void	SV_SetAreaPortalState(sv_entity_c *ent, bool open)
{
	int	leafs[MAX_TOTAL_ENT_LEAFS];
	int	leafs_num;
	int	i;
	int	areaportal = 0;
	
	// entity must touch at least two areas
	if(!ent->_r.area || !ent->_r.area2)
		return;
	
	// get all leafs, including solids
	leafs_num = CM_BoxLeafnums(ent->_r.bbox, leafs, MAX_TOTAL_ENT_LEAFS, NULL);
	
	if(!leafs_num)
		return;
	
	for(i=0; i<leafs_num; i++)
	{
		if(!CM_LeafCluster(leafs[i]))
			continue;
		
		if(CM_LeafContents(leafs[i]) & CONTENTS_AREAPORTAL)
		{
			areaportal = leafs[i];
			break;
		}
	}
	
	if(!areaportal)
		return;
	

	CM_SetAreaPortalState(ent->_r.portal, open);
}
*/

int	SV_PointContents(const vec3_c &p)
{
#if 0
	entity_c		*touch[MAX_ENTITIES], *hit;
	int			i, num;
	int			contents, c2=0;
	int			headnode;
	quaternion_c		quat;
	
	cbbox_c		bbox;
	
	
	bbox._mins = p;
	bbox._maxs = p;

	// get base contents from world
	contents = CM_PointContents(p, 0);

	// or in contents from all the other entities
	num = SV_AreaEdicts(bbox, touch, MAX_ENTITIES, AREA_SOLID);

	for(i=0; i<num; i++)
	{
		hit = touch[i];

		// might intersect, so do an exact clip
		headnode = SV_HullForEntity(hit);
				
		if(hit->_r.solid != SOLID_BSP)
			quat.identity();			// boxes don't rotate
		else
			quat = hit->_s.quat;
	
		c2 = CM_TransformedPointContents(p, headnode, hit->_s.origin, hit->_s.quat);

		contents |= c2;
	}

	return contents;
#else
	return 0;
#endif
}

