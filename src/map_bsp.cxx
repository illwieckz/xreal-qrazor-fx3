/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#include "map_local.h"

// xreal --------------------------------------------------------------------

namespace map
{


face_c::face_c(plane_p plane, winding_p w, int compileflags)
{
	if(!plane)
		Com_Error(ERR_FATAL, "face_c::ctor: NULL plane");
		
	if(!w)
		Com_Error(ERR_FATAL, "face_c::ctor: NULL winding");

	_plane		= plane;
	_winding	= new winding_c(*w);
	setCompileFlags(compileflags);
}

face_c::~face_c()
{
	if(_winding)
		delete _winding;
}


node_c::node_c(const aabb_c& aabb, node_p parent)
{
	_parent		= parent;
	_aabb		= aabb;
	
	_children[SIDE_FRONT]	= NULL;
	_children[SIDE_BACK]	= NULL;
}

void	node_c::splitAABB(const plane_c& p)
{
	if(p.getType() <= PLANE_Z)
	{
		_aabb._mins[p.getType()] = p._dist;
		_aabb._maxs[p.getType()] = p._dist;
	}
}



tree_c::tree_c()
{
	_node_head	= NULL;
	
	_aabb.clear();
}

face_v	tree_c::buildStructuralFaceList(const brush_v& brushes) const
{
	face_v faces;
	
	for(brush_ci i = brushes.begin(); i != brushes.end(); ++i)
	{
		const brushside_v& sides = (*i)->getSides();
		
		for(brushside_ci j = sides.begin(); j != sides.end(); ++j)
		{
			brushside_p s = (*j);
			
			if(!s)
				continue;
			
			winding_p w = s->getWinding();
		
			if(!w)
				continue;
				
			
			if(s->getShader()->hasCompileFlags(C_SKIP))
				continue;
			
			// FIXME
			face_p f = new face_c(s->getPlane(), w, s->getShader()->getCompileFlags());
			
			faces.push_back(f);
		}
	}

	return faces;
}

void	tree_c::buildBSP(face_v& faces)
{
	Com_DPrintf("------- tree_c::buildBSP -------\n");
	
	_aabb.clear();
	for(face_ci i = faces.begin(); i != faces.end(); ++i)
	{
		const face_p f = (*i);
		
		winding_p w = f->getWinding();
		
		_aabb.mergeWith(w->calcAABB());
	}
	Com_DPrintf("%i faces\n", faces.size());
	
	_node_head = new node_c(_aabb);
	
	c_faceleafs = 0;
	buildFaceTree_r(_node_head, faces);
	Com_DPrintf("%i faceleafs\n", c_faceleafs);
}

face_i	tree_c::selectSplitFace(node_p node, face_v& faces) const
{
	//TODO some BSP heuristic

	return faces.end();
}

void	tree_c::buildFaceTree_r(node_p node, face_v& faces)
{
	face_i split_selected = selectSplitFace(node, faces);
	if(split_selected == faces.end())
	{
		// this is a leaf node
		node->setPlane(NULL);
		c_faceleafs++;
		return;
	}
	
	// partition the faces
	plane_p		split_plane = (*split_selected)->getPlane();
	node->setPlane(split_plane);
	
	// multiple surfaces, so split all the polysurfaces into front and back lists
	face_v faces_front;
	face_v faces_back;
	
	for(face_i i = split_selected; i != faces.end(); ++i)
	{
		face_p split = (*i);
		
		if(split->getPlane() == node->getPlane())
		{
			delete split;
			continue;
		}
		
		plane_p		split_plane = split->getPlane();
		winding_p	split_winding = split->getWinding();
		
		plane_side_e side = split_winding->onPlaneSide(split_plane->_normal, split_plane->_dist);
		
		winding_p	winding_front = NULL;
		winding_p	winding_back = NULL;
		
		switch(side)
		{
			case SIDE_FRONT:
			{
				faces_front.push_back(split);
				break;
			}
			case SIDE_BACK:
			{
				faces_back.push_back(split);
				break;
			}
			case SIDE_CROSS:
			{
				split_winding->clip(*split_plane, &winding_front, &winding_back, CLIP_EPSILON * 2);
				
				if(winding_front)
				{
					face_p face = new face_c(split_plane, winding_front, 0);
					faces_front.push_back(face);
				}
				
				if(winding_back)
				{
					face_p face = new face_c(split_plane, winding_back, 0);
					faces_back.push_back(face);
				}
				
				delete split;
			}
			
			default:
				break;
		}
	}
	
	node_p child_front = new node_c(node->getAABB(), node);
	node_p child_back = new node_c(node->getAABB(), node);
	
	child_front->splitAABB(*split_plane);
	child_back->splitAABB(*split_plane);
	
	node->setFrontChild(child_front);
	node->setBackChild(child_back);
	
	buildFaceTree_r(child_front, faces_front);
	buildFaceTree_r(child_back, faces_back);
}

void	tree_c::buildPortals()
{
	//TODO
}

} // namespace map

