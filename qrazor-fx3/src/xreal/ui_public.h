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
#ifndef UI_PUBLIC_H
#define UI_PUBLIC_H

/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "../r_public.h"

// xreal --------------------------------------------------------------------



#define UI_API_VERSION		6


#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	void 		(*Com_Printf)(const char *fmt, ...);
	void 		(*Com_DPrintf)(const char *fmt, ...);
	void 		(*Com_Error)(err_type_e type, const char *fmt, ...);
	int		(*Com_ServerState) (void);
	
	void 		(*Cbuf_AddText)(const std::string &text);
	void 		(*Cbuf_InsertText)(const std::string &text);
	void 		(*Cbuf_ExecuteText)(exec_type_e, const std::string &text);
	void 		(*Cbuf_Execute)();
	
	void		(*Cmd_AddCommand)(const std::string &name, void(*cmd)());
	void		(*Cmd_RemoveCommand)(const std::string &name);
	int		(*Cmd_Argc)();
	const char*	(*Cmd_Argv)(int i);	

	cvar_t*		(*Cvar_Get)(const std::string &name, const std::string &value, uint_t flags);
	cvar_t*		(*Cvar_Set)(const std::string &name, const std::string &value);
	cvar_t*		(*Cvar_ForceSet)(const std::string &name, const std::string &value);
	void		(*Cvar_SetValue)(const std::string &name, float value);
	void		(*Cvar_SetModified)(const std::string &name);
	float		(*Cvar_VariableValue)(const std::string &name);
	int		(*Cvar_VariableInteger)(const std::string &name);
	const char*	(*Cvar_VariableString)(const std::string &name);
	float		(*Cvar_ClampVariable)(const std::string &name, float min, float max);
	
	std::string	(*VFS_Gamedir)();
	std::vector<std::string>	(*VFS_ListFiles)(const std::string &dir, const std::string &extension);
	
	int		(*VFS_FLoad)(const std::string &name, void **buf);
	void		(*VFS_FFree)(void *buf);
		
//	void		(*R_BeginRegistration)(const std::string &map);
//	int		(*R_RegisterModel)(const std::string &name);
//	int		(*R_RegisterAnimation)(const std::string &name);
//	int		(*R_RegisterSkin)(const std::string &name);
//	int		(*R_RegisterShader)(const std::string &name);
	int		(*R_RegisterPic)(const std::string &name);
//	int		(*R_RegisterLight)(const std::string &name);
//	void		(*R_EndRegistration)();

//	void		(*R_BeginFrame)();
//	void		(*R_RenderFrame)(const r_refdef_t &fd);
//	void		(*R_EndFrame)();
	
	void		(*R_DrawPic)(int x, int y, int w, int h, const vec4_c &color, int shader);
	void		(*R_DrawStretchPic)(int x, int y, int w, int h, float s1, float t1, float s2, float t2, const vec4_c &color, int shader);
	void		(*R_DrawFill)(int x, int y, int w, int h, const vec4_c &color);
	
	/*
	void		(*R_ClearScene)();
	
	void		(*R_AddEntity)(const r_entity_t &shared);
	void		(*R_UpdateEntity)(int entity_num, const r_entity_t &shared);
	void		(*R_RemoveEntity)(int entity_num);
	
	void		(*R_AddLight)(int entity_num, const r_entity_t &shared);
	void		(*R_UpdateLight)(int entity_num, const r_entity_t &shared);
	void		(*R_RemoveLight)(int entity_num);
	
	void		(*R_AddParticle)(const r_particle_t &part);
	void		(*R_AddPoly)(const r_poly_t &poly);
	
	bool		(*R_SetupTag)(r_tag_t &tag, const r_entity_t &ent, const std::string &name);
	bool		(*R_SetupAnimation)(int model, int anim);
	*/
		
	char*		(*Key_KeynumToString)(int keynum);
	char*		(*Key_GetBinding)(int keynum);
	void 		(*Key_SetBinding)(int keynum, char *binding);
	void 		(*Key_ClearStates)();
	keydest_t	(*Key_GetKeyDest)();
	void		(*Key_SetKeyDest)(keydest_t key_dest);
	
	void		(*Con_ClearNotify)();
	void		(*Con_ClearTyping)();

	float		(*CL_GetRealtime)();
	bool		(*CL_GetAttractloop)();
	void 		(*CL_Quit_f)();
	void 		(*CL_PingServers_f)();
	void		(*CL_SetServercount)(int count);
	
	void		(*S_StartLocalSound)(const std::string &name);
	
	char*		(*Sys_AdrToString)(const netadr_t &a);
	int		(*Sys_Milliseconds)();
	
	viddef_t*	viddef;

} ui_import_t;


typedef struct
{
	int		apiversion;
	
	void		(*Init)();
	void		(*Shutdown)();
	
	void		(*M_Keydown)(int key);
	void		(*M_Draw)();
	void		(*M_Menu_Main_f)();
	void		(*M_ForceMenuOff)();
	void		(*M_AddToServerList)(const netadr_t &adr, const char *info);

	//TODO

} ui_export_t;



ui_export_t*	GetUIAPI (ui_import_t *import);



#ifdef __cplusplus
}
#endif


#endif	// __XUI_PUBLIC__
