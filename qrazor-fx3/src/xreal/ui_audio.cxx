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
#include "ui_local.h"

class menu_audio_framework_c : public menu_framework_c
{
public:
	virtual void	draw()
	{
		M_Banner("AUDIO");
	
		drawGeneric();
	}
	
	virtual std::string	keyDown(int key)
	{
		return defaultKeyDown(key);
	}
};


class menu_audio_volume_sfx_c : public menu_slider_c
{
public:
	menu_audio_volume_sfx_c()
	:menu_slider_c("effects", 0, 10)
	{
	}

	virtual void	callback()
	{
		trap_Cvar_SetValue("s_sfxvolume", _curvalue / 10);
	}
};

class menu_audio_volume_music_c : public menu_slider_c
{
public:
	menu_audio_volume_music_c()
	:menu_slider_c("music volume", 0, 10)
	{
	}
	
	virtual void	callback()
	{
		trap_Cvar_SetValue("s_musicvolume", _curvalue / 10);
	}
};

class menu_audio_quality_list_c : public menu_spincontrol_c
{
public:
	menu_audio_quality_list_c()
	{
		_name		= "sound quality";
	
		_itemnames.push_back("low");
		_itemnames.push_back("medium");
		_itemnames.push_back("high");
	}

	virtual void	callback()
	{
		if(_curvalue == 0)
			trap_Cvar_SetValue("s_khz", 11);
	
		else if(_curvalue == 1)
			trap_Cvar_SetValue("s_khz", 22);
		
		else if(_curvalue == 2)
			trap_Cvar_SetValue("s_khz", 44);
		
	

		//M_DrawTextBox( 8, 120 - 48, 36, 3 );
		//M_Print( 16 + 16, 120 - 48 + 8,  "Restarting the sound system. This");
		//M_Print( 16 + 16, 120 - 48 + 16, "could take up to a minute, so");
		//M_Print( 16 + 16, 120 - 48 + 24, "please be patient.");

		// the text box won't show up unless we do a buffer swap
		//trap_R_EndFrame();

		trap_Cbuf_AddText("snd_restart\n");
	}
};

static menu_audio_framework_c			s_audio_menu;
static menu_audio_volume_sfx_c			s_audio_sfxvolume_slider;									
static menu_audio_volume_music_c		s_audio_musicvolume_slider;
static menu_audio_quality_list_c		s_audio_quality_list;


static void	M_AudioInit()
{
	int	y = 0;
	int	y_offset = CHAR_MEDIUM_HEIGHT + 5;
	
	s_audio_menu._x = (int)(trap_VID_GetWidth() * 0.50 - (CHAR_MEDIUM_WIDTH * 10));
	
	s_audio_sfxvolume_slider._fontflags	= FONT_MEDIUM;
	s_audio_sfxvolume_slider._x		= 0;
	s_audio_sfxvolume_slider._y		= y = 0;
	s_audio_sfxvolume_slider._curvalue	= trap_Cvar_VariableValue("s_sfxvolume") * 10;
	
	s_audio_musicvolume_slider._fontflags	= FONT_MEDIUM | FONT_CHROME;
	s_audio_musicvolume_slider._x		= 0;
	s_audio_musicvolume_slider._y		= y += y_offset;
	s_audio_musicvolume_slider._curvalue	= trap_Cvar_VariableValue("s_musicvolume") * 10;

	s_audio_quality_list._fontflags		= FONT_MEDIUM;// | FONT_CHROME;
	s_audio_quality_list._x			= 0;
	s_audio_quality_list._y			= y += y_offset;
	
	int kHz = trap_Cvar_VariableInteger("s_khz");
	if(kHz == 11)
		s_audio_quality_list._curvalue = 0;
		
	else if(kHz == 22)
		s_audio_quality_list._curvalue = 1;
	
	else if(kHz == 44)
		s_audio_quality_list._curvalue = 2;
	
	s_audio_menu.addItem(&s_audio_sfxvolume_slider);
	s_audio_menu.addItem(&s_audio_musicvolume_slider);
	s_audio_menu.addItem(&s_audio_quality_list);
	
	s_audio_menu.center();
}

void	M_Menu_Audio_f()
{
	M_AudioInit();
	M_PushMenu(&s_audio_menu);
}
