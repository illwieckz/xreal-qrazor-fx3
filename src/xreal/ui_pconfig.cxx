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
#include "../x_panim.h"

// xreal --------------------------------------------------------------------
#include "ui_local.h"




#define MAX_DISPLAYNAME 16


class playermodelinfo_c
{
public:
	playermodelinfo_c()
	{
		clear();
	}
	
	void	clear()
	{
		skindisplaynames.clear();
		displayname	= "";
		directory	= "";
		animations.clear();
		anim		= 0;
		model		= -1;
	}

	std::vector<std::string>	skindisplaynames;
	std::string			displayname;
	std::string			directory;
	std::vector<r_animation_t>	animations;
	int				anim;
	int				anim_frame;
	int				model;
};


static std::vector<playermodelinfo_c>	s_pmi;
static std::vector<std::string>		s_pmnames;


static int rate_tbl[] = { 2500, 3200, 5000, 10000, 25000, 0 };


class menu_player_model_c : public menu_spincontrol_c
{
public:
	virtual void	callback()
	{
		uii.Cvar_Set("model", s_pmi[_curvalue].directory);

		//FIXME
//		s_player_skin_box._itemnames = s_pmi[s_player_model_box._curvalue].skindisplaynames;
//		s_player_skin_box._curvalue = 0;
	}
};

class menu_player_handedness_c : public menu_spincontrol_c
{
public:
	virtual void	callback()
	{
		uii.Cvar_SetValue("hand", _curvalue);
	}
};

class menu_player_rate_c : public menu_spincontrol_c
{
public:
	virtual void	callback()
	{
		if(_curvalue != sizeof(rate_tbl) / sizeof(*rate_tbl) - 1)
			uii.Cvar_SetValue("rate", rate_tbl[_curvalue]);
	}
};

class menu_player_download_c : public menu_action_c
{
public:
	virtual void	callback()
	{
		M_Menu_DownloadOptions_f();
	}
};

class menu_player_animation_c : public menu_spincontrol_c
{
public:
	virtual void	callback()
	{
		//FIXME
		//s_pmi[s_player_model_box._curvalue].anim = (int)_curvalue;
		//s_pmi[s_player_model_box._curvalue].anim_frame = 0;
	}
};



static menu_framework_c		s_player_config_menu;
static menu_field_c			s_player_name_field;
static menu_player_model_c		s_player_model_box;
static menu_spincontrol_c		s_player_skin_box;
static menu_player_handedness_c		s_player_handedness_box;
static menu_player_rate_c		s_player_rate_box;
static menu_separator_c			s_player_model_title;
static menu_player_download_c		s_player_download_action;
static menu_player_animation_c		s_player_animation_box;



