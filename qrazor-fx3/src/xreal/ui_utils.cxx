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
static void	Action_DoEnter( menuaction_s *a );
static void	Action_Draw( menuaction_s *a );
static void  	Menu_DrawStatusBar( const std::string &string );
//static void	Menulist_DoEnter( menulist_s *l );
static void	MenuList_Draw( menulist_s *l );
static void	Separator_Draw( menuseparator_s *s );
static void	Slider_DoSlide( menuslider_s *s, int dir );
static void	Slider_Draw( menuslider_s *s );
//static void	SpinControl_DoEnter( menulist_s *s );
static void	SpinControl_Draw( menulist_s *s );
static void	SpinControl_DoSlide( menulist_s *s, int dir );
*/

//#define RCOLUMN_OFFSET  16
//#define LCOLUMN_OFFSET -16


/*
================================================================================
				MENU ACTION ITEM
================================================================================
*/
void	menu_action_c::doEnter()
{
	callback();
}

void	menu_action_c::draw()
{
	Menu_DrawString(_x + _parent->_x, _y + _parent->_y, _name, _fontflags);	
	
	if(_ownerdraw)
		_ownerdraw(this);
}


/*
================================================================================
				MENU FIELD ITEM
================================================================================
*/
void	menu_field_c::doEnter()
{
	callback();
}

void	menu_field_c::draw()
{

	//
	// draw field name
	//
	if(_name.length())
		Menu_DrawString(_x + _parent->_x, _y + _parent->_y, _name, _fontflags | FONT_ALT | FONT_CHROME);
	
	//
	// draw field box
	//
	int cwidth = Menu_GetCharWidth(_fontflags);
	trap_R_DrawFill((_name.length()+1)*cwidth + _x + _parent->_x, _y + _parent->_y, cwidth * _length, cwidth, color_grey_med);
	
	
	//
	// draw buffer
	//
	Menu_DrawString((_name.length()+1)*cwidth + _x + _parent->_x, _y + _parent->_y, _buffer, _fontflags);
	
	
	//
	// draw cursor
	//
	if(_parent->getItemAtCursor() == this)
	{
		/*
		int offset;

		if(_visible_offset)
			offset = _visible_length;
		else
			offset = _cursor;
		*/

		if(((int)(trap_Sys_Milliseconds() / 250)) & 1)
		{
			Menu_DrawChar((_name.length()+1)*cwidth + _x + _parent->_x + _cursor*cwidth, _y + _parent->_y, 11, color_white, _fontflags);
		}
		else
		{
			Menu_DrawChar((_name.length()+1)*cwidth + _x + _parent->_x + _cursor*cwidth, _y + _parent->_y, ' ', color_white, _fontflags);
		}
	}
}

bool	menu_field_c::key(int key)
{

	switch(key)
	{
		case K_KP_SLASH:
			key = '/';
			break;
			
		case K_KP_MINUS:
			key = '-';
			break;
			
		case K_KP_PLUS:
			key = '+';
			break;
			
		case K_KP_HOME:
			key = '7';
			break;
			
		case K_KP_UPARROW:
			key = '8';
			break;
			
		case K_KP_PGUP:
			key = '9';
			break;
			
		case K_KP_LEFTARROW:
			key = '4';
			break;
			
		case K_KP_5:
			key = '5';
			break;
			
		case K_KP_RIGHTARROW:
			key = '6';
			break;
			
		case K_KP_END:
			key = '1';
			break;
			
		case K_KP_DOWNARROW:
			key = '2';
			break;
			
		case K_KP_PGDN:
			key = '3';
			break;
			
		case K_KP_INS:
			key = '0';
			break;
			
		case K_KP_DEL:
			key = '.';
			break;
	}

	
	if(key > 127)
	{
		switch(key)
		{
			case K_DEL:
			default:
				return false;
		}
	}


	switch(key)
	{
		case K_KP_LEFTARROW:
		case K_LEFTARROW:
		case K_BACKSPACE:
			if(_cursor > 0)
			{
				//memmove(&_buffer[_cursor-1], &_buffer[_cursor], strlen(&_buffer[_cursor]) + 1);
				_buffer = _buffer.substr(0, _cursor-1);// + _buffer.substr(_cursor, _buffer.length());
				_cursor--;
				
			}
			break;

		case K_KP_DEL:
		case K_DEL:
			//memmove(&_buffer[_cursor], &_buffer[_cursor+1], strlen(&_buffer[_cursor+1]) + 1);
			//_buffer = _buffer.substr(0, _buffer.length()-1);
		//	break;

		case K_KP_ENTER:
		case K_ENTER:
		case K_ESCAPE:
		case K_TAB:
			return false;

		case K_SPACE:
		default:
			if(!isdigit(key) && (_flags & QMF_NUMBERSONLY))
				return false;

			if(_cursor < _length)
			{
				_buffer += key;
				_cursor++;
			}
	}

	return true;
}

