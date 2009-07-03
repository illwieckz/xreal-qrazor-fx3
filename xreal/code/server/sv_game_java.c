/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#if defined(USE_JAVA)


#include "server.h"
#include "../qcommon/vm_java.h"
#include "java/xreal_server_Server.h"
#include "java/xreal_server_game_GameEntity.h"


typedef struct gclient_s
{
	playerState_t   ps;

	jobject         object_Player;

	jstring         playerInfo;

	jmethodID		method_Player_clientBegin;
	jmethodID		method_Player_clientUserInfoChanged;
	jmethodID		method_Player_clientDisconnect;
	jmethodID		method_Player_clientCommand;
	jmethodID		method_Player_clientThink;
} gclient_t;

typedef struct gentity_s
{
	entityState_t   s;			// communicated by server to clients
	entityShared_t  r;			// shared by both the server system and game
	gclient_t      *client;		// NULL if not a client

	qboolean        inUse;
	qboolean        neverFree;	// if true, FreeEntity will only unlink

	int				spawnTime;	// level.time when the object was spawned
	int             freeTime;	// level.time when the object was freed
} gentity_t;

static gentity_t		g_entities[MAX_GENTITIES];
static gclient_t		g_clients[MAX_CLIENTS];
static int				g_levelStartTime; // for relaxing entity free policies
static int				g_levelTime;


int SV_NumForGentity(sharedEntity_t * ent)
{
	int             num;

	num = ((byte *) ent - (byte *) g_entities) / sizeof(gentity_t);
//	num = ent - sv.gentities;
//	num = ent - g_entities;

	return num;
}

sharedEntity_t *SV_GentityNum(int num)
{
	sharedEntity_t *ent;

	ent = (sharedEntity_t *) ((byte *) g_entities + sizeof(gentity_t) * (num));
//	ent = &sv.gentities[num];	// &g_entities[num];

	return ent;
}

playerState_t  *SV_GameClientNum(int num)
{
	playerState_t  *ps;

//	ps = (playerState_t *) ((byte *) sv.gameClients + sv.gameClientSize * (num));
	ps = (playerState_t *) &g_clients[num].ps;

	return ps;
}

svEntity_t     *SV_SvEntityForGentity(sharedEntity_t * gEnt)
{
	if(!gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES)
	{
		Com_Error(ERR_DROP, "SV_SvEntityForGentity: bad gEnt");
	}

	return &sv.svEntities[gEnt->s.number];
}

