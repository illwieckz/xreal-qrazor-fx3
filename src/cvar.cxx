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
#include <map>

#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>

// xreal --------------------------------------------------------------------
#include "cvar.h"
#include "common.h"
#include "cmd.h"
#include "vfs.h"


bool							cvar_userinfo_modified;
static std::map<std::string, cvar_t*, strcasecmp_c>	cvar_map;

static bool	Cvar_InfoValidate(const std::string &s)
{
	if(s.find('\\') != s.npos)
		return false;
		
	if(s.find('\"') != s.npos)
		return false;
		
	if(s.find(';') != s.npos)
		return false;
		
	return true;
}


static cvar_t*	Cvar_FindVar(const std::string &name)
{
	std::map<std::string, cvar_t*>::iterator ir = cvar_map.find(name);
	if(ir != cvar_map.end())
		return ir->second;
	else
		return NULL;

}

float 	Cvar_VariableValue(const std::string &name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar(name);
	if(!var)
		return 0;
	else
		return var->getValue();
}

int	Cvar_VariableInteger(const std::string &name)
{
	cvar_t	*var;

	var = Cvar_FindVar(name);
	if(!var)
		return 0;
	else
		return var->getInteger();
}

const char*	Cvar_VariableString(const std::string &name)
{
	cvar_t *var;
	
	var = Cvar_FindVar(name);
	if(!var)
		return "";
	else
		return var->getString();
}


const char*	Cvar_CompleteVariable(const std::string &partial)
{
	if(partial.empty())
		return NULL;

	std::map<std::string, cvar_t*>::iterator ir;

	// check for exact match
	ir = cvar_map.find(partial);
	if(ir != cvar_map.end())
		return ir->first.c_str();
		
	// check for partial match
	for(ir = cvar_map.begin(); ir != cvar_map.end(); ir++)
		if(!strncmp(partial.c_str(), ir->first.c_str(), partial.length()))
			return ir->first.c_str();

	return NULL;
}


float	Cvar_ClampVariable(const std::string &name, float min, float max)
{
	cvar_t	*var;
	float	 value;
	
	var = Cvar_FindVar(name);
	if(!var)
		value = 0;
	else
		value = var->getValue();
	
	if(value < min) 
		return min;
		
	if(value > max) 
		return max;
	
	return value;
}

/*
============
Cvar_Get

If the variable already exists, the value will not be set
The flags will be or'ed in if the variable exists.
============
*/
cvar_t*	Cvar_Get(const std::string &name, const std::string &values, uint_t flags)
{
	cvar_t*				var = NULL;
	std::string 			value;
	std::vector<std::string>	vvalues;

//	Com_Printf("Cvar_Get: '%s' '%s' '%i' values\n", name.c_str(), values.c_str(), flags);
	
	if(name.empty())
	{
		Com_Printf("Cvar_Get: empty name\n");
		return NULL;
	}
	
	if(flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if(!Cvar_InfoValidate(name))
		{
			Com_Printf("Cvar_Get: invalid info cvar name\n");
			return NULL;
		}
	}

	// parse values
	boost::spirit::parse_info<> info = boost::spirit::parse(values.c_str(),
	(
		// start grammar
		boost::spirit::list_p((*boost::spirit::anychar_p)[boost::spirit::append(vvalues)], ',')
		// end grammar	
		)
		, boost::spirit::space_p
	);

	if(!info.full)
	{
		Com_Printf("Cvar_Get: parsing of '%s' for '%s' failed", values.c_str(), name.c_str());
		return NULL;
	}

//	Com_Printf("Cvar_Get: '%s' has %i values\n", name.c_str(), vvalues.size());

	if(vvalues.empty())
	{
		Com_Printf("Cvar_Get: '%s' has no values\n", name.c_str());
		return NULL;
	}
	value = vvalues[0];

	var = Cvar_FindVar(name);
	if(var)
	{
		var->_flags |= flags;

		if(!vvalues.empty() && vvalues.size() != var->_values.size())
		{
			// add all new values
			for(std::vector<std::string>::const_iterator i = vvalues.begin(); i != vvalues.end(); ++i)
			{
				std::vector<std::string>::const_iterator j = std::find(var->_values.begin(), var->_values.end(), *i);
			
				if(j == var->_values.end())
					var->_values.push_back(*i);
			}
		}

		return var;
	}

	if(flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if(!Cvar_InfoValidate(value))
		{
			Com_Printf("Cvar_Get: invalid info cvar value\n");
			return NULL;
		}
	}	

	// create new var
	var = new cvar_t(name, value, vvalues, flags);

	// link the variable in
	cvar_map.insert(make_pair(name, var));
	
	return var;
}