void	menu_framework_c::addItem(menu_common_c *item)
{
	if(_items.size() == 0)
		_nslots = 0;

	if(_items.size() < MAXMENUITEMS)
	{		
		item->_parent = this;
		
		_items.push_back(item);
	}

	_nslots = getTallySlotsNum();
}

/*
** Menu_AdjustCursor
**
** This function takes the given menu, the direction, and attempts
** to adjust the menu's cursor so that it's at the next available
** slot.
*/
void	menu_framework_c::adjustCursor(int dir)
{
	menu_common_c *citem;

	//
	// see if it's in a valid spot
	//
	if(_cursor >= 0 && _cursor < (int)_items.size())
	{
		if((citem = getItemAtCursor()) != 0 )
		{
			if(citem->getType() != MTYPE_SEPARATOR)
				return;
		}
	}

	//
	// it's not in a valid spot, so crawl in the direction indicated until we
	// find a valid spot
	//
	/*
	if(dir == 1)
	{
		while(true)
		{
			citem = getItemAtCursor();
			if(citem)
				if(citem->getType() != MTYPE_SEPARATOR)
					break;
					
			_cursor += dir;
			if(_cursor >= (int)_items.size())
				_cursor = (int)_items.size() -1;
		}
	}
	else
	*/
	{
		while(true)
		{
			citem = getItemAtCursor();
			if(citem)
				if(citem->getType() != MTYPE_SEPARATOR)
					break;
					
			_cursor += dir;
			
			//if(_cursor < 0)
			//	_cursor = _items.size() - 1;
			X_clamp(_cursor, 0, (int)_items.size()-1);
		}
	}
}

void	menu_framework_c::center()
{
	/*
	unsigned int width = 0;
	
	for(std::vector<menu_common_c*>::const_iterator ir = _items.begin(); ir != _items.end(); ir++)
	{
		int charwidth;
	
		if((*ir)->_fontflags & FONT_MEDIUM)
		{
			charwidth = CHAR_MEDIUM_WIDTH;
		}
		else if((*ir)->_fontflags & FONT_BIG)
		{
			charwidth = CHAR_BIG_WIDTH;
		}
		else
		{
			charwidth = CHAR_SMALL_WIDTH;
		}
	
		if(width > (*ir)->_name.length() * charwidth)
			width = (*ir)->_name.length() * charwidth;
	}
			
	_x = (trap_VID_GetWidth() - width) / 2;
	*/

	int height;

	height = _items[_items.size()-1]->_y;
	height += 10;

	_y = (trap_VID_GetHeight() - height) / 2;
}

#if 0
static void Field_Init( menufield_s *f )
{
	f->generic.mins[0] = f->generic.x + f->generic.parent->x + 16;
	f->generic.mins[1] = f->generic.y + f->generic.parent->y;

	f->generic.maxs[0] = f->generic.mins[0] + (f->buffer.length()* CHAR_SMALL_WIDTH);
	f->generic.maxs[1] = f->generic.mins[1] + CHAR_SMALL_HEIGHT;
}

