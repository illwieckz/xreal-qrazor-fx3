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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include 	"r_local.h"

// xreal --------------------------------------------------------------------


void	r_vis_iface_a::updateVis(const r_entity_t &shared)
{
	if(shared.flags & RF_STATIC)
	{
#if 0
		r_world_tree->boxAreas(shared.radius_bbox, _areas);
#else
		/*
		r_bsptree_leaf_c* leaf = r_world_tree->pointInLeaf(shared.origin);
		if(leaf)
		{
			_cluster = leaf->cluster;
		}
		else
		{
			_cluster = -1;
		}
		
		addArea(0);
		*/
		
		r_world_tree->boxLeafs(shared.radius_bbox, _leafs);
		for(std::vector<r_bsptree_leaf_c*>::iterator ir = _leafs.begin(); ir != _leafs.end(); ++ir)
		{
			r_bsptree_leaf_c *leaf = *ir;
			
			if(leaf->cluster < 0)
				continue;
			
			addArea(leaf->area);
		}
#endif
	}
	else
	{
		r_bsptree_leaf_c* leaf = r_world_tree->pointInLeaf(shared.origin);
		if(leaf)
		{
			_cluster = leaf->cluster;
			
			addArea(leaf->area);
		}
		else
		{
			_cluster = -1;
		}
	}
}