cvar_t*	Cvar_Set2(const std::string &name, const std::string &values, uint_t flags, bool force)
{
	cvar_t*				var = NULL;
	std::string 			value;
	std::vector<std::string>	vvalues;

	var = Cvar_FindVar(name);
	if(!var)
	{	
		// create it
		return Cvar_Get(name, values, flags);
	}

	// parse values
	boost::spirit::parse_info<> info = boost::spirit::parse(values.c_str(),
	(
		// start grammar
		boost::spirit::list_p((*boost::spirit::anychar_p)[boost::spirit::append(vvalues)], ',')
		// end grammar	
		)
		, boost::spirit::space_p
	);

	if(!info.full)
	{
		Com_Printf("Cvar_Set2: parsing of '%s' for '%s' failed", values.c_str(), name.c_str());
		return var;
	}

//	Com_Printf("Cvar_Set2: '%s' has %i values\n", name.c_str(), vvalues.size());

	if(vvalues.empty())
	{
		Com_Printf("Cvar_Set2: '%s' has no values\n", name.c_str());
		//return var;
	}
	value = vvalues[0];
	
	if(vvalues.size() == 1)
		vvalues.clear();

	if(var->_flags & (CVAR_USERINFO | CVAR_SERVERINFO))
	{
		if(!Cvar_InfoValidate(value))
		{
			Com_Printf("Cvar_Set2: invalid info cvar value\n");
			return var;
		}
	}

	if(force)
	{
		var->_string_latched = "";
	}
	else
	{
		if(var->_flags & CVAR_INIT)
		{
			Com_Printf("%s is write protected\n", name.c_str());
			return var;
		}
		
		if(var->_flags & CVAR_ROM)
		{
			Com_Printf("%s is read only\n", name.c_str());
			return var;
		}
		
		if(var->_flags & CVAR_CHEAT)
		{
			//TODO
			/*
			if(!com_cheats->value)
			{
				if(value != var->reset_string)
				{
					Com_Printf("%s is cheat protected\n", name.c_str());
					return var;
				}
			}
			*/
		}

		if(var->_flags & CVAR_LATCH)
		{
			if(var->_string_latched.length())
			{
				if(value == var->_string_latched)
					return var;
					
				var->_string_latched = "";
			}
			else
			{
				if(value == var->_string)
					return var;
			}

			if(Com_ServerState())
			{
				Com_Printf("%s will be changed for next game\n", name.c_str());
				var->_string_latched = value;
				var->isModified(true);
			}
			else
			{
				var->_string = value;
				var->isModified(true);
				var->_value = atof(value.c_str());
				var->_integer = atoi(value.c_str());
			}
			return var;
		}
	}

	if(!vvalues.empty() && vvalues.size() != var->_values.size())
	{
		// add all new values
		for(std::vector<std::string>::const_iterator i = vvalues.begin(); i != vvalues.end(); ++i)
		{
			std::vector<std::string>::const_iterator j = std::find(var->_values.begin(), var->_values.end(), *i);
			
			if(j == var->_values.end())
				var->_values.push_back(*i);
		}
	}

	if(value == var->_string)
		return var;		// not changed

	var->isModified(true);

	if(var->_flags & CVAR_USERINFO)
		cvar_userinfo_modified = true;	// transmit at next oportunity
	
	var->_string = value;
	var->_value = atof(value.c_str());
	var->_integer = atoi(value.c_str());

	return var;
}

cvar_t*	Cvar_ForceSet(const std::string &name, const std::string &value)
{
	return Cvar_Set2(name, value, 0, true);
}

