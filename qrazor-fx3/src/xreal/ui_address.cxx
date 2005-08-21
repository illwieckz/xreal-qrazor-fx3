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

/*
#define NUM_ADDRESSBOOK_ENTRIES 9

static menu_framework_c	s_addressbook_menu;
static menu_field_c	s_addressbook_fields[NUM_ADDRESSBOOK_ENTRIES];

void	AddressBook_MenuInit()
{
	int i;

	s_addressbook_menu._x = uii.viddef->width / 2 - 142;
	s_addressbook_menu._y = uii.viddef->height / 2 - 58;

	for(i=0; i<NUM_ADDRESSBOOK_ENTRIES; i++)
	{
		cvar_t *adr;
		char buffer[20];

		Com_sprintf(buffer, sizeof(buffer), "adr%d", i);

		adr = uii.Cvar_Get(buffer, "", CVAR_ARCHIVE);

		s_addressbook_fields[i]._x		= 0;
		s_addressbook_fields[i]._y		= i * 18 + 0;
		s_addressbook_fields[i]._localdata[0] 	= i;
		s_addressbook_fields[i]._cursor		= 0;
		s_addressbook_fields[i]._length		= 60;
		//s_addressbook_fields[i]._visible_length	= 30;

		s_addressbook_fields[i]._buffer = adr->getString();

		s_addressbook_menu.addItem(&s_addressbook_fields[i]);
	}
}

const std::string	AddressBook_MenuKey(int key)
{
	if(key == K_ESCAPE)
	{
		int index;
		char buffer[20];

		for(index=0; index<NUM_ADDRESSBOOK_ENTRIES; index++)
		{
			Com_sprintf(buffer, sizeof(buffer), "adr%d", index);
			uii.Cvar_Set(buffer, s_addressbook_fields[index]._buffer);
		}
	}
	
	return Default_MenuKey(&s_addressbook_menu, key);
}

void	AddressBook_MenuDraw()
{
	M_Banner("textures/pics/m_banner_addressbook.pcx");
	
	s_addressbook_menu.draw();
}

void	M_Menu_AddressBook_f()
{
	AddressBook_MenuInit();
	M_PushMenu(AddressBook_MenuDraw, AddressBook_MenuKey);
}
*/

