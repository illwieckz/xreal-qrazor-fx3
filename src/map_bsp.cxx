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

void	tree_c::buildBSP(const face_v& faces)
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
	
//	_node_head = new node_c(_aabb);
	
	//TODO
}

void	tree_c::buildPortals()
{
	//TODO
}

} // namespace map