static void	Action_Init( menuaction_s *a )
{
	int fontstyle;

	if ( a->generic.flags & QMF_GIANT )
		fontstyle = FONT_GIANT;
	else
		fontstyle = FONT_SMALL;

	a->generic.mins[0] = a->generic.x + a->generic.parent->x;
	
	if ( a->generic.flags & QMF_CENTERED )
	{
		if(fontstyle == FONT_SMALL)
			a->generic.mins[0] -= ((CHAR_SMALL_WIDTH * a->generic.name.length()) / 2);
		else
			a->generic.mins[0] -= ((CHAR_GIANT_WIDTH * a->generic.name.length()) / 2);
	}
	else if ( !(a->generic.flags & QMF_LEFT_JUSTIFY) )
	{
		if(fontstyle == FONT_SMALL)
			a->generic.mins[0] -= (CHAR_SMALL_WIDTH * a->generic.name.length());
		else
			a->generic.mins[0] -= (CHAR_GIANT_WIDTH * a->generic.name.length());
	}
	a->generic.mins[1] = a->generic.y + a->generic.parent->y;

	if(fontstyle == FONT_SMALL)
			a->generic.maxs[0] = a->generic.mins[0] + (CHAR_SMALL_WIDTH * a->generic.name.length());
		else
			a->generic.maxs[0] = a->generic.mins[0] + (CHAR_GIANT_WIDTH * a->generic.name.length());

	
	a->generic.maxs[1] = a->generic.mins[1] + ((fontstyle == FONT_SMALL) ? CHAR_SMALL_HEIGHT : CHAR_GIANT_HEIGHT);
}

static void	MenuList_Init( menulist_s *l )
{
	std::vector<std::string>	n;
	int fontstyle;
	int xsize, ysize, len, spacing;

	n = l->itemnames;

	if ( !n.size() )
		return;

	fontstyle = FONT_SMALL;

	if ( fontstyle & FONT_SMALL )
	{
		spacing = CHAR_SMALL_HEIGHT + 2;
		l->generic.mins[0] = l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET - (l->generic.name.length() * CHAR_SMALL_WIDTH);	
	}
	else
	{
		spacing = CHAR_BIG_HEIGHT + 2;
		l->generic.mins[0] = l->generic.x + l->generic.parent->x + LCOLUMN_OFFSET - (l->generic.name.length() * CHAR_BIG_WIDTH);	
	}

	l->generic.mins[1] = l->generic.y + l->generic.parent->y;

	ysize = 0;
	xsize = 0;

	for(unsigned int i=0; i<n.size(); i++)
	{
		if(fontstyle == FONT_SMALL)
			len = (CHAR_SMALL_WIDTH * n[i].length());
		else
			len = (CHAR_GIANT_WIDTH * n[i].length());
			
		xsize = max ( xsize, len );
		ysize += spacing;
	}

	l->generic.maxs[0] = l->generic.mins[0] + xsize;
	l->generic.maxs[1] = l->generic.mins[1] + ysize;
}

static void Separator_Init( menuseparator_s *s )
{

}

static void Slider_Init( menuslider_s *s )
{
	//TODO recode
	 
	/*
	int delta;
	int fontstyle;

	fontstyle = FONT_SMALL;
	delta = trap_PropStringLength ( ( char * )s->generic.name, fontstyle );

	s->generic.mins[0] = s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET;
	if ( s->generic.flags & QMF_CENTERED ) {
		s->generic.mins[0] += delta / 2;
	}

	s->generic.mins[1] = s->generic.y + s->generic.parent->y;

	s->generic.maxs[0] = s->generic.mins[0] + SLIDER_RANGE * SMALL_CHAR_WIDTH + SMALL_CHAR_WIDTH;
	s->generic.maxs[1] = s->generic.mins[1] + SMALL_CHAR_HEIGHT;
	*/
}

