/// ============================================================================
/*
Copyright (C) 2004 Didier Cassirame (Tenebrae Team)
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
// qrazor-fx ----------------------------------------------------------------
// xreal --------------------------------------------------------------------
#include "gui_local.h"

//std::map<int, const char*>	xmlorient = {{0, "horizontal"}, {1, "vertical"}};

const char* xmlalign[] =
{
	"start",
	"center",
	"end",
	"baseline",
	"stretch",
	NULL
};

const char* xmlpack[] =
{
	"start",
	"center",
	"end",
	NULL
};

const char* xmlorient[] = 
{
	"horizontal",
	"vertical",
	NULL
};

const char* xmlbool[] =
{
	"false",
	"true",
	NULL
};



gui_widget_c::gui_widget_c()
{

	_font		= -1;
	
	_focus_shader	= -1;
	_focus_type	= 0;
	
	_parent		= NULL;
	_previous	= NULL;
	_next		= NULL;
	
	_debug		= true;
	_enabled	= true;
	_focusable	= true;
	_orient		= true;
	
	_align		= GUI_ALIGN_LEFT;
	_pack		= GUI_PACK_LEFT;
	_xpos		= 0;
	_ypos		= 0;
	_xoffset	= 0;
	_yoffset	= 0;
//	_width		= 0;
//	_height		= 0;
	_accesskey	= 0;
	
//	_onCommand;
//	_onMouseOver;
//	_onMouseDown;
//	_onMouseUp;
}

gui_widget_c::~gui_widget_c()
{
	//TODO
}


int	gui_widget_c::readXmlPropAsChar(xmlNodePtr node, const std::string &name, int defvalue)
{
	int ret;
	
	xmlChar *value = xmlGetProp(node, (const xmlChar*)name.c_str());
	
	if(value)
	{
		ret = (int)value[0];
		xmlFree(value);
	}else
	{
		ret = defvalue;
	}
	
	return ret;
}

int	gui_widget_c::readXmlPropAsInt(xmlNodePtr node, const std::string &name, int defvalue)
{
	int ret;
	
	xmlChar *value = xmlGetProp(node, (const xmlChar*)name.c_str());
	
	if(value)
	{
		ret = atoi((const char*)value);
		xmlFree(value);
	}
	else
	{
		ret = defvalue;
	}
	
	return ret;
}

float	gui_widget_c::readXmlPropAsFloat(xmlNodePtr node, const std::string &name, float defvalue)
{
	float ret;
	
	xmlChar *value = xmlGetProp(node, (const xmlChar*)name.c_str());
	
	if(value)
	{
		ret = atof((const char*)value);
		xmlFree(value);
	}
	else
	{
		ret = defvalue;
	}
	
	return ret;
}

const char*	gui_widget_c::readXmlPropAsString(xmlNodePtr node, const std::string &name)
{
	std::string ret;

	xmlChar *value = xmlGetProp(node, (const xmlChar*)name.c_str());
	
	if(value)
	{
		ret = (const char*)value;
		xmlFree(value);
	}
	else
	{
		ret = "";
	}

	return ret.c_str();
}

/*
char*	gui_widget_c::readXmlPropAsRefString(xmlNodePtr node, const std::string &name)
{
	char *ret;

	xmlChar *value = xmlGetProp(node, (const xmlChar*)name.c_str());
	
	if(value)
	{
		ret = createRefString(value, strlen(value));
		xmlFree(value);
	}else
	{
		ret = "";
	}

	return ret;
}
*/

void	gui_widget_c::readXmlDim(xmlNodePtr node, const std::string &name, gui_dim_t &dim)
{
	char *p;
	
	xmlChar *value = xmlGetProp(node, (const xmlChar*)name.c_str());
	
	if(value)
	{
		p = strchr((const char*)value, '%');
		if(p)
		{
			*p = '\0';
			dim.ratio = atof((const char*)value) / 100;
			*p = '%';
			dim.absolute = -1;	
		} 
		else
		{
			dim.absolute = atoi((const char*)value);
			dim.ratio = -1;
		}
	}
	else
	{
		dim.absolute = -1;
		dim.ratio = -1;
	}
}

