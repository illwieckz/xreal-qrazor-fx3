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

// qrazor-fx ----------------------------------------------------------------
#include "vfs.h"
#include "unzip.h"
#include "cmd.h"
#include "cvar.h"
#include "sys.h"



class zipentry_c
{
public:
	zipentry_c(const std::string &name, int filepos, int filelen, int crc)
	{
		_name		= name;
		_filepos	= filepos;
		_filelen	= filelen;
		_crc		= crc;
	}
	
	const char*	getName()	{return _name.c_str();}
	int		getPos()	{return _filepos;}
	int		getLength()	{return _filelen;}
	int		getCRC()	{return _crc;}
	
private:
	std::string	_name;
	
	int		_filepos;
	int 		_filelen;
	
	int		_crc;
};

class zip_c
{
public:
	zip_c(const std::string &name, unzFile handle, std::map<std::string, zipentry_c*> &entries)
	{
		_name		= name;
		_handle		= handle;
		_entries	= entries;
	}
	
	~zip_c()
	{
		for(std::map<std::string, zipentry_c*>::iterator ir = _entries.begin(); ir != _entries.end(); ir++)
		{
			if(ir->second)
				delete ir->second;
			ir->second = NULL;
		}
		
		_entries.clear();
	}
	
	
	void	addEntry(const std::string &name, zipentry_c *entry)
	{
		_entries.insert(std::make_pair(name, entry));
	}
	
	
	const char*	getName()				{return _name.c_str();}
	unzFile		getHandle()				{return _handle;}
	unsigned int	getEntriesNum()				{return _entries.size();}
	zipentry_c*	getEntry(const std::string &name)
	{
		std::map<std::string, zipentry_c*>::const_iterator ir = _entries.find(name);
		if(ir != _entries.end())
			return ir->second;
		else
			return NULL;
	}
	
	
//private:
	std::string	_name;
	unzFile		_handle;
	
	std::map<std::string, zipentry_c*>	_entries;
	
	int		_checksum;
};

class searchpath_c
{
public:
	searchpath_c(const std::string &name, zip_c *pack)
	{
		_name		= name;
		_pack		= pack;
	}
	
	const char*	getName()	{return _name.c_str();}
	zip_c*		getPack()	{return _pack;}

private:
	std::string	_name;
	zip_c*		_pack;			// only one of filename / pack will be used
};



static cvar_t*	vfs_pkgdatadir;
static cvar_t*	vfs_basedir;
static cvar_t*	vfs_userdatadir;
static cvar_t*	vfs_pkglibdir;
static cvar_t*	vfs_game;


static std::vector<searchpath_c*>	vfs_searchpaths;



static void 	VFS_Dir_f()
{
	std::vector<std::string>	dirnames;
	int	ndirs = 0;
	const char	*dir;
	const char	*ext;
	int	argc;
	
	argc = Cmd_Argc();

	if(argc < 2 || argc > 3)
	{
		Com_Printf("usage: dir <directory> <extension>\n");
		return;
	}
	
	dir = Cmd_Argv(1);
	
	if(argc == 3)
		ext = Cmd_Argv(2);
	else
		ext = "";
		

	if((dirnames = VFS_ListFiles(dir, ext)).size() != 0 )
	{
		for(uint_t i=0; i < dirnames.size(); i++)
		{
			Com_Printf( "%s\n", dirnames[i].c_str());
			ndirs++;
		}
	}
	
	Com_Printf("%i files listed\n", ndirs);
}

static void 	VFS_Path_f()
{
	Com_Printf("Current search path:\n");
	
	for(std::vector<searchpath_c*>::const_iterator ir = vfs_searchpaths.begin(); ir != vfs_searchpaths.end(); ++ir)
	{
		searchpath_c *s = *ir;
	
		if(!s)
			continue;
					
		if(s->getPack())
			Com_Printf("'%s' (%i files)\n", s->getPack()->getName(), s->getPack()->getEntriesNum());
		else
			Com_Printf("'%s'\n", s->getName());
	}
}

