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
#ifndef G_FIELD_H
#define G_FIELD_H

/// includes ===================================================================
// system -------------------------------------------------------------------
#include <deque>

// xreal --------------------------------------------------------------------
// shared -------------------------------------------------------------------
#include "../x_shared.h"


//
// fields are needed for spawning from the entity string
//
enum
{
	FFL_NONE,
	FFL_SPAWNTEMP	= 1,	//		1
	FFL_NOSPAWN	= 2	//		2
};

enum fieldtype_t
{
	F_BOOL,
	F_BYTE,
	F_INT, 
	F_FLOAT,
	F_STRING,		// string on disk, pointer in memory
	F_VECTOR,
	F_COLOR,
	F_ANGLEHACK_TO_QUATERNION,
	F_ANGLES_TO_QUATERNION,
	F_ROTATION_TO_QUATERNION,
	F_EDICT,		// index on disk, pointer in memory
	F_ITEM,			// index on disk, pointer in memory
	F_CLIENT,		// index on disk, pointer in memory
	F_FUNCTION,
	F_MMOVE,
	F_ARRAY,		// array of bytes, used for handling undefined types or static array strings
	F_IGNORE
};


class g_field_c
{
public:
	g_field_c(const std::string &name, void *ptr, fieldtype_t type)
	{
		_name = name;
		_ptr = ptr;
		_type = type;
		_flags = FFL_NONE;
	}

	
	g_field_c(const std::string &name, void *ptr, fieldtype_t type, int flags)
	{
		_name = name;
		_ptr = ptr;
		_type = type;
		_flags = FFL_NONE | flags;
	}
	
	std::string	_name;
	void*		_ptr;
	fieldtype_t	_type;
	int		_flags;
};


class g_field_iface_a
{
protected:
	g_field_iface_a();
	
public:
	void			addField(const g_field_c &f);
	void			setField(const std::string &key, const std::string &value);
	void			setFields(const std::map<std::string, std::string> &epairs);
	
//	virtual void		writeToStream(VFILE *stream) = 0;
//	virtual void		readFromStream(VFILE *stream) = 0;

private:
	std::deque<g_field_c>			_fields;
};





#endif // G_FIELD_H