/*
static bool	PlayerConfig_ParseAnimationCFG(playermodelinfo_c &pmi)
{
	std::string	filename;

	char *buffer = NULL;
	char *ptr;
	char *token;
	
	//int	offset;
	bool	nooffset;
	vec3_c	headoffset;
	
	int	anim = 0;
	int	frames_upper = 0;
	int	frames_lower = 0;
	std::vector<r_anim_sequence_t> upper_sequences;
	std::vector<r_anim_sequence_t> lower_sequences;
	
	//
	// load the animation.cfg
	//
	filename = "models/players/" + pmi.directory + "/animation.cfg";
	uii.VFS_FLoad(filename, (void**)&buffer);
	if(!buffer)
	{
		uii.Com_Printf("PlayerConfig_ParseAnimationCFG: bad config '%s\n", filename.c_str());
		return false;
	}
		
	//
	// parse buffer
	//
	ptr = buffer;
	while(ptr)
	{
		token = Com_Parse(&ptr);
		
		if(!token)
		{
			break;
		}		
		else if(!isdigit(token[0]))
		{
			if(X_strcaseequal(token, "sex"))
			{
				// skip gender
				Com_Parse(&ptr);
			
				continue;
			}
		
			if(X_strcaseequal(token, "headoffset"))
			{
				// parse headoffset origin
			
				for(int i=0; i<3; i++)
				{	
					token = Com_Parse(&ptr);
			
					if(!token)
						break;
					else
						headoffset[i] = atof(token);
				}
														
				continue;
			}
		
			if(X_strcaseequal(token, "footsteps"))
			{
				// skip it
				Com_Parse(&ptr);
			
				continue;
			}
			
			if(X_strcaseequal(token, "nooffset"))
			{
				nooffset = true;
				
				continue;
			}
		}
		else
		{
			r_anim_sequence_t	a;
		
			// well ignore first token
			a.first_frame		= atoi(token);			// first frame
			a.frames_num		= Com_ParseInt(&ptr);		// frames num
			a.looping_frames	= Com_ParseInt(&ptr);		// looping frames
			a.frames_per_second	= Com_ParseInt(&ptr);		// frames per second
						
							
#if 0
			uii.Com_Printf("PlayerConfig_ParseAnimationCFG: %i %i %i %i\n", a.first_frame,
											a.frames_num,
											a.looping_frames,
											a.frames_per_second);
#endif
			
			if(anim <= PLAYER_ANIM_UPPER_DEATH3)	// upper and lower share this animation frames
			{
				upper_sequences.push_back(a);
				lower_sequences.push_back(a);
			}
			else if(anim <= PLAYER_ANIM_UPPER_STAND2)
			{
				upper_sequences.push_back(a);
			}
			else if(anim <= (PLAYER_ANIM_UPPER_STAND2 + (PLAYER_ANIM_LOWER_TURN - PLAYER_ANIM_LOWER_DEATH3)))
			{
				lower_sequences.push_back(a);
			}
			
			anim++;
		}
	}
	
	uii.VFS_FFree(buffer);
	
	//uii.Com_Printf("PlayerConfig_ParseAnimationCFG: animation sequences %i %i\n", upper_sequences.size(), lower_sequences.size());

	pmi.upper_sequences = upper_sequences;
	pmi.lower_sequences = lower_sequences;
	
	
	// setup upper frames
	for(int i=0; i<=(PLAYER_ANIM_UPPER_STAND2); i++)
	{
		if(i==0)
		{
			pmi.upper_sequences[i].first_frame = 0;
			pmi.upper_sequences[i].last_frame = 0;
		}
		else
		{
			pmi.upper_sequences[i].first_frame = (frames_upper += pmi.upper_sequences[i-1].frames_num);
			pmi.upper_sequences[i].last_frame = pmi.upper_sequences[i].first_frame + pmi.upper_sequences[i].frames_num;
		}
	}
	
	
	// setup lower frames
	for(int i=0; i<=(PLAYER_ANIM_LOWER_TURN); i++)
	{
		if(i==0)
		{
			pmi.lower_sequences[i].first_frame = 0;
			pmi.lower_sequences[i].last_frame = 0;
		}
		else
		{
			pmi.lower_sequences[i].first_frame = (frames_lower += pmi.lower_sequences[i-1].frames_num);
			pmi.lower_sequences[i].last_frame = pmi.lower_sequences[i].first_frame + pmi.lower_sequences[i].frames_num;
		}
	}
		
	
#if 1
	pmi.upperframe = pmi.upper_sequences[PLAYER_ANIM_UPPER_STAND].first_frame;
	pmi.lowerframe = pmi.lower_sequences[PLAYER_ANIM_LOWER_STAND].first_frame;
#else
	pmi.upperframe = 0;
	pmi.lowerframe = 0;
#endif
		
	//uii.Com_Printf("PlayerConfig_ParseAnimationCFG: upper %i lower %i\n", pmi.upperframe, pmi.lowerframe);
	
	return true;
}
*/

static bool	PlayerConfig_ScanAnimations(playermodelinfo_c &pmi, const std::vector<std::string> &md5anim_names)
{
#if 0
	pmi.model = uii.R_RegisterModel("models/players/" + pmi.directory + "/body.md5mesh");
	if(pmi.model == -1)
	{
		uii.Com_Printf("PlayerConfig_ScanAnimations: no body.md5mesh in '%s'\n", pmi.directory.c_str());
		return false;
	}
	
	/*
	for(std::vector<std::string>::const_iterator ir = md5anim_names.begin(); ir != md5anim_names.end(); ir++)
	{
		r_animation_t	anim;
	
		//uii.R_RegisterAnimation(pmi.model, "models/players/" + pmi.directory + "/idle.md5anim");
		anim.sequence = uii.R_RegisterAnimation(*ir);
		
		pmi.animations.push_back(anim);
	}
	*/
	
	for(const player_anim_t *anim=player_anims; anim->name; anim++)
	{
		std::string name = "models/players/" + pmi.directory + "/" + std::string(anim->name) + ".md5anim";
		
		if(!uii.VFS_FLoad(name, NULL))
		{
			Com_Error(ERR_DROP, "PlayerConfig_ScanAnimations: player model '%s' does not provide animation '%s'", pmi.directory.c_str(), name.c_str());
			return false;
		}
		
		r_animation_t	anim;
	
		anim.sequence = uii.R_RegisterAnimation(name);
		
		pmi.animations.push_back(anim);
	}
	
	return true;
#else
	return false;
#endif
}