static void 	VFS_Restart_f()
{
	VFS_Restart();
}

std::vector<std::string>	VFS_ListFiles(const std::string &dir, const std::string &extension)
{
	unsigned int j;
		
	std::string			s;
	std::vector<std::string>	out;
			
	searchpath_c*	search = NULL;
	
	zip_c	*pak = NULL;
	
	
	// get file names
	for(std::vector<searchpath_c*>::const_iterator ir = vfs_searchpaths.begin(); ir != vfs_searchpaths.end(); ++ir)
	{	
		search = *ir;
		
		if(!search)
			continue;
		
		if(!search->getPack())
		{
			// check a file in the directory tree
			std::string	findname;
			
			if(dir.length())
				findname = std::string(search->getName()) + "/" + dir + "/*" + extension;
			else
				findname = std::string(search->getName()) + "/*" + extension;
						
			s = Sys_FindFirst(findname, SFF_SUBDIR, 0);
			while(s.length())
			{
				if(dir.length() && s.length() > 1 && s[0] == '/')
					s = s.substr(1, s.length());
					
				//if(s.length() > 1 && s[s.length()-1] == '/')
				//	continue;
					
				if(s.find('.') == s.npos)
					s += '/';
			
				// check if already in list
				for(j=0; j<out.size(); j++)
				{
					if(s == out[j])
						break;
				}
				
				// add to list
				if(j == out.size())
				{
					//s = X_strlwr(s);
					
					out.push_back(strstr(s.c_str(), dir.c_str()));
				}
				
				s = Sys_FindNext(SFF_SUBDIR, 0);
			}
			Sys_FindClose();
		}
		else
		{
			pak = search->getPack();
		
			// look through all the pak file elements
			for(std::map<std::string, zipentry_c*>::const_iterator ir = pak->_entries.begin(); ir != pak->_entries.end(); ir++)
			{
				s = ir->first;
								
				if(X_strncasecmp(s.c_str(), dir.c_str(), dir.length()))
					continue;
					
				if(s.length() > 1 && s[s.length()-1] == '/')
					continue;
				
				if(extension.length())
				{
					if(X_strcasecmp((s.substr(s.length() - extension.length(), s.length()).c_str()), extension.c_str()))
						continue;
				}				
				
				// check if already in list
				for(j=0; j<out.size(); j++)
				{
					if(!X_strcasecmp(s.c_str(), out[j].c_str()))
						break;
				}
				
				// add to list
				if(j == out.size())
				{
					s = X_strlwr(s);
					
					out.push_back(s);
				}
			}
		}
	}
	
	std::sort(out.begin(), out.end());
	
	return out;
}



/*
=================
VFS_LoadPackFile

Takes an explicit (not game tree related) path to a .zip file.

Loads the header and directory, adding the files at the beginning
of the list so they override previous pack files.
=================
*/
zip_c*	VFS_LoadZipFile(const std::string &name)
{
	char			entry_name[MAX_OSPATH];
	zipentry_c*		entry;
	zip_c*			pack;
	std::map<std::string, zipentry_c*>	entries;
	
	unzFile			packhandle;
	unz_global_info		g_info;
	unz_file_info		f_info;

	if(name.empty())
	{
		Com_Printf("VFS_LoadZipFile: empty name\n");
		return NULL;
	}

	// open the zip file	
	packhandle = unzOpen(name.c_str());
	if(unzGetGlobalInfo(packhandle, &g_info)!= UNZ_OK)
	{
		//Com_Printf("VFS_LoadZipFile: bad global info\n");
		return NULL;
	}

	unzGoToFirstFile(packhandle);

	// get the entries
	for(unsigned i=0; i<g_info.number_entry; i++)
	{
		if(unzGetCurrentFileInfo(packhandle, &f_info, entry_name, sizeof(entry_name), NULL, 0, NULL, 0) != UNZ_OK)
			break;
				
		// create new entry
		entry = new zipentry_c(X_strlwr(entry_name), ((unz_s*)packhandle)->pos_in_central_dir, f_info.uncompressed_size, f_info.crc);
		
		// add single entry
		entries.insert(std::make_pair(std::string(entry_name), entry));
				
		unzGoToNextFile(packhandle);
	}
	
	// create new package
	pack = new zip_c(name, packhandle, entries);
	
	
	Com_Printf("added packfile %s (%i files)\n", name.c_str(), entries.size());
	return pack;
}