cvar_t*	Cvar_Set(const std::string &name, const std::string &value)
{
	return Cvar_Set2(name, value, 0, false);
}


void 	Cvar_SetValue(const std::string &name, float value)
{
	char	val[32];

	if (value == (int)value)
		Com_sprintf(val, sizeof(val), "%i",(int)value);
	else
		Com_sprintf(val, sizeof(val), "%f", value);
		
	Cvar_Set(name, val);
}

void 	Cvar_SetInteger(const std::string &name, int integer)
{
	char	val[32];

	Com_sprintf(val, sizeof(val), "%i", integer);
		
	Cvar_Set(name, val);
}

void 	Cvar_SetModified(const std::string &name)
{
	cvar_t	*var;
	
	var = Cvar_FindVar(name);
	if(!var)
		return;
		
	var->isModified(true);
}

/*
============
Cvar_SetLatchedVars

Any variables with latched values will now be updated
============
*/
void 	Cvar_SetLatchedVars()
{
	for(std::map<std::string, cvar_t*>::const_iterator ir = cvar_map.begin(); ir != cvar_map.end(); ir++)
	{
		cvar_t *var = ir->second;
		
		if(var->_string_latched.empty())
			continue;
			
		var->_string = var->_string_latched;
		var->_value = atof(var->_string_latched.c_str());
		var->_integer = atoi(var->_string_latched.c_str());
		
		var->_string_latched = "";
	}
}

/*
============
Cvar_Command

Handles variable inspection and changing from the console
============
*/
bool 	Cvar_Command()
{
	cvar_t			*var;

	// check variables
	var = Cvar_FindVar(Cmd_Argv(0));
	if(!var)
		return false;
		
	// perform a variable print or set
	if(Cmd_Argc() == 1)
	{
		Com_Printf("\"%s\" is: \"%s" S_COLOR_WHITE "\" default: \"%s" S_COLOR_WHITE "\"", var->_name.c_str(), var->getString(), var->getResetString());
		
		if((var->_flags & CVAR_LATCH) && var->_string_latched.length())
		{
			Com_Printf(" latched: \"%s" S_COLOR_WHITE "\"", var->_string_latched.c_str());
		}

		if(var->_values.size() > 1)
		{
			Com_Printf(" values: ");
			for(uint_t i=0; i<var->_values.size(); i++)
			{
				if(i != var->_values.size()-1)
					Com_Printf("\"%s\", ", var->_values[i].c_str()); 
				else
					Com_Printf("\"%s\"", var->_values[i].c_str()); 
			}
		}

		Com_Printf("\n");
		return true;
	}

	Cvar_Set(var->_name, Cmd_Argv(1));
	
	return true;
}


static void 	Cvar_Set_f()
{
	if(Cmd_Argc() < 3)
	{
		Com_Printf("usage: set <variable> <value>\n");
		return;
	}
	
	Cvar_Set(Cmd_Argv(1), Cmd_Argv(2));
}

static void 	Cvar_Setu_f()
{
	if(Cmd_Argc() < 3)
	{
		Com_Printf("usage: setu <variable> <value>\n");
		return;
	}
	
	Cvar_Set2(Cmd_Argv(1), Cmd_Argv(2), CVAR_USERINFO, false);
}

static void 	Cvar_Sets_f()
{
	if(Cmd_Argc() < 3)
	{
		Com_Printf("usage: sets <variable> <value>\n");
		return;
	}
	
	Cvar_Set2(Cmd_Argv(1), Cmd_Argv(2), CVAR_SERVERINFO, false);
}


static void 	Cvar_Seta_f()
{
	if(Cmd_Argc() < 3)
	{
		Com_Printf("usage: seta <variable> <value>\n");
		return;
	}
	
	Cvar_Set2(Cmd_Argv(1), Cmd_Argv(2), CVAR_ARCHIVE, false);
}


static void 	Cvar_Reset_f()
{
	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage: reset <variable>\n");
		return;
	}
	
	cvar_t *var = Cvar_FindVar(Cmd_Argv(1));
	if(!var)
	{
		Com_Printf("\"%s\" is not a variable\n", Cmd_Argv(1));
		return;
	}
	
	Cvar_Set(var->getName(), var->getResetString());
}