static void SpinControl_Init( menulist_s *s )
{
	//TODO recode
	
	/*
	char buffer[100] = { 0 };
	int fontstyle;
	int ysize, xsize, spacing, len;
	const char **n;

	n = s->itemnames;

	if ( !n )
		return;

	fontstyle = FONT_SMALL;

	s->generic.mins[0] = s->generic.x + s->generic.parent->x + RCOLUMN_OFFSET;
	s->generic.mins[1] = s->generic.y + s->generic.parent->y;

	if ( fontstyle == FONT_SMALL ) {
		ysize = PROP_SMALL_HEIGHT;
	} else {
		ysize = PROP_BIG_HEIGHT;
	}
	spacing = ysize + 2;

	xsize = 0;

	while ( *n )
	{
		if ( !strchr( *n, '\n' ) )
		{
			len = trap_PropStringLength ( ( char * )*n, fontstyle );
			xsize = max ( xsize, len );
		}
		else
		{

			strcpy( buffer, *n );
			*strchr( buffer, '\n' ) = 0;
			len = trap_PropStringLength ( buffer, fontstyle );
			xsize = max ( xsize, len );

			ysize = ysize + spacing;
			strcpy( buffer, strchr( *n, '\n' ) + 1 );
			len = trap_PropStringLength ( buffer, fontstyle );
			xsize = max ( xsize, len );
		}

		*n++;
	}

	if ( s->generic.flags & QMF_CENTERED ) {
		s->generic.mins[0] -= xsize / 2;
	}

	s->generic.maxs[0] = s->generic.mins[0] + xsize;
	s->generic.maxs[1] = s->generic.mins[1] + ysize;
	*/
}

void Menu_AdjustRectangle ( int *mins, int *maxs )
{
	//TODO recode
	
	/*
	mins[0] *= uis.scaleX;
	maxs[0] *= uis.scaleX;

	mins[1] *= uis.scaleY;
	maxs[1] *= uis.scaleY;
	*/
}


/*
void	Menu_Init(menuframework_s *menu)
{
	//
	// init items
	//
	for (int i = 0; i < menu->nitems; i++ )
	{
		switch ( ( ( menucommon_s * ) menu->items[i] )->type )
		{
		case MTYPE_FIELD:
			Field_Init( ( menufield_s * ) menu->items[i] );
			break;
		case MTYPE_SLIDER:
			Slider_Init( ( menuslider_s * ) menu->items[i] );
			break;
		case MTYPE_LIST:
			MenuList_Init( ( menulist_s * ) menu->items[i] );
			break;
		case MTYPE_SPINCONTROL:
			SpinControl_Init( ( menulist_s * ) menu->items[i] );
			break;
		case MTYPE_ACTION:
			Action_Init( ( menuaction_s * ) menu->items[i] );
			break;
		case MTYPE_SEPARATOR:
			Separator_Init( ( menuseparator_s * ) menu->items[i] );
			break;
		}
		
		Menu_AdjustRectangle ( ( ( menucommon_s * ) menu->items[i] )->mins, ( ( menucommon_s * ) menu->items[i] )->maxs );
	}
}
*/

#endif

void	menu_framework_c::draw()
{
	menu_common_c *item;
	

	//
	// draw contents
	//
	for(std::vector<menu_common_c*>::const_iterator ir = _items.begin(); ir != _items.end(); ir++)
	{
		(*ir)->draw();
	}


	//
	// draw cursor
	//
	item = getItemAtCursor();

	if(item && item->_cursordraw)
	{
		item->_cursordraw(item);
	}
	else if(_cursordraw)
	{
		_cursordraw(this);
	}
	else if(item && item->getType() != MTYPE_FIELD )
	{
		/*
		if(item->_flags & QMF_LEFT_JUSTIFY)
		{
			Menu_DrawChar(_x + item->_x - 24 + item->_cursor_offset, _y + item->_y, 12 + ((int)(trap_Sys_Milliseconds()/250) & 1), color_white, FONT_NONE);
		}
		else
		*/
		{
			Menu_DrawChar(_x + item->_cursor_offset, _y + item->_y, 12 + ((int)(trap_Sys_Milliseconds()/250) & 1), color_white, FONT_NONE);
		}
	}


	//
	// draw item statusbar or framework statusbar
	//
	if(item)
	{
		if(item->_statusbarfunc)
			item->_statusbarfunc((void*)item);
			
		else if (item->_statusbar.length())
			drawStatusBar(item->_statusbar);
		else
			drawStatusBar(_statusbar);

	}
	else
	{
		drawStatusBar(_statusbar);
	}
}

void	menu_framework_c::drawStatusBar(const std::string &string)
{
	if(string.length())
	{
		int l = string.length();
		int maxcol = trap_VID_GetWidth() / 8;
		int col = maxcol / 2 - l / 2;
		
		vec4_c color(0.5, 0.5, 0.5, 1);

		trap_R_DrawFill(0, trap_VID_GetHeight()-8, trap_VID_GetWidth(), 8, color);
		Menu_DrawString(col*8, trap_VID_GetHeight() - 8, string, FONT_SMALL);
	}
	else
	{
		vec4_c color;
		trap_R_DrawFill(0, trap_VID_GetHeight()-8, trap_VID_GetWidth(), 8, color);
	}
}

