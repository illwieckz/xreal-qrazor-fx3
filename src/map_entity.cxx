/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
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


void	map_entity_c::finish()
{
	getVector3ForKey("origin", _origin);

	adjustBrushesForOrigin();
}

void	map_entity_c::adjustBrushesForOrigin()
{
	for(map_brush_ci i = _brushes.begin(); i != _brushes.end(); ++i)
	{
		map_brush_p b = *i;
		
		b->translate(-_origin);
		//b->createWindings();
	}
}

void	map_entity_c::setKeyValue(const std::string &key, const std::string &value)
{
	std::map<std::string, std::string>::iterator ir = _epairs.find(key);
			
	if(ir != _epairs.end())
	{
		ir->second = value;
	}
	else
	{
		_epairs.insert(make_pair(key, value));
	}
}

const char*	map_entity_c::getValueForKey(const std::string &key) const
{
	std::map<std::string, std::string>::const_iterator ir = _epairs.find(key);
			
	if(ir != _epairs.end())
	{
		return ir->second.c_str();
	}
	else
	{
		return "";
	}
}

vec_t	map_entity_c::getFloatForKey(const std::string &key) const
{
	const char *k = getValueForKey(key);
	return atof(k);
}

void 	map_entity_c::getVector3ForKey(const std::string &key, vec3_c &v) const
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


void	map_entity_c::toString() const
{
//	Com_Printf("------- entity -------\n");
	
	Com_Printf("%i brushes\n", _brushes.size());
	
	Com_Printf("%i patches\n", _patches.size());
	
	Com_Printf("%i epairs ...\n", _epairs.size());
	for(std::map<std::string, std::string>::const_iterator ir = _epairs.begin(); ir != _epairs.end(); ++ir)
	{
		Com_Printf("'%s' = '%s'\n", ir->first.c_str(), ir->second.c_str());
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

	for(std::vector<map_entity_c*>::const_iterator eir = map_entities.begin(); eir != map_entities.end(); ++eir)
	{
		entitystring += "{\n";
				
		for(std::map<std::string, std::string>::const_iterator ir = (*eir)->_epairs.begin(); ir != (*eir)->_epairs.end(); ++ir)
		{
			entitystring += '\"';
			entitystring += ir->first;	// key
			entitystring += '\"';
			
			entitystring += ' ';
		
			entitystring += '\"';
			entitystring += ir->second;	//value
			entitystring += '\"';
			
			entitystring += '\n';
		}

		
		entitystring += "}\n";
	}
	
	for(std::string::iterator ir = entitystring.begin(); ir != entitystring.end(); ++ir)
	{
		dentdata.push_back(*ir);
	}
}