void 	Cvar_Toggle_f()
{
	if(Cmd_Argc() != 2)
	{
		Com_Printf("usage: toggle <variable>\n");
		return;
	}

	cvar_t *var = Cvar_FindVar(Cmd_Argv(1));
	if(!var)
	{
		Com_Printf("\"%s\" is not a variable\n", Cmd_Argv(1));
		return;
	}

//	if(var->hasValues())
		Cvar_Set(var->getName(), var->getNextString());
//	else
//		Cvar_SetInteger(var->getName(), !var->getInteger());
}

void 	Cvar_Add_f()
{
	if(Cmd_Argc() != 3)
	{
		Com_Printf ("usage: add <variable> <value>\n");
		return;
	}

	cvar_t *var = Cvar_FindVar(Cmd_Argv(1));
	if(!var)
	{
		Com_Printf("\"%s\" is not a variable\n", Cmd_Argv(1));
		return;
	}

	Cvar_SetValue(var->getName(), var->getValue() + atof(Cmd_Argv(2)));
}


void 	Cvar_Sub_f()
{
	if(Cmd_Argc() != 3)
	{
		Com_Printf ("usage: sub <variable> <value>\n");
		return;
	}

	cvar_t *var = Cvar_FindVar(Cmd_Argv(1));
	if(!var)
	{
		Com_Printf("\"%s\" is not a variable\n", Cmd_Argv(1));
		return;
	}

	Cvar_SetValue(var->getName(), var->getValue() - atof(Cmd_Argv(2)));
}

void 	Cvar_Mul_f()
{
	if(Cmd_Argc() != 3)
	{
		Com_Printf ("usage: mul <variable> <value>\n");
		return;
	}

	cvar_t *var = Cvar_FindVar(Cmd_Argv(1));
	if(!var)
	{
		Com_Printf("\"%s\" is not a variable\n", Cmd_Argv(1));
		return;
	}

	Cvar_SetValue(var->getName(), var->getValue() * atof(Cmd_Argv(2)));
}

void 	Cvar_Div_f()
{
	if(Cmd_Argc() != 3)
	{
		Com_Printf ("usage: div <variable> <value>\n");
		return;
	}

	cvar_t *var = Cvar_FindVar(Cmd_Argv(1));
	if(!var)
	{
		Com_Printf("\"%s\" is not a variable\n", Cmd_Argv(1));
		return;
	}

	Cvar_SetValue(var->getName(), var->getValue() / atof(Cmd_Argv(2)));
}

void 	Cvar_Inc_f()
{
	if(Cmd_Argc() != 3)
	{
		Com_Printf ("usage: inc <variable> <value>\n");
		return;
	}

	cvar_t *var = Cvar_FindVar(Cmd_Argv(1));
	if(!var)
	{
		Com_Printf("\"%s\" is not a variable\n", Cmd_Argv(1));
		return;
	}

	Cvar_SetInteger(var->getName(), var->getInteger() + 1);
}

void 	Cvar_Dec_f()
{
	if(Cmd_Argc() != 3)
	{
		Com_Printf ("usage: dec <variable> <value>\n");
		return;
	}

	cvar_t *var = Cvar_FindVar(Cmd_Argv(1));
	if(!var)
	{
		Com_Printf("\"%s\" is not a variable\n", Cmd_Argv(1));
		return;
	}

	Cvar_SetInteger(var->getName(), var->getInteger() - 1);
}



/*
============
Cvar_WriteVariables

Appends lines containing "set variable value" for all variables
with the archive flag set to true.
============
*/
void 	Cvar_WriteVars(VFILE *stream)
{
	for(std::map<std::string, cvar_t*>::const_iterator ir = cvar_map.begin(); ir != cvar_map.end(); ++ir)
	{
		cvar_t *var = ir->second;
		
		if(!var)
			continue;
			
		if(var->hasFlags(CVAR_ARCHIVE))
		{
			//if(var->hasValues())
			//	VFS_FPrintf(stream, "set %s \"%s:%s\"\n", var->getName(), var->getString(), var->getValues());
			//else
				VFS_FPrintf(stream, "set %s \"%s\"\n", var->getName(), var->getString());
		}
	}
}