int	Menu_GetCharWidth(int flags)
{
	if(flags & FONT_MEDIUM)
	{
		return CHAR_MEDIUM_WIDTH;
	}
	else if(flags & FONT_BIG)
	{
		return CHAR_BIG_WIDTH;
	}
	else
	{
		return CHAR_SMALL_WIDTH;
	}

}

/*
================
CG_DrawChar

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void	Menu_DrawChar(int x, int y, int num, const vec4_c &color, int flags)
{
	int			row, col;
	int			width, height;
	float			frow, fcol, size;
	int			shader;

		
	num &= 255;
	
	if((num&127) == 32)
		return;		// space
		
	//int cwidth = MenuGetCharWidth(flags);
	//width = height = cwidth;

	if(flags & FONT_MEDIUM)
	{
		width = CHAR_MEDIUM_WIDTH;
		height = CHAR_MEDIUM_HEIGHT;
	}
	else if(flags & FONT_BIG)
	{
		width = CHAR_BIG_WIDTH;
		height = CHAR_BIG_HEIGHT;
	}
	else
	{
		width = CHAR_SMALL_WIDTH;
		height = CHAR_SMALL_HEIGHT;
	}


	if(y <= -height)
		return;			// totally off screen
		
	if(flags & FONT_ALT)
		num = num ^ 0x80;

	row = num>>4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;
	
	
	if(flags & FONT_CHROME)
		shader = trap_R_RegisterPic("fonts/chromechars");
	else
		shader = trap_R_RegisterPic("fonts/conchars");

	
	if(flags & FONT_SHADOWED)
		trap_R_DrawStretchPic(x+2, y+2, width, height, fcol, frow, fcol+size, frow+size, color, shader);
	else
		trap_R_DrawStretchPic(x, y, width, height, fcol, frow, fcol+size, frow+size, color, shader);
}


void	Menu_DrawString(int x, int y, const std::string &string, int flags)
{
	unsigned i;
	
	int width = Menu_GetCharWidth(flags);
	
	for(i=0; i<string.length(); i++)
	{
		Menu_DrawChar((x + i*width), y, string[i], color_white, flags);
	}
}

/*
void	Menu_DrawStringDark(int x, int y, const std::string &string)
{
	unsigned i;

	for(i=0; i<string.length(); i++)
	{
		Menu_DrawChar((x + i*8), y, string[i], color_white, FONT_CHROME | FONT_ALT);
	}
}

void	Menu_DrawStringR2L(int x, int y, const std::string &string)
{
	unsigned i;

	for(i=0; i<string.length(); i++)
	{
		Menu_DrawChar((x - i*8), y, string[string.length()-i-1], color_white, FONT_CHROME);
	}
}

void	Menu_DrawStringR2LDark(int x, int y, const std::string &string)
{
	unsigned i;

	for(i=0; i<string.length(); i++)
	{
		Menu_DrawChar((x - i*8), y, string[string.length()-i-1], color_white, FONT_CHROME | FONT_ALT);
	}
}
*/

menu_common_c*	menu_framework_c::getItemAtCursor()
{
	if(_cursor < 0 || _cursor > (int)_items.size()-1)
		return NULL;

	return _items[_cursor];
}

void	menu_framework_c::selectItem()
{
	menu_common_c *item = getItemAtCursor();

	if(item)
	{
		item->doEnter();
	}
}

void	menu_framework_c::setStatusBar(const std::string &string)
{
	_statusbar = string;
}

void	menu_framework_c::slideItem(int dir)
{
	menu_common_c *item = getItemAtCursor();

	if(item)
	{
		item->doSlide(dir);
	}
}