sharedEntity_t *SV_GEntityForSvEntity(svEntity_t * svEnt)
{
	int             num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum(num);
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand(int clientNum, const char *text)
{
	if(clientNum == -1)
	{
		SV_SendServerCommand(NULL, "%s", text);
	}
	else
	{
		if(clientNum < 0 || clientNum >= sv_maxclients->integer)
		{
			return;
		}
		SV_SendServerCommand(svs.clients + clientNum, "%s", text);
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient(int clientNum, const char *reason)
{
	if(clientNum < 0 || clientNum >= sv_maxclients->integer)
	{
		return;
	}
	SV_DropClient(svs.clients + clientNum, reason);
}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel(sharedEntity_t * ent, const char *name)
{
	clipHandle_t    h;
	vec3_t          mins, maxs;

	if(!name)
	{
		Com_Error(ERR_DROP, "SV_SetBrushModel: NULL");
	}

	if(name[0] != '*')
	{
		Com_Error(ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name);
	}


	ent->s.modelindex = atoi(name + 1);

	h = CM_InlineModel(ent->s.modelindex);
	CM_ModelBounds(h, mins, maxs);
	VectorCopy(mins, ent->r.mins);
	VectorCopy(maxs, ent->r.maxs);
	ent->r.bmodel = qtrue;

	ent->r.contents = -1;		// we don't know exactly what is in the brushes

	// Tr3B: uncommented
	//SV_LinkEntity(ent);           // FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
qboolean SV_inPVS(const vec3_t p1, const vec3_t p2)
{
	int             leafnum;
	int             cluster;
	int             area1, area2;
	byte           *mask;

	leafnum = CM_PointLeafnum(p1);
	cluster = CM_LeafCluster(leafnum);
	area1 = CM_LeafArea(leafnum);
	mask = CM_ClusterPVS(cluster);

	leafnum = CM_PointLeafnum(p2);
	cluster = CM_LeafCluster(leafnum);
	area2 = CM_LeafArea(leafnum);
	if(mask && (!(mask[cluster >> 3] & (1 << (cluster & 7)))))
		return qfalse;
	if(!CM_AreasConnected(area1, area2))
		return qfalse;			// a door blocks sight
	return qtrue;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
qboolean SV_inPVSIgnorePortals(const vec3_t p1, const vec3_t p2)
{
	int             leafnum;
	int             cluster;
	int             area1, area2;
	byte           *mask;

	leafnum = CM_PointLeafnum(p1);
	cluster = CM_LeafCluster(leafnum);
	area1 = CM_LeafArea(leafnum);
	mask = CM_ClusterPVS(cluster);

	leafnum = CM_PointLeafnum(p2);
	cluster = CM_LeafCluster(leafnum);
	area2 = CM_LeafArea(leafnum);

	if(mask && (!(mask[cluster >> 3] & (1 << (cluster & 7)))))
		return qfalse;

	return qtrue;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState(sharedEntity_t * ent, qboolean open)
{
	svEntity_t     *svEnt;

	svEnt = SV_SvEntityForGentity(ent);
	if(svEnt->areanum2 == -1)
	{
		return;
	}
	CM_AdjustAreaPortalState(svEnt->areanum, svEnt->areanum2, open);
}


/*
==================
SV_GameAreaEntities
==================
*/
qboolean SV_EntityContact(vec3_t mins, vec3_t maxs, const sharedEntity_t * gEnt, traceType_t type)
{
	const float    *origin, *angles;
	clipHandle_t    ch;
	trace_t         trace;

	// check for exact collision
	origin = gEnt->r.currentOrigin;
	angles = gEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity(gEnt);
	CM_TransformedBoxTrace(&trace, vec3_origin, vec3_origin, mins, maxs, ch, -1, origin, angles, type);

	return trace.startsolid;
}


/*
===============
SV_GetServerinfo
===============
*/
void SV_GetServerinfo(char *buffer, int bufferSize)
{
	if(bufferSize < 1)
	{
		Com_Error(ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize);
	}
	Q_strncpyz(buffer, Cvar_InfoString(CVAR_SERVERINFO), bufferSize);
}

/*
===============
SV_GetUsercmd
===============
*/
void SV_GetUsercmd(int clientNum, usercmd_t * cmd)
{
	if(clientNum < 0 || clientNum >= sv_maxclients->integer)
	{
		Com_Error(ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum);
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}


// ====================================================================================



// handle to Game class
static jclass   class_Game = NULL;
static jobject  object_Game = NULL;
static jclass   interface_GameListener;
static jmethodID method_Game_ctor;
static jmethodID method_Game_initGame;
static jmethodID method_Game_shutdownGame;
static jmethodID method_Game_clientConnect;
static jmethodID method_Game_runFrame;
static jmethodID method_Game_runAIFrame;
static jmethodID method_Game_consoleCommand;

void Game_javaRegister()
{
	Com_DPrintf("Game_javaRegister()\n");

	// load the interface GameListener
	interface_GameListener = (*javaEnv)->FindClass(javaEnv, "xreal/server/game/GameListener");
	if(CheckException() || !interface_GameListener)
	{
		Com_Error(ERR_DROP, "Couldn't find class xreal.server.game.GameListener");
	}

	// load the class Game
	class_Game = (*javaEnv)->FindClass(javaEnv, "xreal/server/game/Game");
	if(CheckException() || !class_Game)
	{
		Com_Error(ERR_DROP, "Couldn't find class xreal.server.game.Game");
	}

	// check class Game against interface GameListener
	if(!((*javaEnv)->IsAssignableFrom(javaEnv, class_Game, interface_GameListener)))
	{
		Com_Error(ERR_DROP, "The specified game class doesn't implement xreal.server.game.GameListener");
	}

	// remove old game if existing
	(*javaEnv)->DeleteLocalRef(javaEnv, interface_GameListener);

	// load game interface methods
	method_Game_initGame = (*javaEnv)->GetMethodID(javaEnv, class_Game, "initGame", "(IIZ)V");
	method_Game_shutdownGame = (*javaEnv)->GetMethodID(javaEnv, class_Game, "shutdownGame", "(Z)V");
	method_Game_clientConnect = (*javaEnv)->GetMethodID(javaEnv, class_Game, "clientConnect", "(Lxreal/server/game/Player;ZZ)Ljava/lang/String;");
	method_Game_runFrame = (*javaEnv)->GetMethodID(javaEnv, class_Game, "runFrame", "(I)V");
	method_Game_runAIFrame = (*javaEnv)->GetMethodID(javaEnv, class_Game, "runAIFrame", "(I)V");
	method_Game_consoleCommand = (*javaEnv)->GetMethodID(javaEnv, class_Game, "consoleCommand", "()Z");
	if(CheckException())
	{
		Com_Error(ERR_DROP, "Problem getting handle for one or more of the game methods\n");
	}

	// load constructor
	method_Game_ctor = (*javaEnv)->GetMethodID(javaEnv, class_Game, "<init>", "()V");

	object_Game = (*javaEnv)->NewObject(javaEnv, class_Game, method_Game_ctor);
	if(CheckException())
	{
		Com_Error(ERR_DROP, "Couldn't create instance of game object");
	}
}


void Game_javaDetach()
{
	Com_DPrintf("Game_javaDetach()\n");

	if(javaEnv)
	{
		if(class_Game)
		{
			(*javaEnv)->DeleteLocalRef(javaEnv, class_Game);
			class_Game = NULL;
		}

		if(object_Game)
		{
			(*javaEnv)->DeleteLocalRef(javaEnv, object_Game);
			object_Game = NULL;
		}
	}
}

// ====================================================================================


/*
 * Class:     xreal_server_Server
 * Method:    getConfigstring
 * Signature: (I)Ljava/lang/String;
 */
jstring JNICALL Java_xreal_server_Server_getConfigString(JNIEnv *env, jclass cls, jint index)
{
	if(index < 0 || index >= MAX_CONFIGSTRINGS)
	{
		Com_Error(ERR_DROP, "Java_xreal_server_Server_getConfigString: bad index %i\n", index);
	}

	if(!sv.configstrings[index])
	{
		return NULL;
	}

	return (*env)->NewStringUTF(env, sv.configstrings[index]);
}

/*
 * Class:     xreal_server_Server
 * Method:    setConfigstring
 * Signature: (ILjava/lang/String;)V
 */
void JNICALL Java_xreal_server_Server_setConfigString(JNIEnv *env, jclass cls, jint jindex, jstring jvalue)
{
	char           *value;

	value = (char *)((*env)->GetStringUTFChars(env, jvalue, 0));

	SV_SetConfigstring(jindex, value);

	(*env)->ReleaseStringUTFChars(env, jvalue, value);

	//CheckException();
}

// handle to Server class
static jclass   class_Server;
static JNINativeMethod Server_methods[] = {
	{"getConfigString", "(I)Ljava/lang/String;", Java_xreal_server_Server_getConfigString},
	{"setConfigString", "(ILjava/lang/String;)V", Java_xreal_server_Server_setConfigString},
};

void Server_javaRegister()
{
	Com_DPrintf("Server_javaRegister()\n");

	class_Server = (*javaEnv)->FindClass(javaEnv, "xreal/server/Server");
	if(CheckException() || !class_Server)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.server.Server");
	}

	(*javaEnv)->RegisterNatives(javaEnv, class_Server, Server_methods, sizeof(Server_methods) / sizeof(Server_methods[0]));
	if(CheckException())
	{
		Com_Error(ERR_FATAL, "Couldn't register native methods for xreal.server.Server");
	}
}


void Server_javaDetach()
{
	Com_DPrintf("Server_javaDetach()\n");

	if(javaEnv)
	{
		if(class_Server)
		{
			(*javaEnv)->UnregisterNatives(javaEnv, class_Server);
			(*javaEnv)->DeleteLocalRef(javaEnv, class_Server);
			class_Server = NULL;
		}
	}
}


// ====================================================================================

void G_InitGentity(gentity_t * e)
{
	e->inUse = qtrue;
	e->spawnTime = g_levelTime;
//	e->classname = "noclass";
	e->s.number = e - g_entities;
	e->r.ownerNum = ENTITYNUM_NONE;
}


/*
=================
G_EntitiesFree
=================
*/
qboolean G_EntitiesFree(void)
{
	int             i;
	gentity_t      *e;

	e = &g_entities[MAX_CLIENTS];
	for(i = MAX_CLIENTS; i < sv.numEntities; i++, e++)
	{
		if(e->inUse)
		{
			continue;
		}

		// slot available
		return qtrue;
	}
	return qfalse;
}




// ====================================================================================

/*
 * Class:     xreal_server_game_GameEntity
 * Method:    allocateEntity
 * Signature: (I)I
 *
 * Either finds a free entity, or allocates a new one.
 *
 * The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
 * never be used by anything else.
 *
 * Try to avoid reusing an entity that was recently freed, because it
 * can cause the client to think the entity morphed into something else
 * instead of being removed and recreated, which can cause interpolated
 * angles and bad trails.
 *
 */
jint JNICALL Java_xreal_server_game_GameEntity_allocateEntity0(JNIEnv *env, jclass cls, jint reservedIndex)
{
	int             i, force;
	gentity_t      *e;

	e = NULL;					// shut up warning
	i = 0;						// shut up warning

	if(reservedIndex >= 0)
	{
		if(reservedIndex > (MAX_CLIENTS - 1) && reservedIndex != ENTITYNUM_WORLD)
		{
			Com_Error(ERR_DROP, "Java_xreal_server_game_GameEntity_allocateEntity0: bad reserved entity index chosen: %i", reservedIndex);
		}

		e = &g_entities[reservedIndex];

		G_InitGentity(e);
		return e->s.number;
	}

	for(force = 0; force < 2; force++)
	{
		// if we go through all entities and can't find one to free,
		// override the normal minimum times before use
		e = &g_entities[MAX_CLIENTS];

		for(i = MAX_CLIENTS; i < sv.numEntities; i++, e++)
		{
			if(e->inUse)
			{
				continue;
			}

			// the first couple seconds of server time can involve a lot of
			// freeing and allocating, so relax the replacement policy
			if(!force && e->freeTime > g_levelStartTime + 2000 && g_levelTime - e->freeTime < 1000)
			{
				continue;
			}

			// reuse this slot
			G_InitGentity(e);
			return e->s.number;
		}

		if(i != MAX_GENTITIES)
		{
			break;
		}
	}

	if(i == ENTITYNUM_MAX_NORMAL)
	{
		/*
		for(i = 0; i < MAX_GENTITIES; i++)
		{
			G_Printf("%4i: %s\n", i, g_entities[i].classname);
		}
		*/

		Com_Error(ERR_DROP, "Java_xreal_server_game_GameEntity_allocateEntity0: no free entities");
	}

	// open up a new slot
	sv.numEntities++;

	G_InitGentity(e);
	return e->s.number;
}

/*
 * Class:     xreal_server_game_GameEntity
 * Method:    freeEntity0
 * Signature: (I)Z
 */
jboolean JNICALL Java_xreal_server_game_GameEntity_freeEntity0(JNIEnv *env, jclass cls, jint index)
{
	//gentity_t *e = (gentity_t *) entityPtr;
	gentity_t *e = &g_entities[index];

	//trap_UnlinkEntity(e);		// unlink from world

	if(e->neverFree)
	{
		return qfalse;
	}

	memset(e, 0, sizeof(*e));
	//e->classname = "freed";
	e->freeTime = g_levelTime;
	e->inUse = qfalse;

	return qtrue;
}


// handle to GameEntity class
static jclass   class_GameEntity;
static JNINativeMethod GameEntity_methods[] = {
	{"allocateEntity0", "(I)I", Java_xreal_server_game_GameEntity_allocateEntity0},
	{"freeEntity0", "(I)Z", Java_xreal_server_game_GameEntity_freeEntity0},
};

void GameEntity_javaRegister()
{
	Com_DPrintf("GameEntity_javaRegister()\n");

	class_GameEntity = (*javaEnv)->FindClass(javaEnv, "xreal/server/game/GameEntity");
	if(CheckException() || !class_GameEntity)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.server.game.GameEntity");
	}

	(*javaEnv)->RegisterNatives(javaEnv, class_GameEntity, GameEntity_methods, sizeof(GameEntity_methods) / sizeof(GameEntity_methods[0]));
	if(CheckException())
	{
		Com_Error(ERR_FATAL, "Couldn't register native methods for xreal.server.game.GameEntity");
	}
}


void GameEntity_javaDetach()
{
	Com_DPrintf("GameEntity_javaDetach()\n");

	if(javaEnv)
	{
		if(class_GameEntity)
		{
			(*javaEnv)->UnregisterNatives(javaEnv, class_GameEntity);
			(*javaEnv)->DeleteLocalRef(javaEnv, class_GameEntity);
			class_GameEntity = NULL;
		}
	}
}

// ====================================================================================


static jclass   interface_ClientListener;

void ClientListener_javaRegister()
{
	Com_DPrintf("ClientListener_javaRegister()\n");

	interface_ClientListener = (*javaEnv)->FindClass(javaEnv, "xreal/server/game/ClientListener");
	if(CheckException() || !interface_ClientListener)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.server.game.ClientListener");
	}
}


void ClientListener_javaDetach()
{
	Com_DPrintf("ClientListener_javaDetach()\n");

	if(javaEnv)
	{
		if(interface_ClientListener)
		{
			(*javaEnv)->DeleteLocalRef(javaEnv, interface_ClientListener);
			interface_ClientListener = NULL;
		}
	}
}

// ====================================================================================

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs(void)
{
	if(!javaEnv)
	{
		Com_Printf("Can't stop Java VM, javaEnv pointer was null\n");
		return;
	}

	Java_G_ShutdownGame(qfalse);

	Server_javaDetach();
	Game_javaDetach();
	GameEntity_javaDetach();
	ClientListener_javaDetach();
}



void Java_G_GameInit(int levelTime, int randomSeed, qboolean restart)
{
#if 1
	if(!object_Game)
		return;

	g_levelStartTime = levelTime;
	g_levelTime = levelTime;

	(*javaEnv)->CallVoidMethod(javaEnv, object_Game, method_Game_initGame, levelTime, randomSeed, restart);

	CheckException();
#endif
}

void Java_G_ShutdownGame(qboolean restart)
{
#if 1
	if(!object_Game)
		return;

	(*javaEnv)->CallVoidMethod(javaEnv, object_Game, method_Game_shutdownGame, restart);

	CheckException();
#endif
}



/*
 * Class:     xreal_server_game_Player
 * Method:    getUserInfo0
 * Signature: (I)Ljava/lang/String;
 */
jstring JNICALL Java_xreal_server_game_Player_getUserInfo0(JNIEnv *env, jclass cls, jint clientNum)
{
	if(clientNum < 0 || clientNum >= MAX_CONFIGSTRINGS)
	{
		Com_Error(ERR_DROP, "Java_xreal_server_game_Player_getUserInfo0: bad index %i\n", clientNum);
	}

	return (*env)->NewStringUTF(env, svs.clients[clientNum].userinfo);
}

/*
 * Class:     xreal_server_game_Player
 * Method:    setUserInfo0
 * Signature: (ILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_xreal_server_game_Player_setUserInfo0(JNIEnv *env, jclass cls, jint clientNum, jstring juserinfo)
{
	char           *userinfo;

	userinfo = (char *)((*env)->GetStringUTFChars(env, juserinfo, 0));

	SV_SetUserinfo(clientNum, userinfo);

	(*env)->ReleaseStringUTFChars(env, juserinfo, userinfo);
}

static JNINativeMethod Player_methods[] = {
	{"getUserInfo0", "(I)Ljava/lang/String;", Java_xreal_server_game_Player_getUserInfo0},
	{"setUserInfo0", "(ILjava/lang/String;)V", Java_xreal_server_game_Player_setUserInfo0},
};

char           *Java_G_ClientConnect(int clientNum, qboolean firstTime, qboolean isBot)
{
	gclient_t	   *client;

	jclass			class_Player;
	jmethodID		method_Player_ctor;
	jobject         object_Player;

	static char		string[MAX_STRING_CHARS];
	jobject 		result;
	jthrowable      exception;

	extern jmethodID method_Throwable_getMessage;

	if(!object_Game)
		return "no Game class loaded yet!!!";

	Com_Printf("Java_G_ClientBegin(%i)\n", clientNum);

	client = &g_clients[clientNum];

	// load the class Game
	class_Player = (*javaEnv)->FindClass(javaEnv, "xreal/server/game/Player");
	if(CheckException() || !class_Player)
	{
		Com_Error(ERR_DROP, "Couldn't find class xreal.server.game.Player");
	}

	// check class Game against interface GameListener
	if(!((*javaEnv)->IsAssignableFrom(javaEnv, class_Player, interface_ClientListener)))
	{
		Com_Error(ERR_DROP, "The current Player class doesn't implement xreal.server.game.ClientListener");
	}

	// register the native methods
	(*javaEnv)->RegisterNatives(javaEnv, class_Player, Player_methods, sizeof(Player_methods) / sizeof(Player_methods[0]));
	if(CheckException())
	{
		Com_Error(ERR_DROP, "Couldn't register native methods for xreal.server.game.Player");
	}

	// load constructor
	method_Player_ctor = (*javaEnv)->GetMethodID(javaEnv, class_Player, "<init>", "(IZZ)V");

	client->method_Player_clientBegin = (*javaEnv)->GetMethodID(javaEnv, class_Player, "clientBegin", "()V");
	client->method_Player_clientUserInfoChanged = (*javaEnv)->GetMethodID(javaEnv, class_Player, "clientUserInfoChanged", "(Ljava/lang/String;)V");
	client->method_Player_clientDisconnect = (*javaEnv)->GetMethodID(javaEnv, class_Player, "clientDisconnect", "()V");
	client->method_Player_clientCommand = (*javaEnv)->GetMethodID(javaEnv, class_Player, "clientCommand", "()V");
	client->method_Player_clientThink = (*javaEnv)->GetMethodID(javaEnv, class_Player, "clientThink", "()V");

	if(CheckException())
	{
		Com_Error(ERR_DROP, "Problem getting handle for one or more of the Player methods\n");
	}

	// create new player object
	client->object_Player = object_Player = (*javaEnv)->NewObject(javaEnv, class_Player, method_Player_ctor, clientNum, firstTime, isBot);
	exception = (*javaEnv)->ExceptionOccurred(javaEnv);
	if(exception)
	{
		//Com_Error(ERR_DROP, "Couldn't create instance of Player object");

		result = (*javaEnv)->CallObjectMethod(javaEnv, exception, method_Throwable_getMessage);

		ConvertJavaString(string, result, sizeof(string));

		(*javaEnv)->ExceptionClear(javaEnv);

		//Com_Printf("Java_G_ClientConnect '%s'\n", string);

		return string;
	}

	return NULL;
}

void Java_G_ClientBegin(int clientNum)
{
	gclient_t	   *client;

	if(!object_Game)
		return;

	Com_Printf("Java_G_ClientBegin(%i)\n", clientNum);

	client = &g_clients[clientNum];

	(*javaEnv)->CallVoidMethod(javaEnv, client->object_Player, client->method_Player_clientBegin);

	CheckException();
}

void Java_G_ClientUserInfoChanged(int clientNum)
{
	gclient_t	   *client;
	jstring			userinfo;

	if(!object_Game)
		return;

	//Com_Printf("Java_G_ClientUserInfoChanged(%i)\n", clientNum);

	client = &g_clients[clientNum];
	userinfo = (*javaEnv)->NewStringUTF(javaEnv, svs.clients[clientNum].userinfo);

	(*javaEnv)->CallVoidMethod(javaEnv, client->object_Player, client->method_Player_clientUserInfoChanged, userinfo);

	CheckException();
}

void Java_G_ClientDisconnect(int clientNum)
{
	gclient_t	   *client;

	if(!object_Game)
		return;

	Com_Printf("Java_G_ClientDisconnect(%i)\n", clientNum);

	client = &g_clients[clientNum];

	(*javaEnv)->CallVoidMethod(javaEnv, client->object_Player, client->method_Player_clientDisconnect);

	CheckException();
}

void Java_G_ClientCommand(int clientNum)
{
	gclient_t	   *client;

	if(!object_Game)
		return;

	Com_Printf("Java_G_ClientCommand(%i)\n", clientNum);

	client = &g_clients[clientNum];

	(*javaEnv)->CallVoidMethod(javaEnv, client->object_Player, client->method_Player_clientCommand);

	CheckException();
}

void Java_G_ClientThink(int clientNum)
{
	gclient_t	   *client;

	if(!object_Game)
		return;

	//Com_Printf("Java_G_ClientThink(%i)\n", clientNum);

	client = &g_clients[clientNum];

	(*javaEnv)->CallVoidMethod(javaEnv, client->object_Player, client->method_Player_clientThink);

	CheckException();
}

void Java_G_RunFrame(int time)
{
	if(!object_Game)
		return;

	g_levelTime = time;

	(*javaEnv)->CallVoidMethod(javaEnv, object_Game, method_Game_runFrame, time);

	CheckException();
}

void Java_G_RunAIFrame(int time)
{
	if(!object_Game)
		return;

	(*javaEnv)->CallVoidMethod(javaEnv, object_Game, method_Game_runAIFrame, time);

	CheckException();
}

qboolean Java_G_ConsoleCommand(void)
{
	jboolean result;

	if(!object_Game)
		return qfalse;

	result = (*javaEnv)->CallBooleanMethod(javaEnv, object_Game, method_Game_consoleCommand);

	CheckException();

	return result;
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM(qboolean restart)
{
	int             i;

	Com_DPrintf("SV_InitGameVM(restart = %i)\n", restart);

	// start the entity parsing at the beginning
	sv.entityParsePoint = CM_EntityString();

	// clear all gentity pointers that might still be set from
	// a previous level
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=522
	//   now done before GAME_INIT call
	for(i = 0; i < sv_maxclients->integer; i++)
	{
		svs.clients[i].gentity = NULL;
	}

	// initialize all entities for this game
	memset(g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]));

	// initialize all clients for this game
//	level.maxclients = g_maxclients.integer;
	memset(g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]));

	// set client fields on player ents
	for(i = 0; i < sv_maxclients->integer; i++)
	{
		g_entities[i].client = g_clients + i;
	}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	sv.numEntities = MAX_CLIENTS;

	// use the current msec count for a random seed
	// init for this gamestate
	Java_G_GameInit(sv.time, Com_Milliseconds(), restart);
}



/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs(void)
{
	Com_DPrintf("SV_RestartGameProgs()\n");

	if(!javaEnv)
	{
		return;
	}

	Java_G_ShutdownGame(qtrue);
	SV_InitGameVM(qtrue);
}




/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs(void)
{
	Com_DPrintf("SV_InitGameProgs()\n");

	Server_javaRegister();
	Game_javaRegister();
	GameEntity_javaRegister();
	ClientListener_javaRegister();

	SV_InitGameVM(qfalse);
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
qboolean SV_GameCommand(void)
{
	if(sv.state != SS_GAME)
	{
		return qfalse;
	}

	return Java_G_ConsoleCommand();
}

#endif							//defined(USE_JAVA
