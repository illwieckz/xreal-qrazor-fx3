/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
#include <algorithm>

// xreal --------------------------------------------------------------------
#include "cmd.h"
#include "common.h"
#include "cvar.h"
#include "vfs.h"



#define	ALIAS_LOOP_COUNT	16




static int					cmd_alias_count;		// for detecting runaway loops
static std::map<std::string, std::string>	cmd_alias_map;

static bool		cmd_wait;

static std::string	cmd_text_buf;
static std::string	cmd_defer_text_buf;



/*
============
Cbuf_AddText

Adds command text at the end of the buffer
============
*/
void 	Cbuf_AddText(const std::string &text)
{
	cmd_text_buf = cmd_text_buf + text;
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
============
*/
void 	Cbuf_InsertText(const std::string &text)
{
	cmd_text_buf = text + cmd_text_buf;
}


void 	Cbuf_CopyToDefer()
{
	cmd_defer_text_buf = cmd_text_buf;
	cmd_text_buf = "";
}

void 	Cbuf_InsertFromDefer()
{
	Cbuf_InsertText(cmd_defer_text_buf);
	cmd_defer_text_buf = "";
}


void 	Cbuf_ExecuteText(exec_type_e type, const std::string &text)
{
	switch(type)
	{
		case EXEC_NOW:
			Cmd_ExecuteString(text);
			break;
			
		case EXEC_INSERT:
			Cbuf_InsertText(text);
			break;
			
		case EXEC_APPEND:
			Cbuf_AddText(text);
			break;
			
		default:
			Com_Error(ERR_FATAL, "Cbuf_ExecuteText: bad type");
	}
}

void 	Cbuf_Execute()
{
	unsigned int	i;
	std::string	text;
	std::string	line;
	int		quotes;

	cmd_alias_count = 0;			// don't allow infinite alias loops

	while(cmd_text_buf.length())
	{
		// find a \n or ; line break
		text = cmd_text_buf;

		quotes = 0;
		
		for(i=0; i<text.length(); i++)
		{
			if(text[i] == '"')
				quotes++;
				
			if(!(quotes & 1) && text[i] == ';')
				break;		// don't break if inside a quoted string
				
			if(text[i] == '\n')
				break;
		}
			
				
		line = text.substr(0, i);
		
		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec, alias) can insert data at the
		// beginning of the text buffer
		if(i == cmd_text_buf.length())
			cmd_text_buf = "";
		else
		{
			i++;
			cmd_text_buf = cmd_text_buf.substr(i, cmd_text_buf.length());
		}

		// execute the command line
		Cmd_ExecuteString(line);
		
		if(cmd_wait)
		{
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait = false;
			break;
		}
	}
}


/*
===============
Cbuf_AddEarlyCommands

Adds command line parameters as script statements
Commands lead with a +, and continue until another +

Set commands are added early, so they are guaranteed to be set before
the client and server initialize for the first time.

Other commands are added late, after all initialization is complete.
===============
*/
void 	Cbuf_AddEarlyCommands(bool clear)
{
	int		i;
	const char	*s;

	for(i=0; i<Com_Argc(); i++)
	{
		s = Com_Argv(i);
		
		if(strcmp(s, "+set"))
			continue;
			
		Cbuf_AddText(va("set %s %s\n", Com_Argv(i+1), Com_Argv(i+2)));
		
		if(clear)
		{
			Com_ClearArgv(i);
			Com_ClearArgv(i+1);
			Com_ClearArgv(i+2);
		}
		i+=2;
	}
}

/*
=================
Cbuf_AddLateCommands

Adds command line parameters as script statements
Commands lead with a + and continue until another + or -
quake +vid_ref gl +map amlev1

Returns true if any late commands were added, which
will keep the demoloop from immediately starting
=================
*/
bool 	Cbuf_AddLateCommands()
{
	int		i, j;
	int		s;
	char	*text, *build, c;
	int		argc;
	bool	ret;

	// build the combined string to parse from
	s = 0;
	argc = Com_Argc();
	for (i=1 ; i<argc ; i++)
	{
		s += strlen (Com_Argv(i)) + 1;
	}
	if (!s)
		return false;
		
	text = (char*)Com_Alloc(s+1);
	text[0] = 0;
	for (i=1 ; i<argc ; i++)
	{
		strcat (text,Com_Argv(i));
		if (i != argc-1)
			strcat (text, " ");
	}
	
	// pull out the commands
	build = (char*)Com_Alloc(s+1);
	build[0] = 0;
	
	for (i=0 ; i<s-1 ; i++)
	{
		if (text[i] == '+')
		{
			i++;

			for (j=i ; (text[j] != '+') && (text[j] != '-') && (text[j] != 0) ; j++)
				;

			c = text[j];
			text[j] = 0;
			
			strcat (build, text+i);
			strcat (build, "\n");
			text[j] = c;
			i = j-1;
		}
	}

	ret = (build[0] != 0);
	if (ret)
		Cbuf_AddText (build);
	
	Com_Free (text);
	Com_Free (build);

	return ret;
}