int	menu_framework_c::getTallySlotsNum()
{
	int total = 0;

	for(unsigned int i=0; i<_items.size(); i++)
	{
		if(((menu_common_c*)_items[i])->getType() == MTYPE_LIST)
		{
			int nitems = 0;
			std::vector<std::string> n = ((menu_list_c*)_items[i])->_itemnames;

			for(unsigned int j=0; j<n.size(); j++)
				nitems++;

			total += nitems;
		}
		else
		{
			total++;
		}
	}

	return total;
}

/*
================================================================================
				MENU LIST ITEM
================================================================================
*/
void 	menu_list_c::draw()
{
	int y = 0;

	Menu_DrawString(_x + _parent->_x, _y + _parent->_y, _name, _fontflags | FONT_ALT);

  	trap_R_DrawFill(_x - 112 + _parent->_x, _y + _parent->_y + _curvalue*10 + 10, 128, 10, color_grey_med);
	
	for(unsigned int i=0; i<_itemnames.size(); i++)
	{
		Menu_DrawString(_x + _parent->_x, _y + _parent->_y + y + 10, _itemnames[i], _fontflags | FONT_ALT);
		y += 10;
	}
}

/*
================================================================================
				MENU SEPARATOR ITEM
================================================================================
*/
void	menu_separator_c::draw()
{
	if(_name.length())
		Menu_DrawString(_x + _parent->_x, _y + _parent->_y, _name, _fontflags);
}


/*
================================================================================
				MENU SLIDER ITEM
================================================================================
*/
void	menu_slider_c::doSlide(int dir)
{
	_curvalue += dir;

	X_clamp(_curvalue, _minvalue, _maxvalue);

	callback();
}

#define SLIDER_RANGE 10

void	menu_slider_c::draw()
{
	Menu_DrawString(_x + _parent->_x, _y + _parent->_y, _name, _fontflags);

	float range = (_curvalue - _minvalue) / (float)(_maxvalue - _minvalue);

	X_clamp(range, 0, 1);
	
#if 0
	int	i;
	
	int cwidth = Menu_GetCharWidth(_fontflags);
		
	//Menu_DrawChar(_name.length()*CHAR_MEDIUM_WIDTH + _x + _parent->_x + RCOLUMN_OFFSET, _y + _parent->_y, 128, color_white, FONT_NONE);
	
	for(i=0; i<SLIDER_RANGE; i++)
		Menu_DrawChar(_name.length()*cwidth + _x + i*CHAR_MEDIUM_WIDTH + _parent->_x + CHAR_MEDIUM_WIDTH, _y + _parent->_y, 129, color_white, FONT_NONE);
		
	Menu_DrawChar(_name.length()*cwidth + _x + i*CHAR_MEDIUM_WIDTH + _parent->_x + CHAR_MEDIUM_WIDTH, _y + _parent->_y, 130, color_white, FONT_NONE);
	Menu_DrawChar((int)(8 + _x + _parent->_x + (SLIDER_RANGE-1)*CHAR_MEDIUM_WIDTH * _range), _y + _parent->_y, 131, color_white, FONT_NONE);
#else

	int cwidth = Menu_GetCharWidth(_fontflags);
	trap_R_DrawFill((_name.length()+1)*cwidth +_x + _parent->_x, _y + _parent->_y, cwidth * SLIDER_RANGE, cwidth, color_grey_med);
	trap_R_DrawFill((_name.length()+1)*cwidth +_x + _parent->_x, _y + _parent->_y,(int)(cwidth * SLIDER_RANGE * range), cwidth, color_blue);
#endif
}


/*
================================================================================
				MENU SPINCONTROL ITEM
================================================================================
*/
void	menu_spincontrol_c::doSlide(int dir)
{
	_curvalue += dir;
	
	X_clamp(_curvalue, 0, (int)_itemnames.size()-1);
	
	callback();
}

void	menu_spincontrol_c::doEnter()
{
	callback();
}

void	menu_spincontrol_c::draw()
{
	std::string	buffer;

	if(_name.length())
	{
		Menu_DrawString(_x + _parent->_x, _y + _parent->_y, _name, _fontflags | FONT_ALT | FONT_CHROME);
	}
	
	int cwidth = Menu_GetCharWidth(_fontflags);
	
	Menu_DrawString(_name.length()*(cwidth+1) + _x + _parent->_x, _y + _parent->_y, _itemnames[_curvalue], _fontflags);
}