/*
================
VFS_AddGameDirectory

Sets vfs_game, adds the directory to the head of the path,
then loads and adds the .pk4 files
================
*/
/*
class paksort_c : public std::binary_function<std::string, std::string, bool>
{
public:
	bool operator()(const std::string &x, const std::string &y)
	{
		return strcmp(x.c_str(), y.c_str());
	}
};
*/

static void 	VFS_AddGameDirectory(const std::string &dir)
{
	Com_Printf("adding game directory '%s' ...\n", dir.c_str());

	vfs_searchpaths.push_back(new searchpath_c(dir, NULL));

	std::vector<std::string>	filenames;
	
	std::string s = Sys_FindFirst(dir + "/*.pk4", 0, 0);
	while(s.length())
	{
		filenames.push_back(s);
		
		s = Sys_FindNext(0, 0);
	}
	Sys_FindClose();
	
	std::sort(filenames.begin(), filenames.end(), std::greater<std::string>());
//	std::sort(filenames.begin(), filenames.end(), paksort_c());
	
	for(uint_t i=0; i<filenames.size(); i++)
	{
		const std::string& pakfile = filenames[i];
		
		//Com_Printf("adding pak file '%s' ...\n", pakfile.c_str());
		
		zip_c *pak = VFS_LoadZipFile(pakfile);
		if(!pak)
			continue;
		
		vfs_searchpaths.push_back(new searchpath_c(dir, pak));
	}
}


void 	VFS_Init()
{	
	Com_Printf("------- VFS_Init -------\n");
	
	Cmd_AddCommand("path", VFS_Path_f);
	Cmd_AddCommand("dir", VFS_Dir_f);
	Cmd_AddCommand("vfs_restart", VFS_Restart_f);


	//
	// gamedir
	//
	vfs_game = Cvar_Get("vfs_game", "", CVAR_LATCH | CVAR_SERVERINFO);
	vfs_game->isModified(false);
	//VFS_SetGamedir(vfs_game->getString());


	//
	// userdir
	//
	vfs_userdatadir = Cvar_Get("vfs_userdatadir", Sys_ExpandSquiggle(va("%s", VFS_USERDATADIR)), CVAR_ARCHIVE);
	
	//
	// sharedir <path>
	//	
	vfs_pkgdatadir = Cvar_Get("vfs_pkgdatadir", VFS_PKGDATADIR, CVAR_ARCHIVE);

	//
	// basedir <path>
	// allows the game to run from outside the data tree
	//
	vfs_basedir = Cvar_Get("vfs_basedir", VFS_BASEDIRNAME, CVAR_ARCHIVE); 
	
	//
	// libdir
	//
	vfs_pkglibdir = Cvar_Get("vfs_pkglibdir", VFS_PKGLIBDIR, CVAR_ARCHIVE);
	
	
	//
	// start up with xreal by default
	//
	if(X_strequal(vfs_game->getString(), ""))
	{
		VFS_AddGameDirectory(std::string(vfs_userdatadir->getString()) + "/" + std::string(vfs_basedir->getString()));
		VFS_AddGameDirectory(std::string(vfs_pkgdatadir->getString()) + "/" + std::string(vfs_basedir->getString()));
		//VFS_AddGameDirectory(std::string(vfs_pkglibdir->getString()) + "/" + std::string(vfs_basedir->getString()));
	}
	else
	{
		VFS_AddGameDirectory(std::string(vfs_userdatadir->getString()) + "/" + std::string(vfs_game->getString()));
		VFS_AddGameDirectory(std::string(vfs_userdatadir->getString()) + "/" + std::string(vfs_basedir->getString()));
		
		VFS_AddGameDirectory(std::string(vfs_pkgdatadir->getString()) + "/" + std::string(vfs_game->getString()));
		VFS_AddGameDirectory(std::string(vfs_pkgdatadir->getString()) + "/" + std::string(vfs_basedir->getString()));
		
		//VFS_AddGameDirectory(std::string(vfs_pkglibdir->getString()) + "/" + std::string(vfs_game->getString()));
		//VFS_AddGameDirectory(std::string(vfs_pkglibdir->getString()) + "/" + std::string(vfs_basedir->getString()));
	}
	
	//VFS_AddGameDirectory(vfs_pkglibdir->getString());

	
	//
	// show search paths
	//
	VFS_Path_f();
}