/*
==============================================================================
				SCRIPT COMMANDS
==============================================================================
*/


static void 	Cmd_Exec_f()
{
	char*		buf = NULL;
	std::string	filename;
	int		len;

	if(Cmd_Argc () != 2)
	{
		Com_Printf("exec <filename> : execute a script file\n");
		return;
	}
	
	filename = Cmd_Argv(1);
	
	len = VFS_FLoad(filename, (void **)&buf);
	if (!buf)
	{
		Com_Printf("couldn't exec %s\n", filename.c_str());
		return;
	}
	Com_Printf("executing %s ...\n", filename.c_str());
		
	Cbuf_InsertText(buf);

	VFS_FFree(buf);
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
static void 	Cmd_Echo_f()
{
	for(int i=1 ; i<Cmd_Argc() ; i++)
		Com_Printf("%s ",Cmd_Argv(i));
		
	Com_Printf("\n");
}

/*
===============
Cmd_Alias_f

Creates a new command that executes a command string (possibly ; seperated)
===============
*/
static void 	Cmd_Alias_f()
{
	std::string	cmd;
	std::string	name;

	std::map<std::string, std::string>::iterator ir;

	if(Cmd_Argc() == 1)
	{
		Com_Printf ("Current alias commands:\n");
		
		for(ir=cmd_alias_map.begin(); ir != cmd_alias_map.end(); ir++)
			Com_Printf("%s : %s\n", ir->first.c_str(), ir->second.c_str());
		return;
	}
	
	name = Cmd_Argv(1);
	/*
	s = Cmd_Argv(1);
	if (strlen(s) >= MAX_ALIAS_NAME)
	{
		Com_Printf ("Alias name is too long\n");
		return;
	}
	*/

	
	// if the alias already exists, reuse it
	ir = cmd_alias_map.find(name);
	if(ir != cmd_alias_map.end())
	{	
		cmd_alias_map.erase(ir);
	}
	


	// copy the rest of the command line
	cmd = "";		// start out with a null string
	for(int i=2 ; i<Cmd_Argc(); i++)
	{
		cmd += Cmd_Argv(i);
		
		if(i != (Cmd_Argc() - 1))
			cmd += " ";
	}
	cmd += "\n";
	
	cmd_alias_map.insert(std::make_pair(name, cmd));
}





/*
=============================================================================
			COMMAND EXECUTION
=============================================================================
*/


static std::vector<std::string>	cmd_argv;
static std::string		cmd_args;

static std::map<std::string, void (*)()>	cmd_functions_map;		// possible commands to execute

int	Cmd_Argc()
{
	return cmd_argv.size();
}

const char*	Cmd_Argv(int arg)
{
	if(arg < 0 || arg >= (int)cmd_argv.size())
	{
		//Com_Error(ERR_WARNING, "Cmd_Argv: bad argument num %i during %i tokens\n", arg, cmd_argv.size())
		return "";
		
	}

	return cmd_argv[arg].c_str();
}

/*
============
Cmd_Args

Returns a single string containing argv(1) to argv(argc()-1)
============
*/
const char*	Cmd_Args()
{
	return cmd_args.c_str();
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
$Cvars will be expanded unless they are in a quoted token
============
*/
void 	Cmd_TokenizeString(const std::string &string)
{
	char*	token;
	char*	text;

	cmd_argv.clear();
	cmd_args = "";
	
	if(string.empty())
		return;
	
	text = (char*)string.c_str();
	
	//token = cmd_tokens;
	
	
	while(cmd_argv.size() != MAX_STRING_TOKENS)
	{
#if 0
		// skip whitespace up to a /n
		while(*text && *text <= ' ')
		{
			text++;
		}
		
		if(!*text)
			break;
			
		// skip double slash comments
		if(text[0] == '/' && text[1] == '/')
			break;
			
		// skip /* */ comments
		if(text[0] =='/' && text[1] == '*')
		{
			text += 2;
			
			while(*text && (*text != '*' || text[1] != '/'))
				text++;
			
			if(!*text)
				break;
			
			text += 2;
			continue;
		}
		
		cmd_argv.push_back(token);
		
		// handle quoted strings specially
		if(*text == '\"')
		{
			text++;
			
			while(*text && *text !='\"')
				*token++ = *text++;
				
			*token++ = 0;
			
			if(!*text)
				break;
			
			text++;
			continue;
		}

		// parse a regular word
		do
		{
			if( *text == '\"' ) 
			{
				 break;
			}
			
			if( text[0] == '/' && text[1] == '/' )
			{
				 break;
			}
			
			if( text[0] == '/' && text[1] == '*' ) 
			{
				 break;
			}
			
			*token++ = *text++;
		} while( *text > ' ' );	
		
		*token++ = 0;
#else
		// skip whitespace up to a /n
		while (*text && *text <= ' ' && *text != '\n')
		{
			text++;
		}
		
		if (*text == '\n')
		{	// a newline seperates commands in the buffer
			text++;
			break;
		}

		if (!*text)
			return;

		// set cmd_args to everything after the first arg
		if (cmd_argv.size() == 1)
		{
			//int		l;

			cmd_args = text;

			/*
			// strip off any trailing whitespace
			l = strlen(cmd_args) - 1;
			for ( ; l >= 0 ; l--)
				if (cmd_args[l] <= ' ')
					cmd_args[l] = 0;
				else
					break;
			*/
		}


		token = Com_Parse(&text);
		
		if(!token[0])
			break;
			
		cmd_argv.push_back(token);
#endif
	}
	
}


void	Cmd_AddCommand(const std::string &name, void (*cmd)())
{	
	// fail if the command is a variable name
	if(Cvar_VariableString(name.c_str())[0])
	{
		Com_Printf("Cmd_AddCommand: %s already defined as a var\n", name.c_str());
		return;
	}
	
	
	// fail if the command already exists
	std::map<std::string, void (*)()>::iterator ir = cmd_functions_map.find(name);
	if(ir != cmd_functions_map.end())
	{	
		Com_Printf("Cmd_AddCommand: %s already defined\n", name.c_str());
		return;
	}

	cmd_functions_map.insert(std::make_pair(name, cmd));
}

void	Cmd_RemoveCommand(const std::string &name)
{
	std::map<std::string, void (*)()>::iterator ir = cmd_functions_map.find(name);
	if(ir != cmd_functions_map.end())
		cmd_functions_map.erase(ir);
	else
		Com_Printf("Cmd_AddCommand: %s already defined\n", name.c_str());
}

bool	Cmd_Exists(const std::string &name)
{
	std::map<std::string, void (*)()>::iterator ir = cmd_functions_map.find(name);
	if(ir != cmd_functions_map.end())
		return true;
	else
		return false;
}



const char*	Cmd_CompleteCommand(const std::string &partial)
{	
	if(partial.empty())
		return NULL;
		
	// check for exact match		
	std::map<std::string, void (*)()>::iterator fir = cmd_functions_map.find(partial);
	if(fir != cmd_functions_map.end())
		return fir->first.c_str();
		
	std::map<std::string, std::string>::iterator air = cmd_alias_map.find(partial);
	if(air != cmd_alias_map.end())
		return air->first.c_str();

	// check for partial match
	for(fir=cmd_functions_map.begin(); fir != cmd_functions_map.end(); fir++)
		if(!strncmp(partial.c_str(), fir->first.c_str(), partial.length()))
			return fir->first.c_str();
			
	for(air=cmd_alias_map.begin(); air != cmd_alias_map.end(); air++)
		if(!strncmp(partial.c_str(), air->first.c_str(), partial.length()))
			return air->first.c_str();
	

	return NULL;
}


void	Cmd_ExecuteString(const std::string &text)
{	
	Cmd_TokenizeString(text);
	
	//Com_Printf("Cmd_ExecuteString: %s\n", text.c_str()); 
			
	// execute the command line
	if(!Cmd_Argc())
		return;		// no tokens

	// check functions
	std::map<std::string, void (*)()>::iterator fir = cmd_functions_map.find(Cmd_Argv(0));
	if(fir != cmd_functions_map.end())
	{
		if(!fir->second)
			Cmd_ExecuteString("cmd " + text);
		else
			fir->second();
		return;
		
	}
	
	// check alias
	std::map<std::string, std::string>::iterator air = cmd_alias_map.find(Cmd_Argv(0));
	if(air != cmd_alias_map.end())
	{
		if(++cmd_alias_count == ALIAS_LOOP_COUNT)
		{
			Com_Printf("Cmd_ExecuteString: ALIAS_LOOP_COUNT\n");
			return;
		}
		Cbuf_InsertText(air->second);
		return;
	}
	
	
	
	// check cvars
	if(Cvar_Command())
		return;

	// send it as a server command if we are connected
	Cmd_ForwardToServer();
}

static void	Cmd_List_f()
{
	int		i;
	std::map<std::string, void (*)()>::iterator fir;
	
	for(i=0, fir=cmd_functions_map.begin(); fir != cmd_functions_map.end(); i++, fir++)
		Com_Printf("%s\n", fir->first.c_str());

	Com_Printf("%i commands\n", i);
}

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "impulse 5 ; +attack ; wait ; -attack ; impulse 2"
============
*/
static void 	Cmd_Wait_f()
{
	cmd_wait = true;
}


/*
============
Cmd_Init
============
*/
void 	Cmd_Init()
{
	
	Com_Printf("------- Cmd_Init -------\n");
	
	//
	// register our commands
	//
	Cmd_AddCommand("cmdlist",	Cmd_List_f);
	Cmd_AddCommand("exec",		Cmd_Exec_f);
	Cmd_AddCommand("echo",		Cmd_Echo_f);
	Cmd_AddCommand("alias",		Cmd_Alias_f);
	Cmd_AddCommand("wait",		Cmd_Wait_f);
	

}