static void	PlayerConfig_ScanDirectories()
{
	std::vector<std::string>	dir_names;
	int i;

	
	for(std::vector<playermodelinfo_c>::iterator ir = s_pmi.begin(); ir != s_pmi.end(); ir++)
		(*ir).clear();
		
	s_pmi.clear();

	//
	// get a list of directories
	//
	if((dir_names = uii.VFS_ListFiles("models/players", "")).size() == 0)
		return;
	

	//
	// go through the subdirectories
	//
	for(i=0; i<(int)dir_names.size(); i++)
	{
		unsigned int j, k;
		
		std::vector<std::string>	skin_names;
		std::vector<std::string>	skin_names_final;
		std::vector<std::string>	md5mesh_names;
		std::vector<std::string>	md5anim_names;

		if(dir_names[i].length() == 0)
			continue;
			
				
		if(dir_names[i][dir_names[i].length()-1] != '/')
			continue;
		else
			dir_names[i] = dir_names[i].substr(0, dir_names[i].length()-1);
			
		uii.Com_Printf("PlayerConfig_ScanDirectories: scanning '%s' ...\n", dir_names[i].c_str());


		// verify the existence of at least one .skin file
		skin_names = uii.VFS_ListFiles(dir_names[i], ".skin");
		if(!skin_names.size())
		{
			dir_names[i] = "";
			continue;
		}


		// verify the existence of at least one .md5mesh file
		md5mesh_names = uii.VFS_ListFiles(dir_names[i], ".md5mesh");
		if(!md5mesh_names.size())
		{
			dir_names[i] = "";
			continue;
		}
		
		// verify the existence of at least one .md5anim file
		md5anim_names = uii.VFS_ListFiles(dir_names[i], ".md5anim");
		if(!md5anim_names.size())
		{
			dir_names[i] = "";
			continue;
		}


		// count valid skins, which consist of a skin with a matching "_i" icon
		for(k=0; k<skin_names.size(); k++)
		{
			std::string skin_name_full = skin_names[k];
			std::string skin_name;
			bool add = true;
			
			skin_name = skin_name_full.substr(skin_name_full.find("_")+1, skin_name_full.length());
			skin_name = skin_name.substr(0, skin_name.find('.'));
					
			//uii.Com_Printf("PlayerConfig_ScanDirectories: '%s'\n", skinname.c_str());
	
			for(j=0; j<skin_names_final.size(); j++)
			{
				if(strcmp(skin_names_final[j].c_str(), skin_name.c_str()) == 0)
					add = false;
			}

			if(add)
			{
				skin_names_final.push_back(skin_name);
			}
		}
		
		if(!skin_names_final.size())
			continue;
		
			
		//
		// setup player model info
		//	
		playermodelinfo_c	pmi;
		
		
		// at this point we have a valid player model
		//s_pmi[s_numplayermodels].nskins			= skinnames_final.size();
		pmi.skindisplaynames = skin_names_final;

		// make short name for the model
#if 1
		char *a, *b, *c;
		
		a = strrchr(dir_names[i].c_str(), '/');
		b = strrchr(dir_names[i].c_str(), '\\');

		if(a > b)
			c = a;
		else
			c = b;

		pmi.displayname = c + 1;//, MAX_DISPLAYNAME-1;
		pmi.directory = c + 1;
#else
		pmi.displayname = dir_names[i];
		pmi.directory = dirn_ames[i];
#endif
	
	
		if(!PlayerConfig_ScanAnimations(pmi, md5anim_names))
			continue;
	
		
		s_pmi.push_back(pmi);
	}
}