void 	VFS_Shutdown()
{	
	Com_Printf("------- VFS_Shutdown -------\n");

	//
	// clear searchpaths
	//
	for(std::vector<searchpath_c*>::iterator ir = vfs_searchpaths.begin(); ir != vfs_searchpaths.end(); ir++)
	{
		if(*ir)
			delete *ir;
		*ir = NULL;
	}
	vfs_searchpaths.clear();
		
	Cmd_RemoveCommand("path");
	Cmd_RemoveCommand("dir");
	Cmd_RemoveCommand("vfs_restart");
}

void 	VFS_Restart()
{
	VFS_Shutdown();
	VFS_Init();
	VFS_ExecAutoexec();
}


/*
================
VFS_SetGamedir

Sets the gamedir and path to a different directory.
================
*/
void 	VFS_SetGamedir(const std::string &dir)
{
	/*
	if(strstr(dir.c_str(), "..") || strstr(dir.c_str(), "/") || strstr(dir.c_str(), "\\") || strstr(dir.c_str(), ":") )
	{
		Com_Printf("VFS_SetGamedir: dir should be a single filename, not a path\n");
		return;
	}


	//
	// flush all data, so it will be forced to reload
	//
	if(dedicated && !dedicated->value)
		Cbuf_AddText("vid_restart\nsnd_restart\n");


	vfs_gamedir = vfs_userdatadir->getString() + "/" + dir;
	
	
	Com_Printf("VFS_SetGamedir: vfs_gamedir %s\n", vfs_gamedir.c_str());
	

	if((dir == VFS_BASEDIRNAME) || dir.empty())
	{
		Cvar_Set2("vfs_game", "", CVAR_LATCH | CVAR_SERVERINFO, false);
	}
	else
	{
		Cvar_Set2("vfs_game", dir, CVAR_SERVERINFO | CVAR_INIT, false);
		
		VFS_AddGameDirectory(vfs_userdatadir->getString() + "/" + dir);		
		VFS_AddGameDirectory(vfs_pkgdatadir->getString() + "/" + dir);
	}
	*/
}

/*
============
VFS_Gamedir

Called to find where to write a file (demos, savegames, etc)
============
*/
std::string	VFS_Gamedir()
{
	if(vfs_searchpaths.size())
		return vfs_searchpaths[0]->getName();
	else
		return VFS_BASEDIRNAME;
}

void 	VFS_ExecAutoexec()
{
	std::string dir;
	std::string name;

	dir = Cvar_VariableString("vfs_game");
	
	if(dir.length())
		name = "%s/%s/autoexec.cfg", std::string(vfs_basedir->getString()) + "/" + dir + "/autoexec.cfg"; 
	else
		name = "%s/%s/autoexec.cfg", std::string(vfs_basedir->getString()) + "/" + VFS_BASEDIRNAME + "/autoexec.cfg"; 
		
	if((Sys_FindFirst(name, 0, SFF_SUBDIR | SFF_HIDDEN | SFF_SYSTEM)).length())
		Cbuf_AddText("exec autoexec.cfg\n");
		
	Sys_FindClose();
}

void 	VFS_CheckForChanges()
{
	if(vfs_game->isModified())
	{
		vfs_game->isModified(false);
		
		VFS_Restart();
	}
}