int	gui_widget_c::compareXmlProp(xmlNodePtr node, const std::string &name, const char **strings, int count)
{
	int ret;
	
	xmlChar *value = xmlGetProp(node, (const xmlChar*)name.c_str());
	if(!value)
		return -1;
		
	for(ret = 0; ret < count; ret++)
	{
		if(strcmp((char*)value, (char*)strings[ret]) == 0)
		{
			xmlFree(value);
			return ret;
		}
	}
	xmlFree(value);
	return -1;
}

void	gui_widget_c::prepareWindow()
{
	//TODO
}

void	gui_widget_c::loadXmlElement(xmlNodePtr root)
{
	/*
		These are attributes shared by all widgets.

	 * --- supported attributes :

	   align       =  {start|center|end|baseline|stretch}
	   pack        =  {start|center|end}
	   debug       =  bool  : draw a border around it and all children ?
	   height      =  int 
	   width       =  int 
	   id          =  string : the name/command of the object
	   orient      =  {horizontal|vertical}

	 * --- unsupported :

	   allowevents
	   allownegativeassertions
	   class
	   coalesceduplicatearcs
	   collapsed
	   container
	   containment
	   context
	   contextmenu
	   datasources
	   dir
	   empty
	   equalsize   =  {always|never} : This attribute can be used to make the children of the element equal in size.
	   flags       =  {dont-test-empty|dont-build-content}
	   flex
	   flexgroup
	   hidden    
	   insertafter
	   insertbefore
	   left
	   maxheight
	   maxwidth
	   menu
	   minheight
	   minwidth
	   observes
	   ordinal
	   pack       =  {start|center|end}
	   persist
	   popup
	   position
	   ref
	   removeelement
	   statustext
	   style
	   template
	   tooltip
	   tooltiptext
	   top
	   uri
	*/

	// read general attributes from the file 
	readXmlDim(root, "width", _width);
	readXmlDim(root, "height", _height);
	_debug = (readXmlPropAsInt(root, "debug", 0) != 0);

	// TODO
//	self->id = readXmlPropAsRefString(root, "id");
//	self->name = readXmlPropAsRefString(root, "name");

	// check for a font
	_font = -1;
	const char* str = readXmlPropAsString(root, "font");
	if(str[0] != '\0')
		_font = trap_R_RegisterPic(str);

	// check for a focus shader
//	buffer[0] = 0;
//	self->focusShader = NULL;
//	M_ReadXmlPropAsString (root, "focusshader", buffer, sizeof(buffer));
//	if (buffer[0])
//		self->focusShader = GL_ShaderForName(buffer);

	// check for a focus type
//	buffer[0] = 0;
//	self->focusType = FOCUS_NOTSPECIFIED;
//	M_ReadXmlPropAsString (root, "focustype", buffer, sizeof(buffer));
//	if (!strcmp(buffer, "border")) {
//		self->focusType = FOCUS_BORDER;
//	} else if (!strcmp(buffer,"rectangle")) {
//		self->focusType = FOCUS_OVER;
//	}
		
	int tmp = compareXmlProp(root, "orient", xmlorient, 2);
	if(tmp != -1)
		_orient = tmp;
	
	tmp = compareXmlProp(root, "align", xmlalign, 5);
	if(tmp != -1)
		_align = (gui_align_e)tmp;
		
	tmp = compareXmlProp(root, "pack", xmlpack, 3);
	if(tmp != -1)
		_pack = (gui_pack_e)tmp;

	// then init the private data
	load(root);
}

void	gui_widget_c::insertWidget(gui_widget_c *child)
{
	child->_parent = this;
	
	_children.push_back(child);
}


const char gui_tag_box[]	= "box";
const char gui_tag_hbox[]	= "hbox";
const char gui_tag_vbox[]	= "vbox";
const char gui_tag_label[]	= "label";
const char gui_tag_image[]	= "image";
const char gui_tag_button[]	= "button";
const char gui_tag_menuitem[]	= "menuitem";
const char gui_tag_checkbox[]	= "checkbox";
const char gui_tag_radio[]	= "radio";
const char gui_tag_rgroup[]	= "radiogroup";
const char gui_tag_slider[]	= "slider";
const char gui_tag_edit[]	= "edit";
const char gui_tag_window[]	= "window";
const char gui_tag_package[]	= "package";


