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
// qrazor-fx ----------------------------------------------------------------
#include "map_local.h"

#include "common.h"
#include "vfs.h"

// xreal --------------------------------------------------------------------

namespace map
{

void	entity_c::finish()
{
	getVector3ForKey("origin", _origin);

	calcShapeContents();
	adjustBrushesForOrigin();
}

void	entity_c::processWorldModel()
{
	tree_p tree = new tree_c();
	
	face_v faces = tree->buildStructuralFaceList();
	tree->buildBSP(faces);
	tree->buildPortals();
	
	//TODO
}

void	entity_c::processSubModel()
{
	//TODO
}


void	entity_c::setKeyValue(const std::string &key, const std::string &value)
{
	epairs_i i = _epairs.find(key);
	
	if(i != _epairs.end())
	{
		i->second = value;
	}
	else
	{
		_epairs.insert(make_pair(key, value));
	}
}

const char*	entity_c::getValueForKey(const std::string &key) const
{
	epairs_ci i = _epairs.find(key);
	
	if(i != _epairs.end())
	{
		return i->second.c_str();
	}
	else
	{
		return "";
	}
}

vec_t	entity_c::getFloatForKey(const std::string &key) const
{
	const char *k = getValueForKey(key);
	return atof(k);
}

void 	entity_c::getVector3ForKey(const std::string &key, vec3_c &v) const
{
	const char *k;
	double	v1, v2, v3;

	k = getValueForKey(key);
	
	// scanf into doubles, then assign, so it is vec_t size independent
	v1 = v2 = v3 = 0;
	
	sscanf(k, "%lf %lf %lf", &v1, &v2, &v3);
	
	v[0] = v1;
	v[1] = v2;
	v[2] = v3;
}

bool	entity_c::isWorldSpawn() const
{
	const char* classname = getValueForKey("classname");
	
	return X_strcaseequal(classname, "worldspawn");
}


void	entity_c::print() const
{
//	Com_Printf("------- entity -------\n");
	
	Com_Printf("%i brushes\n", _brushes.size());
	
	Com_Printf("%i patches\n", _patches.size());
	
	Com_Printf("%i epairs ...\n", _epairs.size());
	for(epairs_ci i = _epairs.begin(); i != _epairs.end(); ++i)
	{
		Com_Printf("'%s' = '%s'\n", i->first.c_str(), i->second.c_str());
	}
}


void	entity_c::calcShapeContents()
{
	for(brush_ci i = _brushes.begin(); i != _brushes.end(); ++i)
	{
		(*i)->calcContents();
	}
}

void	entity_c::adjustBrushesForOrigin()
{
	if(!isWorldSpawn())
	{
		for(brush_ci i = _brushes.begin(); i != _brushes.end(); ++i)
		{
			(*i)->translate(-_origin);
		}
	}
	
	for(brush_ci i = _brushes.begin(); i != _brushes.end(); ++i)
	{
		(*i)->createWindings();
	}
}


/*
================
UnparseEntities

Generates the dentdata string from all the entities
================
*/
void	UnparseEntities()
{
	std::string	entitystring;

	for(entity_ci e = entities.begin(); e != entities.end(); ++e)
	{
		entitystring += "{\n";
				
		for(std::map<std::string, std::string>::const_iterator i = (*e)->_epairs.begin(); i != (*e)->_epairs.end(); ++i)
		{
			entitystring += '\"';
			entitystring += i->first;	// key
			entitystring += '\"';
			
			entitystring += ' ';
		
			entitystring += '\"';
			entitystring += i->second;	//value
			entitystring += '\"';
			
			entitystring += '\n';
		}

		
		entitystring += "}\n";
	}
	
	for(std::string::iterator i = entitystring.begin(); i != entitystring.end(); ++i)
	{
		dentdata.push_back(*i);
	}
}

} // namespace map