int 	VFS_FOpenByMode (const std::string &filename, VFILE *stream, vfs_mode_e mode)
{
	int 	size;
	bool	sync = false;
	
	
	switch(mode)
	{
		case VFS_READ:
			size = VFS_FOpenRead(filename, &stream);
			break;
		
		case VFS_WRITE:
			size = VFS_FOpenWrite(filename, &stream);
			break;
			
		case VFS_APPEND_SYNC:
			sync = true;
		
		case VFS_APPEND:
			size = VFS_FOpenAppend(filename, &stream);
			break;
			
		default:
			Com_Error(ERR_FATAL, "VFS_OpenByMode: bad mode\n");
			return -1;
	}
	
	
	//check stream
	if(stream)
	{
		stream->pos		= VFS_FTell(stream);
		stream->size		= size;
		stream->force_flush	= sync;
	}
	
	return size;
}

/*
===========
VFS_FOpenFile

Finds the file in the search path.
returns filesize and an open FILE *
Used for streaming data out of either a pak file or
a seperate file.
===========
*/
int 	VFS_FOpenRead(const std::string &filename, VFILE **stream)
{
	searchpath_c	*search;
	std::string	netpath;
	zip_c		*pak;

	VFILE*		nstream = NULL;
	
	
	if(!filename.length())
	{
		Com_Error(ERR_FATAL, "VFS_FOpenRead: NULL filename");
		*stream = 0;
		return -1;
	}
	
	
	nstream = new VFILE;
	

	//
	// search through the path, one element at a time
	//
	for(std::vector<searchpath_c*>::const_iterator ir = vfs_searchpaths.begin(); ir != vfs_searchpaths.end(); ir++)
	{
		search = *ir;
				
		if(!search)
			continue;
			
		// is the element a real file?
		if(!search->getPack())
		{
			netpath = search->getName();
			netpath +=  "/" + filename;
			
			//Com_Printf("VFS_FOpenRead: netpath '%s'\n", netpath.c_str());
			
			// check if the file is a gzip file
			if(netpath.find(".gz") != netpath.npos)
			{
				nstream->file = gzopen(netpath.c_str(), "rb");
				if(!nstream->file)
					continue;
					
				nstream->type = FT_GZIP;
			}
			else
			{
				nstream->file = (FILE*)fopen(netpath.c_str(), "rb");
				if(!nstream->file)
					continue;
				
				nstream->type = FT_REAL;
			} 
			
			//Com_DPrintf ("FindFile: %s\n", netpath);
			
			*stream = nstream;
			
			nstream->filename = filename;
			nstream->fullpath = netpath;
			nstream->size = VFS_FSize(nstream);
			
			return nstream->size;
		}
		else
		{
			// look through all the pak file elements
			pak = search->getPack();
			zipentry_c *entry = pak->getEntry(filename);
			
			if(entry)
			{
				//if (!X_strcasecmp(pak->entries[i].name, filename.c_str()))
				{	
					// found it!
					//Com_DPrintf ("PackFile: %s : %s\n",pak->name, filename);
					
					FILE *file = NULL;
					
					
					//
					// open zip
					//
					if(!(file = fopen(pak->getName(), "rb")))
					{
						Com_Error (ERR_FATAL, "VFS_FOpenRead: couldn't reopen %s", pak->getName());
						*stream = 0;
						delete nstream;
						return -1;
					}
					
					
					//
					// get zip info
					//
					unz_s *info = (unz_s*) malloc(sizeof(unz_s));
					memcpy(info, file, sizeof(unz_s));
					info->file = (FILE*)file;
					
					unzFile unz_file = (unzFile)info;
					
					nstream->file = (FILE*)unz_file;
				
					unz_s* unz = (unz_s*)nstream->file;
					
					//
					// seek to file
					//
					FILE *unz_file2 = unz->file;
					
					unzSetCurrentFileInfoPosition(pak->getHandle(), entry->getPos()); 
										
					memcpy(nstream->file, pak->getHandle(), sizeof(*unz));
					
					unz->file = unz_file2;
								
					unzOpenCurrentFile(nstream->file);
					
										
					//set file informations
					nstream->filename = filename;
					nstream->fullpath = std::string(pak->getName()) + "#" + filename;
					nstream->type = FT_ZIP;
					nstream->size = entry->getLength();
					nstream->offset = entry->getPos();
					
					*stream = nstream;
					
					return nstream->size;
				}
			}
		}
				
	}
	
	*stream = 0;
	delete nstream;
	return -1;
}