/*
static int pmicmpfnc(const void *_a, const void *_b)
{
	const playermodelinfo_c *a = (const playermodelinfo_c*) _a;
	const playermodelinfo_c *b = (const playermodelinfo_c*) _b;

	//
	// sort by male, female, then alphabetical
	//
	if ( strcmp( a->directory.c_str(), "male" ) == 0 )
		return -1;
	else if ( strcmp( b->directory.c_str(), "male" ) == 0 )
		return 1;

	if ( strcmp( a->directory.c_str(), "female" ) == 0 )
		return -1;
	else if ( strcmp( b->directory.c_str(), "female" ) == 0 )
		return 1;

	return strcmp( a->directory.c_str(), b->directory.c_str() );
}
*/


static bool	PlayerConfig_MenuInit()
{
	//extern cvar_t *name;
	//extern cvar_t *skin;
	char currentdirectory[1024];
	char currentskin[1024];
	int i = 0;

	int currentdirectoryindex = 0;
	int currentskinindex = 0;

	//cvar_t *hand = Cvar_Get( "hand", "0", CVAR_USERINFO | CVAR_ARCHIVE );

	PlayerConfig_ScanDirectories();

	if(s_pmi.size() == 0)
	{
		//uii.Com_DPrintf("PlayerConfig_MenuInit: no player models\n");
		return false;
	}
	
	

	if(uii.Cvar_VariableValue("hand") < 0 || uii.Cvar_VariableValue("hand") > 2)
		uii.Cvar_SetValue("hand", 0);

	strcpy(currentdirectory, uii.Cvar_VariableString("skin"));

	if(strchr(currentdirectory, '/'))
	{
		strcpy(currentskin, strchr(currentdirectory, '/') + 1);
		*strchr(currentdirectory, '/') = 0;
	}
	else if (strchr(currentdirectory, '\\' ))
	{
		strcpy(currentskin, strchr(currentdirectory, '\\') + 1);
		*strchr(currentdirectory, '\\') = 0;
	}
	else
	{
		strcpy(currentdirectory, "male");
		strcpy(currentskin, "grunt");
	}

	//qsort(s_pmi, s_numplayermodels, sizeof(s_pmi[0]), pmicmpfnc);
	s_pmnames.clear();

	for(i=0; i<(int)s_pmi.size(); i++)
	{
		s_pmnames.push_back(s_pmi[i].displayname);
		
		if(X_stricmp(s_pmi[i].directory.c_str(), currentdirectory) == 0)
		{
			unsigned int j;

			currentdirectoryindex = i;

			for(j=0; j<s_pmi[i].skindisplaynames.size(); j++)
			{
				if(X_stricmp(s_pmi[i].skindisplaynames[j].c_str(), currentskin) == 0)
				{
					currentskinindex = j;
					break;
				}
			}
		}
	}
	
	int	y;
	int	y_offset = CHAR_MEDIUM_HEIGHT + 5;

	s_player_config_menu._x = uii.viddef->width/2 - CHAR_MEDIUM_WIDTH * 13; 
	s_player_config_menu._y = uii.viddef->height/2 - 150;

	s_player_name_field._fontflags	= FONT_MEDIUM;
	s_player_name_field._name = "name";
	s_player_name_field._x		= 0;
	s_player_name_field._y		= y = 0;
	s_player_name_field._length	= 20;
	//s_player_name_field._visible_length = 20;
	s_player_name_field._buffer = uii.Cvar_VariableString("name");
	s_player_name_field._cursor = strlen(uii.Cvar_VariableString("name"));

	
	s_player_model_box._fontflags	= FONT_MEDIUM;
	s_player_model_box._name	= "model";
	s_player_model_box._x	= 0;
	s_player_model_box._y	= y += y_offset;
	s_player_model_box._cursor_offset = -48;
	s_player_model_box._curvalue = currentdirectoryindex;
	s_player_model_box._itemnames = s_pmnames;

	
	s_player_skin_box._fontflags = FONT_MEDIUM;
	s_player_skin_box._x	= 0;
	s_player_skin_box._y	= y += y_offset;
	s_player_skin_box._name	= "skin";
	s_player_skin_box._cursor_offset = -48;
	s_player_skin_box._curvalue = currentskinindex;
	s_player_skin_box._itemnames = s_pmi[currentdirectoryindex].skindisplaynames;

	s_player_handedness_box._fontflags = FONT_MEDIUM;
	s_player_handedness_box._x	= 0;
	s_player_handedness_box._y	= y += y_offset;
	s_player_handedness_box._name	= "hand";
	s_player_handedness_box._cursor_offset = -48;
	s_player_handedness_box._curvalue = uii.Cvar_VariableInteger("hand");
	s_player_handedness_box._itemnames.push_back("right");
	s_player_handedness_box._itemnames.push_back("left");
	s_player_handedness_box._itemnames.push_back("center");

	for (i = 0; i < (int)sizeof(rate_tbl) / (int)sizeof(*rate_tbl) - 1; i++)
		if (uii.Cvar_VariableValue("rate") == rate_tbl[i])
			break;


	//s_player_rate_box._type = MTYPE_SPINCONTROL;
	s_player_rate_box._fontflags	= FONT_MEDIUM;
	s_player_rate_box._x		= 0;
	s_player_rate_box._y		= y += y_offset;
	s_player_rate_box._name		= "connect speed";
	s_player_rate_box._cursor_offset = -48;
	s_player_rate_box._curvalue = i;
	//static const char *rate_names[] = { "28.8 Modem", "33.6 Modem", "Single ISDN", "Dual ISDN/Cable", "T1/LAN", "User defined", 0 };
	s_player_rate_box._itemnames.push_back("28.8 Modem");
	s_player_rate_box._itemnames.push_back("33.6 Modem");
	s_player_rate_box._itemnames.push_back("Dual ISDN/Cable");
	s_player_rate_box._itemnames.push_back("T1/LAN");
	s_player_rate_box._itemnames.push_back("User defined");

	//s_player_download_action._type = MTYPE_ACTION;
	s_player_download_action._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_player_download_action._name	= "download options";
	s_player_download_action._flags= QMF_LEFT_JUSTIFY;
	s_player_download_action._x	= 0;
	s_player_download_action._y	= y += y_offset;
	//s_player_download_action._statusbar = "";
	
	//uii.Com_Printf("PlayerConfig_MenuInit: s_player_model_box._curvalue %i\n", s_player_model_box._curvalue);
	//uii.Com_Printf("PlayerConfig_MenuInit: s_pmi.size() %i\n", s_pmi.size());
	//uii.Com_Printf("PlayerConfig_MenuInit: anims %i\n", s_pmi[s_player_model_box._curvalue].anim_data.size());
	
	/*
	s_player_upper_slider._flags		= QMF_LEFT_JUSTIFY;
	s_player_upper_slider._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_player_upper_slider._x		= 0;
	s_player_upper_slider._y		= y += y_offset*2;
	s_player_upper_slider._name		= "upper frame";
	s_player_upper_slider._minvalue 	= 0;
	s_player_upper_slider._maxvalue 	= s_pmi[s_player_model_box._curvalue].upper_sequences[PLAYER_ANIM_UPPER_STAND2].last_frame;
	s_player_upper_slider._curvalue		= s_pmi[s_player_model_box._curvalue].upperframe;
	s_player_upper_slider._callback		= UpperCallback;
	
	s_player_lower_slider._flags		= QMF_LEFT_JUSTIFY;
	s_player_lower_slider._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_player_lower_slider._x		= 0;
	s_player_lower_slider._y		= y += y_offset;
	s_player_lower_slider._name		= "lower frame";
	s_player_lower_slider._minvalue 	= 0;
	s_player_lower_slider._maxvalue 	= s_pmi[s_player_model_box._curvalue].lower_sequences[PLAYER_ANIM_LOWER_TURN].last_frame;
	s_player_lower_slider._curvalue		= s_pmi[s_player_model_box._curvalue].lowerframe;
	s_player_lower_slider._callback		= LowerCallback;
	*/
	
	s_player_animation_box._flags	= QMF_LEFT_JUSTIFY;
	s_player_animation_box._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_player_animation_box._x		= 0;
	s_player_animation_box._y		= y += y_offset;
	s_player_animation_box._name		= "anim sequence";
	//s_player_animation_box._minvalue	= 0;
	//s_player_animation_box._maxvalue 	= s_pmi[s_player_model_box._curvalue].animations.size() - 1;
	s_player_animation_box._curvalue	= 0;

	for(const player_anim_t *anim=player_anims; anim->name; anim++)
	{
		s_player_animation_box._itemnames.push_back(anim->name);	
	}	

	s_player_config_menu.addItem(&s_player_name_field);
	s_player_config_menu.addItem(&s_player_model_box);
	
	if(s_player_skin_box._itemnames.size())
	{
		s_player_config_menu.addItem(&s_player_skin_box);
	}
	
	s_player_config_menu.addItem(&s_player_handedness_box);
	s_player_config_menu.addItem(&s_player_rate_box);
	s_player_config_menu.addItem(&s_player_download_action);
	
	
	if(uii.Cvar_VariableInteger("developer"))
	{
		s_player_config_menu.addItem(&s_player_animation_box);
	}
	
	
	//s_player_config_menu.center();

	return true;
}


