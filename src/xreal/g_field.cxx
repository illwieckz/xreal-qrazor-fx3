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
#include "g_field.h"
#include "g_local.h"


g_field_iface_a::g_field_iface_a()
{
}

void	g_field_iface_a::addField(const g_field_c &f)
{
	_fields.push_back(f);
}


/*
====================
G_SetEntityFields

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
void	g_field_iface_a::setFields(const std::map<std::string, std::string> &map)
{
	for(std::map<std::string, std::string>::const_iterator ir = map.begin(); ir != map.end(); ++ir)
	{
		setField(ir->first, ir->second);
	}
}

static std::string	G_NewString(const std::string &s)
{
	std::string newb;

	int		i,l;
	
	l = s.length();

	//newb = (char*)trap_Z_TagMalloc (l, TAG_LEVEL);
	//newb = (char*)trap_Z_Malloc(l);

	//new_p = newb;
	
	
	for(i=0 ; i< l ; i++)
	{
		if(s[i] == '\\' && i < l-1)
		{
			i++;
			if(s[i] == 'n')
				newb += '\n';
			else
				newb += '\\';
		}
		else
			newb += s[i];
	}
	
	//TODO
	
	return s;
}




/*
===============
ED_ParseField

Takes a key/value pair and sets the binary values
in an edict
===============
*/
void	g_field_iface_a::setField(const std::string &key, const std::string &value)
{
	float		v;
	vec3_c		vec;
	matrix_c	m;
	
//	trap_Com_Printf("g_field_iface_a:setField: checking for '%s' '%s'\n", key.c_str(), value.c_str());

	for(std::deque<g_field_c>::iterator ir = _fields.begin(); ir != _fields.end(); ++ir)
	{
		g_field_c& f = *ir;
		
		if(!(f._flags & FFL_NOSPAWN) && X_strcaseequal(f._name.c_str(), key.c_str()))
		{
			if(f._flags & FFL_SPAWNTEMP)
				continue;
			
			switch(f._type)
			{
				case F_INT:
					*(int*)(f._ptr) = atoi(value.c_str());
					break;
					
				case F_FLOAT:
					*(float*)(f._ptr) = atof(value.c_str());
					break;
			
				case F_STRING:
					(*(std::string*)(f._ptr)) = G_NewString(value);
					break;
					
				case F_VECTOR:
					sscanf(value.c_str(), "%f %f %f", (float*)&vec[0], (float*)&vec[1], (float*)&vec[2]);
					((float*)(f._ptr))[0] = vec[0];
					((float*)(f._ptr))[1] = vec[1];
					((float*)(f._ptr))[2] = vec[2];
					break;
				
				case F_COLOR:
					sscanf(value.c_str(), "%f %f %f %f", (float*)&vec[0], (float*)&vec[1], (float*)&vec[2], (float*)&vec[3]);
					((float*)(f._ptr))[0] = vec[0];
					((float*)(f._ptr))[1] = vec[1];
					((float*)(f._ptr))[2] = vec[2];
					((float*)(f._ptr))[3] = vec[3];
					break;
					
				case F_ANGLEHACK_TO_QUATERNION:
					v = atof(value.c_str());
					((quaternion_c*)(f._ptr))->fromAngles(0, v, 0);
					break;
					
				case F_ANGLES_TO_QUATERNION:
					sscanf(value.c_str(), "%f %f %f", (float*)&vec[0], (float*)&vec[1], (float*)&vec[2]);
					((quaternion_c*)(f._ptr))->fromAngles(vec[0], vec[1], vec[2]);
					break;
					
				case F_ROTATION_TO_QUATERNION:
					m.identity();
					sscanf(value.c_str(), "%f %f %f %f %f %f %f %f %f",	&m[0][0], &m[0][1], &m[0][2],
												&m[1][0], &m[1][1], &m[1][2],
												&m[2][0], &m[2][1], &m[2][2]);
					//m.transpose();
					((quaternion_c*)(f._ptr))->fromMatrix(m);
					break;
					
				case F_IGNORE:
					break;

				default:
					break;
			}
			return;
		}
	}
	

	trap_Com_Printf("%s is not a field\n", key.c_str());
}