/*
=================
VFS_FOpenWrite

Replaces fopen(filename, "wb");
=================
*/
int	VFS_FOpenWrite(const std::string &filename, VFILE **stream)
{
	VFILE*			nstream = NULL;
	std::string		name;
	
	if(!filename.length())
	{
		Com_Error(ERR_FATAL, "VFS_FOpenWrite: NULL filename");
		return -1;
	}
	
	name = VFS_Gamedir() + "/" + filename;
	
	VFS_Mkdir(name);
	
	//Com_Printf("VFS_FOpenWrite: '%s'\n", name.c_str());
	
	nstream = new VFILE;
	if(!nstream)
	{
		*stream = 0;
		return -1;
	}
	
	// check if gzip file
	if(name.substr(name.length()-3, name.length()) == ".gz")
	{
		nstream->file = gzopen(name.c_str(), "wb9");
		nstream->type = FT_GZIP;
	}
	else
	{
		nstream->file = fopen(name.c_str(), "wb");
		nstream->type = FT_REAL;
	}
	
	if(!nstream->file)
	{
		delete nstream;
		*stream = 0;
		return -1;
	}
			
		
	nstream->filename = filename;
	nstream->fullpath = name;
	nstream->force_flush = false;
	*stream = nstream;
	
	return VFS_FSize(*stream);
}

/*
=================
VFS_FOpenAppend

Replaces fopen(filename, "ab");
=================
*/
int	VFS_FOpenAppend(const std::string &filename, VFILE **stream)
{
	VFILE*			nstream = NULL;
	std::string		name;
	
	if(!filename.length())
	{
		Com_Error(ERR_FATAL, "VFS_FOpenAppend: NULL filename");
		return -1;
	}
	
	name = VFS_Gamedir() + "/" + filename;
	
	VFS_Mkdir(name);
	
	nstream = new VFILE;
	if(!nstream)
	{
		*stream = 0;
		return -1;
	}
	
	// check if gzip file
	/*
	if(name.substr(name.length()-3, name.length()) == ".gz")
	{
		nstream->file = gzopen(name.c_str(), "wb9");
		nstream->type = FT_GZIP;
	}
	else
	*/
	{
		nstream->file = fopen(name.c_str(), "ab");
		nstream->type = FT_REAL;
	}
	
	if(!nstream->file)
	{
		delete nstream;
		*stream = 0;
		return -1;
	}
			
		
	nstream->filename = filename;
	nstream->fullpath = name;
	nstream->force_flush = false;
	*stream = nstream;
	
	return VFS_FSize(*stream);
}


void 	VFS_FClose(VFILE **stream)
{
	switch((*stream)->type)
	{
		case FT_REAL:
			fclose((FILE*)(*stream)->file);
			break;
		
		case FT_ZIP:
			//unzCloseCurrentFile(stream->file);
			unzClose((*stream)->file);
			break;
		
		case FT_GZIP:
			gzclose((*stream)->file);
			break;
		
		default:
			Com_Error(ERR_FATAL, "VFS_FClose: stream '%s' has bad type %i\n", (*stream)->filename.c_str(), (*stream)->type);
	}
		
	// free file
	delete *stream;
	
	// reset pointer
	*stream = NULL;
}