static void	PlayerConfig_MenuDraw()
{
	r_refdef_t refdef;
	std::string	scratch;

	refdef.x = uii.viddef->width / 2;
	refdef.y = uii.viddef->height / 2 - 50;
	refdef.width = 300;
	refdef.height = 400;
	refdef.setFOV(45);
	//refdef.time = uii.CL_GetRealtime() * 0.001;
	refdef.areabits = NULL;
	refdef.rdflags = RDF_NOWORLDMODEL;

	if(s_pmi[s_player_model_box._curvalue].skindisplaynames.size())
	{
	
		//static int	time = 0;
#if 1
		// turn model
		static vec3_c	angles(0, 90, 0);

		angles[1] += 2.0f;
		
		if(angles[1] > 360)
			angles[1] = 0;	
#else
		// just draw it with in front view
		static vec3_c angles(0, 180, 0);
#endif
			
		//uii.R_ClearScene();
		

#if 0
		r_entity_t		lower;
		r_orientation_t		upper_lerped;

		//
		// lower
		//
		scratch = "models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/lower.md3";
		lower.model = uii.R_RegisterModel(scratch);
		
		lower.custom_shader = -1;
		
		scratch = "models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/" + "lower_" + s_pmi[s_player_model_box._curvalue].skindisplaynames[s_player_skin_box._curvalue] + ".skin";
		lower.custom_skin = uii.R_RegisterSkin(scratch);
		
		lower.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DEPTHHACK;
		lower.origin.set(120, 0, 0);
		lower.origin_old = lower.origin;
		
		lower.quat.fromAngles(angles);

		lower.frame = s_pmi[s_player_model_box._curvalue].lowerframe;
		lower.frame_old = lower.frame;
		
		uii.R_AddEntityToScene(lower);
		
		
		if(uii.R_LerpAttachment(upper_lerped, lower, "tag_torso"))
		{
			r_entity_t	upper;
			r_orientation_t	head_lerped;
		
			//
			// upper
			//
			scratch = "models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/upper.md3";
			upper.model = uii.R_RegisterModel(scratch);
		
			upper.custom_shader = -1;
		
			scratch = "models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/" + "upper_" + s_pmi[s_player_model_box._curvalue].skindisplaynames[s_player_skin_box._curvalue] + ".skin";
			upper.custom_skin = uii.R_RegisterSkin(scratch);
			
			upper.frame = s_pmi[s_player_model_box._curvalue].upperframe;
			upper.frame_old = upper.frame;
			
			upper.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DEPTHHACK;
			
			upper.origin = lower.origin;
			
			matrix_c axis;
			axis.fromQuaternion(lower.quat);
			for(int i=0; i<3; i++)  
				Vector3_MA(upper.origin, upper_lerped.origin[i], axis[i], upper.origin);
		
			upper.quat = lower.quat * upper_lerped.quat;
					
			uii.R_AddEntityToScene(upper);
			
			if(uii.R_LerpAttachment(head_lerped, upper, "tag_head"))
			{
				r_entity_t head;
				
				//
				// head
				//
				scratch = "models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/head.md3";
				head.model = uii.R_RegisterModel(scratch);
		
				head.custom_shader = -1;
		
				scratch = "models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/" + "head_" + s_pmi[s_player_model_box._curvalue].skindisplaynames[s_player_skin_box._curvalue] + ".skin";
				head.custom_skin = uii.R_RegisterSkin(scratch);
			
				head.frame = 0;
				head.frame_old = 0;
			
				head.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DEPTHHACK;
						
				head.origin = upper.origin;
				
				matrix_c axis;
				axis.fromQuaternion(upper.quat);
				for(int i=0; i<3; i++)  
					Vector3_MA(head.origin, head_lerped.origin[i], axis[i], head.origin);
								
				head.quat = upper.quat * head_lerped.quat;
						
				uii.R_AddEntityToScene(head);
			}
		}

#else
		//
		// MD5 model viewer
		//
		/*
		
		// body
		r_entity_t	body;
	
		body.model = uii.R_RegisterModel("models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/body.md5mesh");
		
		if(!uii.R_SetupAnimation(body.model, s_pmi[s_player_model_box._curvalue].anim))
			uii.Com_Error(ERR_DROP, "animation setup failed");
			
		if(uii.Sys_Milliseconds() - 41 > time)	// 41 milliseconds -> 24 frames per second
		{
			time = uii.Sys_Milliseconds();
			s_pmi[s_player_model_box._curvalue].anim_frame++;
		}
		
		body.custom_skin = uii.R_RegisterSkin("models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/body_" + s_pmi[s_player_model_box._curvalue].skindisplaynames[s_player_skin_box._curvalue] + ".skin");

						
		body.flags = RF_FULLBRIGHT | RF_NOSHADOW | RF_DEPTHHACK;
		body.origin.set(150, 0, 0);
		body.origin2 = body.origin;

		body.quat.fromAngles(angles);
				
		body.frame = s_pmi[s_player_model_box._curvalue].anim_frame;
		body.frame_old = body.frame;
		
		uii.R_AddEntity(body);
		
		
		// light
		r_entity_t	light;
		
		light.type = ET_LIGHT_OMNI;
		light.custom_shader = uii.R_RegisterLight("lights/defaultPointLight");
		light.color = color_white;
		light.origin.set(0, 100, 100);
		light.radius.set(1000, 1000, 1000);
		light.radius_bbox._maxs = light.origin + light.radius;
		light.radius_bbox._mins = light.origin - light.radius;
		light.radius_value = light.radius_bbox.radius();
		
		uii.R_AddLightToScene(light);
		*/
#endif
		
		s_player_config_menu.draw();

		//M_DrawTextBox((int)((refdef.x) * (640.0F / uii.viddef->width ) - 8), (int)(( uii.viddef->height / 2 ) * (480.0F / uii.viddef->height) - 77), refdef.width / 8, refdef.height / 8);
		refdef.height += 4;

		//uii.R_RenderFrame(refdef);

		//
		// render icon
		//
		scratch = "models/players/" + s_pmi[s_player_model_box._curvalue].directory + "/icon_" + 
			s_pmi[s_player_model_box._curvalue].skindisplaynames[s_player_skin_box._curvalue];
			
		//uii.Com_Printf("PlayerConfig_MenuDraw: icon '%s'\n", scratch.c_str());
			
		//uii.R_DrawStretchPic(s_player_config_menu._x - 40, s_player_config_menu._y + 32, 32, 32, 0, 0, 1, 1, color_white, uii.R_RegisterPic(scratch));
		
	}
}

static const std::string	PlayerConfig_MenuKey(int key)
{
	if(key == K_ESCAPE)
	{
		std::string	scratch;

		uii.Cvar_Set("name", s_player_name_field._buffer);

		scratch =	s_pmi[s_player_model_box._curvalue].directory + 
				"/" + 
				s_pmi[s_player_model_box._curvalue].skindisplaynames[s_player_skin_box._curvalue];

		uii.Cvar_Set("skin", (char*)scratch.c_str());

		for(unsigned int i=0; i<s_pmi.size(); i++)
		{
			/*
			for(unsigned int j=0; j<s_pmi[i].skindisplaynames.size(); j++)
			{
				s_pmi[i].skindisplaynames[j] = "";
			}
			*/
			s_pmi[i].skindisplaynames.clear();
		}
	}
	
	return	Default_MenuKey(&s_player_config_menu, key);
}


void	M_Menu_PlayerConfig_f()
{
	if(!PlayerConfig_MenuInit())
	{
		s_multiplayer_menu.setStatusBar("No valid player models found");
		return;
	}
	
	//s_multiplayer_menu.setStatusBar("");
	M_PushMenu(PlayerConfig_MenuDraw, PlayerConfig_MenuKey);
}