/*
	This actually traverses the xml parse tree and creates windows, widgets, ... from it
*/
static gui_widget_c*	GUI_LoadXmlElementTree(xmlNodePtr root)
{
	xmlNodePtr	node;
	gui_widget_c	*ret, *sub;

	// we don't want anonymous tags
	if(!root->name)
		return NULL;
		
	// first read/initialize specifics attributes 
	if(!xmlStrcmp(root->name, (const xmlChar*)gui_tag_window))
	{
		// everything ok
		ret = new gui_window_c();
	}
	else
	{
		return NULL;
	}

	/*
	if(handler->datasize)
		ret->data = M_AllocateMem(handler->datasize, NULL);
		
	ret->tag = handler->tag;
	*/

	// new window ?
	/*
	if(ret->tag == window_tag)
		loading = ret->data;
	*/

	//TODO
	ret->loadXmlElement(root);

	/* load all the subnodes */
	node = root->xmlChildrenNode;
	while(node != NULL)
	{
		// comment nodes have node->name == NULL
		if(!xmlIsBlankNode(node) && (node->type != XML_COMMENT_NODE))
		{
			sub = GUI_LoadXmlElementTree(node);
			if(sub)
				ret->insertWidget(sub);
		}
		node = node->next;
	}
	return ret;
}

static void 	GUI_XMLError(void *ctx, const char *fmt, ...)
{
	va_list		argptr;
	char		text[MAX_STRING_CHARS];

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	trap_Com_Error(ERR_FATAL, "%s", text);
}


void	GUI_LoadXGUIFile(const std::string &filename)
{
	char*		buffer;
	xmlDocPtr	doc;
	xmlNodePtr	node;
	int len;

	// setup error funciton pointers
	xmlInitParser();
	xmlSetGenericErrorFunc(NULL, GUI_XMLError);

	trap_Com_DPrintf("loading '%s' ...\n", filename.c_str());

	len = trap_VFS_FLoad(filename, (void**)&buffer);
	if(!buffer)
	{
		trap_Com_Error(ERR_FATAL, "GUI_LoadXGUIFile: could not load '%s'", filename.c_str());
		return;
	}
	
	doc = xmlParseMemory(buffer, len);
	if(doc == NULL)
	{
		trap_Com_Error(ERR_FATAL, "GUI_LoadXGUIFile: xmlParseMemory returned NULL for '%s'", filename.c_str());
		return;
	}
	node = xmlDocGetRootElement(doc);
	if(node == NULL)
	{
		trap_Com_Error(ERR_FATAL, "GUI_LoadXGUIFile: empty document '%s'", filename.c_str());
		xmlFreeDoc (doc);
		return;
	}
	
	if(!xmlIsBlankNode(node) && (node->name))
	{
		if(!xmlStrcmp(node->name, (const xmlChar*)"package"))
		{
			// parse all the menus in this package
			node = node->xmlChildrenNode;
			while(node != NULL)
			{
				// comment nodes have node->name == NULL  
				if(!xmlIsBlankNode(node) && (node->type != XML_COMMENT_NODE))
				{
					gui_widget_c *widget = GUI_LoadXmlElementTree(node);
					
					if(widget)
						widget->prepareWindow();
				}
				node = node->next;
			}
		}
		else
		{
			trap_Com_DPrintf("GUI_LoadXGUIFile: Warning: '%s' document root isn't a package\n", filename.c_str());
		}
	}

	xmlFreeDoc (doc);

	trap_Com_DPrintf("XML : loading '%s' document ended\n", filename.c_str());
}

static void	GUI_OpenWindow_f()
{
	//TODO
}

static void	GUI_CloseWindow_f()
{
	//TODO
}

static void	GUI_WindowList_f()
{
	//TODO
}

void	GUI_Init()
{
	trap_Com_Printf("------- GUI_Init -------\n");

	trap_Cmd_AddCommand("openwindow",	GUI_OpenWindow_f);
	trap_Cmd_AddCommand("closewindow",	GUI_CloseWindow_f);
	trap_Cmd_AddCommand("windowlist",	GUI_WindowList_f);
}

void	GUI_Shutdown()
{
	trap_Com_Printf("------- GUI_Shutdown -------\n");

	//TODO
}