/*
============
VFS_LoadFile

Filename are reletive to the quake search path
a null buffer will just return the file length without loading
============
*/
int 	VFS_FLoad(const std::string &filename, void **buffer)
{
	int	len = -1;
	
	byte*	buf = NULL;
	VFILE*	stream = NULL;
	
	if(!filename.length())
	{
		if(buffer)
			*buffer = NULL;
			
		//Com_Error(ERR_FATAL, "VFS_FLoad: NULL filename");
		return -1;
	}

	// look for it in the virtual filesystem or pack files
	len = VFS_FOpenRead(filename, &stream);
	if(!stream)
	{
		if(buffer)
			*buffer = NULL;
		return -1;
	}
	
	if(!buffer)
	{
		// if NULL was sent to buffer then VFS_FLoad is used to check
		// wether the file exists or not
		VFS_FClose(&stream);
		return len;
	}

	buf = (byte*)Com_Alloc(len+1);
	*buffer = buf;

	VFS_FRead(buf, len, stream);
	((byte*)(buf))[len] = 0;		// Tr3B -  because text files are handled as binary files
						// make sure that they have a trailing 0
	VFS_FClose(&stream);

	return len;
}



void	VFS_FSave(const std::string &filename, void *buffer, int len)
{
	VFILE*	stream = NULL;

	if(!filename.length())
	{
		Com_Error(ERR_FATAL, "VFS_FSave: NULL filename");
		return;
	}
	
	if(!buffer)
	{
		Com_Error(ERR_FATAL, "VFS_FSave: NULL buffer");
		return;	
	}
	
	VFS_FOpenWrite(filename, &stream);
	if(!stream)
	{
		Com_Error(ERR_FATAL, "VFS_FSave: can't write file %s", filename.c_str());
		return;
	}
	
	VFS_FWrite(buffer, len, stream);
	VFS_FClose(&stream);
}


void 	VFS_FFree(void *buffer)
{
	Com_Free(buffer);
}

/*
=============
VFS_FullPath

Returns the full filepath starting from /
=============
*/
const char*	VFS_FullPath(const std::string &filename)
{
	VFILE *stream = NULL;
	
	VFS_FOpenRead(filename, &stream);
	
	if(!stream)
		return "";
	else
	{
		std::string full = stream->fullpath;
		VFS_FClose(&stream);
		
		return full.c_str();
	}
}



int 	VFS_FRead(void *buffer, int len, VFILE *stream)
{
	int		read;
	byte*		buf;
	
	if(!stream)
	{
		//Com_Error(ERR_FATAL, "VFS_FRead: NULL stream");
		return -1;
	}

	buf = (byte *)buffer;

	switch(stream->type)
	{
		case FT_REAL:
			read = fread(buf, 1, len, (FILE*)stream->file);
			break;
		
		case FT_ZIP:
			read = unzReadCurrentFile(stream->file, buf, len);
			break;
		
		case FT_GZIP:
			read = gzread(stream->file, buf, len);
			break;
		
		default:
			Com_Error(ERR_FATAL, "VFS_FRead: stream '%s' has bad type %i\n", stream->filename.c_str(), stream->type);
			read = -1;
	}
	
	return read;
}


int	VFS_FWrite(const void *buffer, int len, VFILE *stream)
{
	int 		nsize;
	
	if(!stream)
	{
		//Com_Error(ERR_FATAL, "VFS_FWrite: NULL stream");
		return -1;
	}
	
	switch(stream->type)
	{
		case FT_REAL:
			nsize = fwrite(buffer, 1, len, (FILE*)stream->file);
			break;
		
		case FT_ZIP:
			nsize = -1;
			break;
		
		case FT_GZIP:
			nsize = gzwrite(stream->file, (void*)buffer, len);
			break;
		
		default:
			Com_Error(ERR_FATAL, "VFS_FWrite: stream '%s' has bad type %i\n", stream->filename.c_str(), stream->type);
			nsize = -1;
	}
		
	return nsize;
}

int	VFS_FPrintf(VFILE *stream, const char *fmt, ...)
{
	va_list ap;
	char	buf[4096];
	
	va_start(ap, fmt);
	vsprintf(buf, fmt,ap);
	
	return VFS_FWrite(buf, strlen(buf), stream);
}