/*
void 	Cvar_WriteLatchedVars(VFILE *stream)
{
	char	name[MAX_OSPATH], string[128];
		
	for(std::map<std::string, cvar_t*>::const_iterator ir = cvar_map.begin(); ir != cvar_map.end(); ir++)
	{
		cvar_t *var = ir->second;
		
		if(!var)
			continue;
			
		if(!var->hasFlags(CVAR_LATCH))
			continue;
			
		if(strlen(var->getName()) >= sizeof(name)-1 || strlen(var->getString()) >= sizeof(string)-1)
		{
			Com_Printf("Cvar too long: '%s' = '%s'\n", var->getName(), var->getString());
			continue;
		}
		
		memset(name, 0, sizeof(name));
		memset(string, 0, sizeof(string));
		
		strcpy(name, var->getName());
		strcpy(string, var->getString());
		
		VFS_FWrite(name, sizeof(name), stream);
		VFS_FWrite(string, sizeof(string), stream);
	}
}
*/

void 	Cvar_List_f()
{
	for(std::map<std::string, cvar_t*>::const_iterator ir = cvar_map.begin(); ir != cvar_map.end(); ++ir)
	{
		cvar_t *var = ir->second;
		
		if(var->hasFlags(CVAR_ARCHIVE))
			Com_Printf("A");
		else
			Com_Printf(" ");
			
		if(var->hasFlags(CVAR_USERINFO))
			Com_Printf("U");
		else
			Com_Printf(" ");
			
		if(var->hasFlags(CVAR_SERVERINFO))
			Com_Printf("S");
		else
			Com_Printf(" ");
			
		if(var->hasFlags(CVAR_INIT))
			Com_Printf("I");
		else
			Com_Printf(" ");
			
		if(var->hasFlags(CVAR_LATCH))
			Com_Printf("L");
		else
			Com_Printf(" ");
			
		if(var->hasFlags(CVAR_ROM))
			Com_Printf("R");
		else
			Com_Printf(" ");
			
		if(var->hasFlags(CVAR_CHEAT))
			Com_Printf("C");
		else
			Com_Printf(" ");
			
		
		Com_Printf(" %s \"%s\"\n", var->getName(), var->getString());
	}
	
	Com_Printf("%i cvars\n", cvar_map.size());
}


static const char*	Cvar_InfoString(uint_t bits)
{
	info_c	info;
	
	for(std::map<std::string, cvar_t*>::const_iterator ir = cvar_map.begin(); ir != cvar_map.end(); ++ir)
	{
		cvar_t *var = ir->second;
			
		if(var->hasFlags(bits))
			info.setValueForKey(var->getName(), var->getString());
	}
	
	return info.toString();
}

// returns an info string containing all the CVAR_USERINFO cvars
const char*	Cvar_Userinfo()
{
	return Cvar_InfoString(CVAR_USERINFO);
}

// returns an info string containing all the CVAR_SERVERINFO cvars
const char*	Cvar_Serverinfo()
{
	return Cvar_InfoString(CVAR_SERVERINFO);
}

void 	Cvar_Init ()
{
	Com_Printf("------- Cvar_Init -------\n");
	
	Cmd_AddCommand("set",		Cvar_Set_f);
	Cmd_AddCommand("setu",		Cvar_Setu_f);
	Cmd_AddCommand("sets",		Cvar_Sets_f);
	Cmd_AddCommand("seta",		Cvar_Seta_f);
	Cmd_AddCommand("reset",		Cvar_Reset_f);
	Cmd_AddCommand("toggle",	Cvar_Toggle_f);
	Cmd_AddCommand("add",		Cvar_Add_f);
	Cmd_AddCommand("sub",		Cvar_Sub_f);
	Cmd_AddCommand("mul",		Cvar_Div_f);
	Cmd_AddCommand("inc",		Cvar_Inc_f);
	Cmd_AddCommand("dec",		Cvar_Dec_f);
	Cmd_AddCommand("cvarlist",	Cvar_List_f);
}