int 	VFS_FSeek(VFILE *stream, long offset, vfs_seek_e type)
{
	int		whence;
	
	if(!stream)
	{
		//Com_Error(ERR_FATAL, "VFS_FSeek: NULL stream");
		return -1;
	}
	
	if(stream->type == FT_REAL || stream->type == FT_GZIP)
	{
		switch(type)
		{
			case VFS_SEEK_CUR:
				whence = SEEK_CUR;
				break;
				
			case VFS_SEEK_END:
				whence = SEEK_END;
				break;
				
			case VFS_SEEK_SET:
				whence = SEEK_SET;
				break;
				
			default:
				Com_Error(ERR_FATAL, "VFS_FSeek: bad seek type");
				whence = SEEK_SET;
		}
		
		if(stream->type == FT_REAL)
			return fseek((FILE*)stream->file, offset, whence);
		else
			return gzseek(stream->file, offset, whence);
	}	
	else if(stream->type == FT_ZIP)
	{
		if(offset == 0 && type == SEEK_SET)
		{
			unzSetCurrentFileInfoPosition(stream->file, offset);
			return unzOpenCurrentFile(stream->file);
		}
		else
		{
			Com_Error(ERR_FATAL, "VFS_FSeek: zip files not supported completely, SEEK_SET or offset were not right");
			return -1;
		}
	}
	else
	{
		Com_Error(ERR_FATAL, "VFS_FClose: stream '%s' has bad type %i\n", stream->filename.c_str(), stream->type);
		return -1;
	}
}

int 	VFS_FTell(VFILE *stream)
{
	int	ofs;

	if(!stream)
	{
		//Com_Error(ERR_FATAL, "VFS_FTell: NULL stream");
		return -1;
	}
	
	switch(stream->type)
	{
		case FT_REAL:
			ofs = ftell((FILE*)stream->file);
			break;
		
		case FT_ZIP:
			ofs = unztell(stream->file);
			break;
		
		case FT_GZIP:
			ofs = gztell(stream->file);
			break;
		
		default:
			Com_Error(ERR_FATAL, "VFS_FTell: stream '%s' has bad type %i\n", stream->filename.c_str(), stream->type);
			ofs = -1;
	}
	
	return ofs;
}

int 	VFS_FEOF(VFILE *stream)
{
	if(!stream)
	{
		//Com_Error(ERR_FATAL, "VFS_FTell: NULL stream");
		return -1;
	}
	
	switch(stream->type)
	{
		case FT_REAL:
			return feof((FILE*)stream->file);
		
		case FT_ZIP:
			return unzeof(stream->file);
		
		case FT_GZIP:
			return gzeof(stream->file);
		
		default:
			Com_Error(ERR_FATAL, "VFS_FEOF: stream '%s' has bad type %i\n", stream->filename.c_str(), stream->type);
	}
	
	return -1;
}




int 	VFS_FSize(VFILE *stream)
{
	int		pos;
	int		end;
	
	if(!stream)
	{
		//Com_Error(ERR_FATAL, "VFS_FSize: NULL stream");
		return -1;
	}

	pos = VFS_FTell(stream);
	VFS_FSeek(stream, 0, VFS_SEEK_END);
	end = VFS_FTell(stream);
	VFS_FSeek(stream, pos, VFS_SEEK_SET);
	
	return end;
}

void	VFS_FFlush(VFILE *stream)
{		
	if(!stream)
	{
		//Com_Error(ERR_FATAL, "VFS_FFlush: NULL stream");
		return;
	}
	
	//TODO check error code
	
	switch(stream->type)
	{
		case FT_REAL:
			fflush((FILE*)stream->file);
			break;
		
		//case FT_ZIP:
		//	unflush(stream->file);
		//	break;
		
		case FT_GZIP:
			gzflush(stream->file, Z_FINISH);
			break;
		
		default:
			Com_Error(ERR_FATAL, "VFS_FFlush: stream '%s' has bad type %i\n", stream->filename.c_str(), stream->type);
	}
}



/*
============
VFS_Mkdir

Creates any directories needed to store the given filename
============
*/
void	VFS_Mkdir(const std::string &path)
{
	for(unsigned int i=1; i<path.length(); i++)
	{
		if(path[i] == '/')
		{
			// create the directory
			std::string s = path.substr(0, i);	
			Sys_Mkdir(s);
		}
	}
}











