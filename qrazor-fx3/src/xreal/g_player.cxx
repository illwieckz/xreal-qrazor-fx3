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
// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_player.h"
#include "g_item.h"
#include "g_target.h"
#include "g_projectile.h"


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(g_entity_c **entity)
{
	//if (!coop->getInteger())
	//	return;
		
	g_entity_c *ent = new g_target_teleport_c();
	*entity = ent;
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/

void SP_info_player_deathmatch(g_entity_c **entity)
{
	if(!deathmatch->getInteger())
		return;
	
	g_entity_c *ent = new g_target_teleport_c();
	*entity = ent;
}

/*QUAKED info_player_coop (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for coop games
*/

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
/*
void SP_info_player_intermission(void)
{
}
*/










/*
================================================================================
				PLAYER
================================================================================
*/

g_player_c::g_player_c()
:g_entity_c(false)
{
	_s.type = ET_PLAYER;
	
	_pers.clear();
	clearAllButPersistant();

	// setup ODE rigid body
//	_body->setPosition(start);
//	_body->setQuaternion(_s.quat);
//	_body->setLinearVel(dir * speed);
//	_body->setGravityMode(0);
	
	// setup rigid body
//	dMass m;
//	dMassSetBoxTotal(&m, 30, 32, 32, 56);
//	dMassSetSphereTotal(&m, 200, 16);
//	dMassSetCappedCylinderTotal(&m, 10, 2, 32, 56);
//	dMassTranslate(&m, 0, 0, -28);

//	_body->setMass(&m);
//	_body->disable();
			
	// setup ODE collision detection
//	_space = new d_simple_space_c(g_ode_space_toplevel->getId());
//	_space->setCollideBits(MASK_PLAYERSOLID);
	
//	geom = new d_box_c(g_ode_space->getId(), vec3_c(32, 32, 56));
//	geom = new d_ccylinder_c(g_ode_space->getId(), 32, 56);

//	_sphere = new d_sphere_c(_space->getId(), 16);
// 	_sphere->setBody(_body->getId());
//	_sphere->setData(this);
//	_sphere->setCollideBits(MASK_PLAYERSOLID);
	
//	_geoms.push_back(_sphere);
	
	/*
	_ray = new d_ray_c(_space->getId(), vec3_c(0, 0, 0), vec3_c(0, 0, -1), 18);
	_ray->setBody(_body->getId());
	_ray->setData(geom_info);
	_ray->setCollideBits(MASK_PLAYERSOLID);
	_geoms.insert(std::make_pair(_ray, geom_info));
	*/
}



g_player_c::~g_player_c()
{
	//trap_Com_Printf("g_player_c::dtor:\n");
}

/*
void	g_player_c::pain(g_entity_c *other, float kick, int damage)
{
	// player pain is handled at the end of the frame in P_DamageFeedback
}
*/

void	g_player_c::die(g_entity_c *inflictor, g_entity_c *attacker, int damage, const vec3_c &point)
{
// 	_body->setAngularVel(0, 0, 0);

	_takedamage = DAMAGE_YES;
	_movetype = MOVETYPE_TOSS;

	//_s.angles[0] = 0;
	//_s.angles[2] = 0;

	_s.index_sound = 0;
	_weapon_sound = 0;

	_r.bbox._maxs[2] = -8;

//	self->solid = SOLID_NOT;
	_r.svflags |= SVF_CORPSE;

	if(!_deadflag)
	{
		_time_respawn = level.time + 1000;
		
		lookAtKiller(inflictor, attacker);
		
		_r.ps.pmove.pm_type = PM_DEAD;
		
		clientObituary(inflictor, attacker);
		
		tossWeapon();
		
		//if(deathmatch->getInteger())
		//	Cmd_Help_f (this);		// show scores

		// clear inventory
		// this is kind of ugly, but it's how we want to handle keys in coop
		for(int i=0; i<MAX_ITEMS; i++)
		{
			//g_item_c *item = g_items[i];
			
			//if(coop->getInteger() && item->getFlags() & IT_KEY)
			//	_resp.coop_respawn.inventory[i] = _pers.inventory[i];
				
			_pers.inventory[i] = 0;
		}
	}

	// remove powerups
	_quad_framenum = 0;
	_invincible_framenum = 0;
	_breather_framenum = 0;
	_enviro_framenum = 0;
	_flags &= ~FL_POWER_ARMOR;



	if(_health < -40)
	{
#if 0	
		// gib
		trap_SV_StartSound (NULL, self, CHAN_BODY, trap_SV_SoundIndex("misc/udeath.wav"), 1, ATTN_NORM, 0);
		for (n= 0; n < 4; n++)
			ThrowGib(self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
		ThrowClientHead(self, damage);
#endif

		_takedamage = DAMAGE_NO;
	}
	else
	{	// normal death
		if(!_deadflag)
		{
			static int i;

			i = (i+1)%3;
			// start a death animation
			_anim_priority = ANIM_DEATH;
			if(_r.ps.pmove.pm_flags & PMF_DUCKED)
			{
				//_anim_current[PLAYER_BODY_PART_HEAD] = PLAYER_ANIM_HEAD_IDLE;
				//_anim_current[PLAYER_BODY_PART_UPPER] = PLAYER_ANIM_UPPER_DEATH3;
				//_anim_current[PLAYER_BODY_PART_LOWER] = PLAYER_ANIM_LOWER_DEATH3;
				//_anim_time = PLAYER_ANIM_BOTH_DEATH3_TIME;
				
				//_s.frame = FRAME_crdeath1-1;
				//_anim_end = FRAME_crdeath5;
			}
			else switch(i)
			{
				case 0:
					//_s.frame = FRAME_death101-1;
					//_anim_end = FRAME_death106;
					
					//_anim_current[PLAYER_BODY_PART_HEAD] = PLAYER_ANIM_HEAD_IDLE
					//_anim_current[PLAYER_BODY_PART_UPPER] = PLAYER_ANIM_UPPER_DEATH1;
					//_anim_current[PLAYER_BODY_PART_LOWER] = PLAYER_ANIM_LOWER_DEATH1;
					//_anim_time = PLAYER_ANIM_BOTH_DEATH1_TIME;
					break;
				
				case 1:
					//_s.frame = FRAME_death201-1;
					//_anim_end = FRAME_death206;
					
					//_anim_current[PLAYER_BODY_PART_HEAD] = PLAYER_ANIM_HEAD_IDLE
					//_anim_current[PLAYER_BODY_PART_UPPER] = PLAYER_ANIM_UPPER_DEATH2;
					//_anim_current[PLAYER_BODY_PART_LOWER] = PLAYER_ANIM_LOWER_DEATH2;
					//_anim_time = PLAYER_ANIM_BOTH_DEATH2_TIME;
					break;
				
				case 2:
					//_s.frame = FRAME_death301-1;
					//_anim_end = FRAME_death308;
					
					//_anim_current[PLAYER_BODY_PART_HEAD] = PLAYER_ANIM_HEAD_IDLE
					//_anim_current[PLAYER_BODY_PART_UPPER] = PLAYER_ANIM_UPPER_DEATH3;
					//_anim_current[PLAYER_BODY_PART_LOWER] = PLAYER_ANIM_LOWER_DEATH3;
					//_anim_time = PLAYER_ANIM_BOTH_DEATH3_TIME;
					break;
			}
			//trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
		}
	}

	_deadflag = DEAD_DEAD;
}



std::string	g_player_c::clientTeam()
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	strcpy(value, _pers.userinfo.getValueForKey("skin"));
	p = strchr(value, '/');
	if(!p)
		return value;

	if(dmflags->getInteger() & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->getInteger()) & DF_SKINTEAMS)
	return ++p;
}

void	g_player_c::takeDamage(g_entity_c *inflictor, g_entity_c *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod)
{
	int			take;
	int			save;
	//int			asave;
	//int			psave;
	int			te_sparks;

	if(!_takedamage)
		return;

	// friendly fire avoidance
	// if enabled you can't hurt teammates (but you can hurt yourself)
	// knockback still occurs
	if((this != attacker) && ((deathmatch->getInteger() && (dmflags->getInteger() & (DF_MODELTEAMS | DF_SKINTEAMS))) || coop->getInteger()))
	{
		if(G_OnSameTeam(this, attacker))
		{
			if(dmflags->getInteger() & DF_NO_FRIENDLY_FIRE)
				damage = 0;
			else
				mod |= MOD_FRIENDLY_FIRE;
		}
	}
	meansOfDeath = mod;

	// easy mode takes half damage
	if(skill->getInteger() == 0 && deathmatch->getInteger() == 0)
	{
		damage *= (int)0.5;
		if(!damage)
			damage = 1;
	}

	if(dflags & DAMAGE_BULLET)
		te_sparks = TE_BULLET_SPARKS;
	else
		te_sparks = TE_SPARKS;

	Vector3_Normalize(dir);

	
	if(_flags & FL_NO_KNOCKBACK)
		knockback = 0;

	// figure momentum add
	if(!(dflags & DAMAGE_NO_KNOCKBACK))
	{
		if((knockback) && (_movetype != MOVETYPE_NONE) && (_movetype != MOVETYPE_BOUNCE) && (_movetype != MOVETYPE_PUSH) && (_movetype != MOVETYPE_STOP))
		{
			vec3_c	kvel;
			float	mass;

			if(_mass < 50)
				mass = 50;
			else
				mass = _mass;

			if(attacker == this)
				Vector3_Scale(dir, 1600.0 * (float)knockback / mass, kvel);	// the rocket jump hack...
			else
				Vector3_Scale(dir, 500.0 * (float)knockback / mass, kvel);

			//_body->addForce(kvel);
		}
	}

	take = damage;
	save = 0;

	// check for godmode
	if((_flags & FL_ULTRAMANMODE) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		take = 0;
		save = damage;
		G_SpawnDamage(te_sparks, point, normal, save);
	}

	// check for invincibility
	/*
	if((client && _invincible_framenum > level.framenum ) && !(dflags & DAMAGE_NO_PROTECTION))
	{
		if (targ->_pain_debounce_time < level.time)
		{
			trap_SV_StartSound (NULL, targ, CHAN_ITEM, trap_SV_SoundIndex("items/protect4.wav"), 1, ATTN_NORM, 0);
			targ->_pain_debounce_time = level.time + 2000;
		}
		
		take = 0;
		save = damage;
	}
	*/

	//psave = CheckPowerArmor (targ, point, normal, take, dflags);
	//take -= psave;

	//asave = CheckArmor (targ, point, normal, take, te_sparks, dflags);
	//take -= asave;

	//treat cheat/powerup savings the same as armor
	//asave += save;

	// team damage avoidance
	//if(!(dflags & DAMAGE_NO_PROTECTION) && CheckTeamDamage(targ, attacker))
	//	return;

	// do the damage
	if(take)
	{
		G_SpawnDamage(TE_BLOOD, point, normal, take);
		
		_health -= take;
			
		if(_health <= 0)
		{
			_flags |= FL_NO_KNOCKBACK;
				
			G_Killed(this, inflictor, attacker, take, point);
			return;
		}
	}

	
	//if(client)
	//{
	//	if(!(_flags & FL_ULTRAMANMODE) && (take))
	//		pain (attacker, knockback, take);
	//}
	//else */
	//if(take)
	//{
	//	if(targ->hasPain())
	//		targ->pain(attacker, knockback, take);
	//}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	
	//if(client)
	{
		//_damage_parmor += psave;
		//_damage_armor += asave;
		_damage_blood += take;
		_damage_knockback += knockback;
		_damage_from = point;
	}
	
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
bool	g_player_c::clientConnect(info_c &userinfo)
{
	//g_entity_c*	ent = (g_entity_c*)entity;
	
	const char	*value;

	//
	// check to see if they are on the banned IP list
	//
	value = userinfo.getValueForKey("ip");
	if(SV_FilterPacket(value))
	{
		userinfo.setValueForKey("rejmsg", "Banned.");
		return false;
	}

	//
	// check for a spectator
	//
	value = userinfo.getValueForKey("spectator");
	if(deathmatch->getInteger() && *value && strcmp(value, "0"))
	{
		int i, numspec;

		if(!X_strequal(spectator_password->getString(), "") && !X_strequal(spectator_password->getString(), "none") && !X_strequal(spectator_password->getString(), value)) 
		{
			userinfo.setValueForKey("rejmsg", "Spectator password required or incorrect.");
			return false;
		}

		// count spectators
		for(i=numspec=0; i < maxclients->getInteger(); i++)
			if (g_entities[i+1]->_r.inuse && ((g_player_c*)g_entities[i+1])->_pers.spectator)
				numspec++;

		if(numspec >= maxspectators->getInteger()) 
		{
			userinfo.setValueForKey("rejmsg", "Server spectator limit is full.");
			return false;
		}
	} 
	else 
	{
		// check for a password
		value = userinfo.getValueForKey("password");
		if(!X_strequal(password->getString(), "") && !X_strequal(password->getString(), "none") && !X_strequal(password->getString(), value)) 
		{
			userinfo.setValueForKey("rejmsg", "Password required or incorrect.");
			return false;
		}
	}

	//
	// they can connect
	//
	//trap_Com_Printf("G_ClientConnect: ent->s.number %i\n", ent->s.number);
	_r.isclient = true;

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if(_r.inuse == false)
	{
		// clear the respawning variables
		initClientResp();
		
		if(!game.autosaved || !_pers.weapon)
			initClientPersistant();
	}

	clientUserinfoChanged(userinfo);

	if(game.maxclients > 1)
		trap_Com_DPrintf("%s connected\n", _pers.netname);

	_r.svflags = 0; // make sure we start with known default
	_pers.connected = true;
	
	return true;
}

/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void	g_player_c::clientBegin()
{
	if(!_r.isclient)
		trap_Com_Error(ERR_DROP, "g_player_c::clientBegin: got no client");

	if(deathmatch->getInteger())
	{
		clientBeginDeathmatch();
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if(_r.inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		_r.ps.pmove.delta_angles = _r.ps.view_angles;
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		initClientResp();
		
		putClientInServer();
	}

	if(level.intermission_time)
	{
		moveToIntermission();
	}
	else
	{
		// send effect if in a multiplayer game
		if(game.maxclients > 1)
		{
			trap_SV_WriteBits(SVC_MUZZLEFLASH, svc_bitcount);
			trap_SV_WriteShort(_s.getNumber());
			trap_SV_WriteByte(MZ_LOGIN);
			trap_SV_Multicast(_s.origin, MULTICAST_PVS);

			trap_SV_BPrintf(PRINT_HIGH, "%s entered the game\n", _pers.netname);
		}
	}

	// make sure all view stuff is valid
	endServerFrame();
}


/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void	g_player_c::clientBeginDeathmatch()
{
	initClientResp();

	// locate ent at a spawn point
	putClientInServer();

	if(level.intermission_time)
	{
		moveToIntermission();
	}
	else
	{
		// send effect
		trap_SV_WriteBits(SVC_MUZZLEFLASH, svc_bitcount);
		trap_SV_WriteShort(_s.getNumber());
		trap_SV_WriteByte(MZ_LOGIN);
		trap_SV_Multicast(_s.origin, MULTICAST_PVS);
	}

	trap_SV_BPrintf(PRINT_HIGH, "%s entered the game\n", _pers.netname);
	
	//trap_Com_Printf("G_ClientBeginDeathmatch:\n");

	// make sure all view stuff is valid
	endServerFrame();
}


/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void	g_player_c::clientUserinfoChanged(info_c &userinfo)
{
	const char	*s;
	int		playernum;
	
	// check for malformed or illegal info strings
	/*
	if(!Info_Validate(userinfo))
	{
		Com_sprintf(userinfo, sizeof(userinfo), "\\name\\badinfo\\skin\\%s/%s", DEFAULT_PLAYERMODEL, DEFAULT_PLAYERSKIN);
	}
	*/

	// set name
	s = userinfo.getValueForKey("name");
	strncpy(_pers.netname, s, sizeof(_pers.netname)-1);

	// set spectator
	s = userinfo.getValueForKey("spectator");
	
	// spectators are only supported in deathmatch
	if(deathmatch->getInteger() && *s && strcmp(s, "0"))
		_pers.spectator = true;
	else
		_pers.spectator = false;
		
	// scan for proper animation files
	s = userinfo.getValueForKey("model");
	scanAnimations(s);

	// set skin
	s = userinfo.getValueForKey("skin");

	playernum = _s.getNumber() -1;

	// combine name and skin into a configstring
	trap_SV_SetConfigString(CS_PLAYERSKINS + playernum, va("%s\\%s", _pers.netname, s));

	// fov
	if(deathmatch->getInteger() && (dmflags->getInteger() & DF_FIXED_FOV))
	{
		_r.ps.fov = 90;
	}
	else
	{
		_r.ps.fov = atoi(userinfo.getValueForKey("fov"));
		
		if(_r.ps.fov < 1)
			_r.ps.fov = 90;
		
		else if(_r.ps.fov > 160)
			_r.ps.fov = 160;
	}

	// handedness
	s = userinfo.getValueForKey("hand");
	if(strlen(s))
	{
		_pers.hand = atoi(s);
	}

	// save off the userinfo in case we want to check something later
	_pers.userinfo = userinfo;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void	g_player_c::clientDisconnect()
{
	int		playernum;
	
	//if(!getClient())
	//	return;

	trap_SV_BPrintf(PRINT_HIGH, "%s disconnected\n", _pers.netname);

	// send effect
	trap_SV_WriteBits(SVC_MUZZLEFLASH, svc_bitcount);
	trap_SV_WriteShort(_s.getNumber());
	trap_SV_WriteByte(MZ_LOGOUT);
	trap_SV_Multicast(_s.origin, MULTICAST_PVS);
	
	_s.index_model = 0;
	_r.solid = SOLID_NOT;
	_r.inuse = false;
	_classname = "disconnected";
	_pers.connected = false;

	playernum = _s.getNumber() - 1;
	
	trap_SV_SetConfigString(CS_PLAYERSKINS + playernum, "");
}


void	g_player_c::clientCommand()
{
	const char	*cmd;
	

	if(!_r.isclient)
		return;		// not fully in game yet
		
	cmd = trap_Cmd_Argv(0);
	
#if 0
	if(X_strcaseequal(cmd, "addnode") == 0 && debug_mode)
	{
		_last_node = ACEND_AddNode(this, atoi(trap_Cmd_Argv(1)));
		return;
	}	
	else if(X_strcaseequal(cmd, "removelink") == 0 && debug_mode)
	{
		ACEND_RemoveNodeEdge(this, atoi(trap_Cmd_Argv(1)), atoi(trap_Cmd_Argv(2)));
		return;
	}
	else if(X_strcaseequal(cmd, "addlink") == 0 && debug_mode)
	{
		ACEND_UpdateNodeEdge(atoi(trap_Cmd_Argv(1)), atoi(trap_Cmd_Argv(2)));
		return;
	}
	else if(X_strcaseequal(cmd, "showpath") == 0 && debug_mode)
    	{
		ACEND_ShowPath(this, atoi(trap_Cmd_Argv(1)));
		return;
	}
	else if(X_strcaseequal(cmd, "findnode") == 0 && debug_mode)
	{
		int node = ACEND_FindClosestReachableNode(this, NODE_DENSITY, NODE_ALL);
		
		trap_SV_BPrintf(PRINT_MEDIUM, "node: %d type: %d x: %f y: %f z %f\n", node, nodes[node].type, nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2]);
		return;
	}
	else if(X_strcaseequal(cmd, "movenode") == 0 && debug_mode)
	{
		int node = atoi(trap_Cmd_Argv(1));
		
		nodes[node].origin[0] = atof(trap_Cmd_Argv(2));
		nodes[node].origin[1] = atof(trap_Cmd_Argv(3));
		nodes[node].origin[2] = atof(trap_Cmd_Argv(4));
		
		trap_SV_BPrintf(PRINT_MEDIUM, "node: %d moved to x: %f y: %f z %f\n", node, nodes[node].origin[0], nodes[node].origin[1], nodes[node].origin[2]);
		return;
	}
#endif

	if(X_stricmp(cmd, "players") == 0)
	{
		players_f();
		return;
	}
	
	if(X_stricmp(cmd, "say") == 0)
	{
		say_f(false, false);
		return;
	}
	
	if(X_stricmp(cmd, "say_team") == 0)
	{
		say_f(true, false);
		return;
	}
	
	if(X_stricmp(cmd, "score") == 0)
	{
		score_f();
		return;
	}

	if(level.intermission_time)
		return;

	if(X_stricmp(cmd, "use") == 0)
		use_f();
		
	else if(X_stricmp(cmd, "drop") == 0)
		drop_f();
		
	else if(X_stricmp(cmd, "give") == 0)
		give_f();
		
	else if(X_stricmp(cmd, "ultraman") == 0)
		ultraman_f();
		
	else if(X_stricmp(cmd, "notarget") == 0)
		notarget_f();
		
	else if(X_stricmp(cmd, "noclip") == 0)
		noclip_f();
		
	else if(X_stricmp(cmd, "inven") == 0)
		inven_f();
		
	else if(X_stricmp(cmd, "invnext") == 0)
		selectNextItem(-1);
		
	else if(X_stricmp(cmd, "invprev") == 0)
		selectPrevItem(-1);
		
	else if(X_stricmp(cmd, "invnextw") == 0)
		selectNextItem(IT_WEAPON);
		
	else if(X_stricmp(cmd, "invprevw") == 0)
		selectPrevItem(IT_WEAPON);
		
	else if(X_stricmp(cmd, "invnextp") == 0)
		selectNextItem(IT_POWERUP);
		
	else if(X_stricmp(cmd, "invprevp") == 0)
		selectPrevItem(IT_POWERUP);
		
	else if(X_stricmp(cmd, "invuse") == 0)
		invUse_f();
		
	else if(X_stricmp(cmd, "invdrop") == 0)
		invDrop_f();
		
	else if(X_stricmp(cmd, "weapprev") == 0)
		weapPrev_f();
		
	else if(X_stricmp(cmd, "weapnext") == 0)
		weapNext_f();
		
	else if(X_stricmp(cmd, "weaplast") == 0)
		weapLast_f();
		
	else if(X_stricmp(cmd, "weapreload") == 0)
		weapReload_f();
		
	else if(X_stricmp(cmd, "kill") == 0)
		kill_f();
		
	else if(X_stricmp(cmd, "putaway") == 0)
		putAway_f();
		
	else if(X_stricmp(cmd, "wave") == 0)
		wave_f();
		
	else if(X_stricmp(cmd, "playerlist") == 0)
		playerList_f();

	else if(X_stricmp(cmd, "animnext") == 0)
		animNext_f();

	else if(X_stricmp(cmd, "animprev") == 0)
		animPrev_f();

	else if(X_stricmp(cmd, "animreset") == 0)
		animReset_f();

	else if(X_stricmp(cmd, "spawnrocket") == 0)
		spawnRocket_f();
				
	else	// anything that doesn't match a command will be a chat
		say_f(false, true);
}



// pmove doesn't need to know about passent and contentmask
/*
trace_t	PM_Trace(const vec3_c &start, const aabb_c &bbox, const vec3_c &end)
{
	trace_t trace;
	
	memset(&trace, 0, sizeof(trace));
	
	return trace;
}
*/

static g_entity_c*	pm_passent;

// pmove doesn't need to know about passent and contentmask
static trace_t	PM_Trace(const vec3_c &start, const aabb_c &aabb, const vec3_c &end)
{
	if(pm_passent->_health > 0)
		return G_Trace(start, aabb, end, pm_passent, MASK_PLAYERSOLID);
	else
		return G_Trace(start, aabb, end, pm_passent, MASK_DEADSOLID);
}


/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void	g_player_c::clientThink(const usercmd_t &cmd)
{
	if(level.intermission_time)
	{
		_r.ps.pmove.pm_type = PM_FREEZE;
		
		// can exit intermission after five seconds
		if(level.time > level.intermission_time + 5000 && (cmd.buttons & BUTTON_ANY))
			level.intermission_exit = true;
			
		return;
	}
	
	pm_passent = this;

	// set up for pmove
	pmove_t	pm;
	pm.clear();

	if(_movetype == MOVETYPE_NOCLIP)
		_r.ps.pmove.pm_type = PM_SPECTATOR;
			
//	else if(_s.index_model != 255)
//		_r.ps.pmove.pm_type = PM_GIB;
			
	else if(_deadflag)
		_r.ps.pmove.pm_type = PM_DEAD;
			
	else
		_r.ps.pmove.pm_type = PM_NORMAL;

#if defined(ODE)
	_r.ps.pmove.gravity = (9.81 * 32.0 * 2.3) * g_gravity->getValue();
#else
	_r.ps.pmove.gravity = g_gravity->getValue();
#endif
	pm.s = _r.ps.pmove; 

	pm.s.origin = _s.origin;
	pm.s.velocity_linear = _s.velocity_linear;
	pm.s.velocity_angular = _s.velocity_angular;

	if(memcmp(&_old_pmove, &pm.s, sizeof(pm.s)))
	{
		//FIXME
		pm.snapinitial = true;
//s		trap_Com_DPrintf("g_player_c::clientThink: pmove changed!\n");
	}

	pm.cmd = cmd;

	pm.rayTrace = NULL;//G_RayTrace;
	pm.boxTrace = PM_Trace;	// adds default parms
	pm.pointContents = G_PointContents;
	
//	trap_Com_DPrintf("G_ClientThink: performing pmove ...\n");

	// perform a pmove
	BG_PMove(&pm); 
	
//	trap_Com_DPrintf("G_ClientThink: performed pmove\n");

	// save results of pmove
	_r.ps.pmove = pm.s;
	_old_pmove = pm.s;

	// update entity network state
 	_s.origin = pm.s.origin;
 	_s.velocity_linear = pm.s.velocity_linear;
 	_s.velocity_angular = pm.s.velocity_angular;
	
	_r.bbox = pm.bbox;

	_resp.cmd_angles = cmd.angles;

	if(_groundentity && !pm.groundentity && (pm.cmd.upmove >= 10) && (pm.waterlevel == 0))
	{
		//trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex("*jump1.wav"), 1, ATTN_NORM, 0);
	}

	_v_height = pm.viewheight;
	_waterlevel = pm.waterlevel;
	_watertype = pm.watertype;
	_groundentity = (g_entity_c*)pm.groundentity;
	
	if(pm.groundentity)
		_groundentity_linkcount = ((g_entity_c*)pm.groundentity)->_r.linkcount;

	if(_deadflag)
	{
		_r.ps.view_angles[ROLL] = 40;
		_r.ps.view_angles[PITCH] = -15;
		_r.ps.view_angles[YAW] = _killer_yaw;
	}
	else
	{
		setViewAngles(pm.viewangles);
		_r.ps.view_angles = pm.viewangles;
	}

	link();

	if(_movetype != MOVETYPE_NOCLIP)
		touchTriggers();

	// touch other objects
	for(uint_t i=0; i<pm.touchents.size(); i++)
	{
		g_entity_c* other = (g_entity_c*) pm.touchents[i];
		
		uint_t j;
		for(j=0; j<i; j++)
			if(pm.touchents[j] == other)
				break;
		if(j != i)
			continue;	// duplicated
				
		other->touch(this, NULL, NULL);
	}

	_buttons_old = _buttons;
	_buttons = cmd.buttons;
	_buttons_latched |= _buttons & ~_buttons_old;
	
	
	// set movement flags
	_anim_moveflags = 0;

	if(cmd.forwardmove < -1)
		_anim_moveflags |= ANIMMOVE_BACK;
	else if (cmd.forwardmove > 1)
		_anim_moveflags |= ANIMMOVE_FRONT;

	if(cmd.sidemove < -1)
		_anim_moveflags |= ANIMMOVE_LEFT;
	else if(cmd.sidemove > 1)
		_anim_moveflags |= ANIMMOVE_RIGHT;

	if(cmd.buttons & BUTTON_WALK)
		_anim_moveflags |= ANIMMOVE_WALK;

	if(_r.ps.pmove.pm_flags & PMF_DUCKED)
		_anim_moveflags |= ANIMMOVE_DUCK;

	
	// fire weapon from final position if needed
	if(_buttons_latched & BUTTON_ATTACK  || _buttons_latched & BUTTON_ATTACK2)
	{
		if(_resp.spectator)
		{
			// do nothing
		}
		else if(!_weapon_thunk)
		{
			// shoot !
			_weapon_thunk = true;
			thinkWeapon();
		}
	}

	if(_resp.spectator)
	{
		if(cmd.upmove >= 10)
		{
			if(!(_r.ps.pmove.pm_flags & PMF_JUMP_HELD))
			{
				_r.ps.pmove.pm_flags |= PMF_JUMP_HELD;
			}
		}
		else
			_r.ps.pmove.pm_flags &= ~PMF_JUMP_HELD;
	}
}



void	g_player_c::lookAtKiller(g_entity_c *inflictor, g_entity_c *attacker)
{
#if 0
	vec3_c		dir;

	if(attacker && attacker != g_world && attacker != this)
	{
		dir = attacker->_s.origin - _s.origin;
	}
	else if(inflictor && inflictor != g_world && inflictor != this)
	{
		dir = inflictor->_s.origin - _s.origin, dir;
	}
	else
	{
		_killer_yaw = _s.angles[YAW];
		return;
	}

	if(dir[0])
		_killer_yaw = 180/M_PI*atan2(dir[1], dir[0]);
	else
	{
		_killer_yaw = 0;
		
		if(dir[1] > 0)
			_killer_yaw = 90;
			
		else if (dir[1] < 0)
			_killer_yaw = -90;
	}
	
	if(_killer_yaw < 0)
		_killer_yaw += 360;
#endif
}

bool	g_player_c::isFemale()
{
	const char		*info;

	//if(!getClient())		// <-- bad something went really wrong
	//	return false;

	info = _pers.userinfo.getValueForKey("gender");
	
	if(info[0] == 'f' || info[0] == 'F')
		return true;
	else
		return false;
}

bool	g_player_c::isNeutral()
{
	const char		*info;

	//if(!getClient())
	//	return false;

	info = _pers.userinfo.getValueForKey("gender");
	
	if(info[0] != 'f' && info[0] != 'F' && info[0] != 'm' && info[0] != 'M')
		return true;
	else
		return false;
}


void	g_player_c::clientObituary(g_entity_c *inflictor, g_entity_c *attacker)
{
	int			mod;
	char		*message;
	char		*message2;
	bool	ff;

	if(coop->getInteger() && attacker->_r.isclient)
		meansOfDeath |= MOD_FRIENDLY_FIRE;

	if(deathmatch->getInteger() || coop->getInteger())
	{
		ff = meansOfDeath & MOD_FRIENDLY_FIRE;
		mod = meansOfDeath & ~MOD_FRIENDLY_FIRE;
		message = NULL;
		message2 = "";

		switch (mod)
		{
		case MOD_SUICIDE:
			message = "suicides";
			break;
		case MOD_FALLING:
			message = "cratered";
			break;
		case MOD_CRUSH:
			message = "was squished";
			break;
		case MOD_WATER:
			message = "sank like a rock";
			break;
		case MOD_SLIME:
			message = "melted";
			break;
		case MOD_LAVA:
			message = "does a back flip into the lava";
			break;
		case MOD_EXPLOSIVE:
		case MOD_BARREL:
			message = "blew up";
			break;
		case MOD_EXIT:
			message = "found a way out";
			break;
		case MOD_TARGET_LASER:
			message = "saw the light";
			break;
		case MOD_TARGET_BLASTER:
			message = "got blasted";
			break;
		case MOD_BOMB:
		case MOD_SPLASH:
		case MOD_TRIGGER_HURT:
			message = "was in the wrong place";
			break;
		}
		
		if (attacker == this)
		{
			switch (mod)
			{
			case MOD_HELD_GRENADE:
				message = "tried to put the pin back in";
				break;
			case MOD_HG_SPLASH:
			case MOD_G_SPLASH:
				if (isNeutral())
					message = "tripped on its own grenade";
				else if (isFemale())
					message = "tripped on her own grenade";
				else
					message = "tripped on his own grenade";
				break;
			case MOD_R_SPLASH:
				if (isNeutral())
					message = "blew itself up";
				else if (isFemale())
					message = "blew herself up";
				else
					message = "blew himself up";
				break;
			case MOD_BFG_BLAST:
				message = "should have used a smaller gun";
				break;
			default:
				if (isNeutral())
					message = "killed itself";
				else if (isFemale())
					message = "killed herself";
				else
					message = "killed himself";
				break;
			}
		}
		
		if(message)
		{
			trap_SV_BPrintf (PRINT_MEDIUM, "%s %s.\n", _pers.netname, message);
			
			if(deathmatch->getInteger())
				_resp.score--;
				
			_enemy = NULL;
			return;
		}

		_enemy = attacker;
		
		if(attacker && attacker->_r.isclient)
		{
			switch (mod)
			{
			case MOD_BLASTER:
				message = "was blasted by";
				break;
			case MOD_SHOTGUN:
				message = "was gunned down by";
				break;
			case MOD_SSHOTGUN:
				message = "was blown away by";
				message2 = "'s super shotgun";
				break;
			case MOD_MACHINEGUN:
				message = "was machinegunned by";
				break;
			case MOD_CHAINGUN:
				message = "was cut in half by";
				message2 = "'s chaingun";
				break;
			case MOD_GRENADE:
				message = "was popped by";
				message2 = "'s grenade";
				break;
			case MOD_G_SPLASH:
				message = "was shredded by";
				message2 = "'s shrapnel";
				break;
			case MOD_ROCKET:
				message = "ate";
				message2 = "'s rocket";
				break;
			case MOD_R_SPLASH:
				message = "almost dodged";
				message2 = "'s rocket";
				break;
			case MOD_HYPERBLASTER:
				message = "was melted by";
				message2 = "'s hyperblaster";
				break;
			case MOD_RAILGUN:
				message = "was railed by";
				break;
			case MOD_BFG_LASER:
				message = "saw the pretty lights from";
				message2 = "'s BFG";
				break;
			case MOD_BFG_BLAST:
				message = "was disintegrated by";
				message2 = "'s BFG blast";
				break;
			case MOD_BFG_EFFECT:
				message = "couldn't hide from";
				message2 = "'s BFG";
				break;
			case MOD_HANDGRENADE:
				message = "caught";
				message2 = "'s handgrenade";
				break;
			case MOD_HG_SPLASH:
				message = "didn't see";
				message2 = "'s handgrenade";
				break;
			case MOD_HELD_GRENADE:
				message = "feels";
				message2 = "'s pain";
				break;
			case MOD_TELEFRAG:
				message = "tried to invade";
				message2 = "'s personal space";
				break;
			}
			
			if(message)
			{
				trap_SV_BPrintf (PRINT_MEDIUM,"%s %s %s%s\n", _pers.netname, message, ((g_player_c*)attacker)->_pers.netname, message2);
				
				if (deathmatch->getInteger())
				{
					if (ff)
						((g_player_c*)attacker)->_resp.score--;
					else
						((g_player_c*)attacker)->_resp.score++;
				}
				return;
			}
		}
	}

	trap_SV_BPrintf (PRINT_MEDIUM,"%s died.\n", _pers.netname);
	
	if (deathmatch->getInteger())
		_resp.score--;
}



/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void	G_SaveClientData()
{
	for(int i=1; i<game.maxclients+1; i++)
	{
		g_player_c *player = (g_player_c*)g_entities[i];
		
		if(!player->_r.inuse)
			continue;
		
		player->saveClientData();
	}
}



/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	g_player_c::rangeFromSpot(g_entity_c *spot)
{
	g_entity_c*	player;
	float		bestplayerdistance;
	vec3_c		v;
	int		n;
	float		playerdistance;


	bestplayerdistance = 9999999;

	for(n=1; n <= maxclients->getInteger(); n++)
	{
		player = (g_entity_c*)g_entities[n];

		if(!player->_r.inuse)
			continue;

		if(player->_health <= 0)
			continue;

		v = spot->_s.origin - player->_s.origin;
		playerdistance = v.length();

		if(playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
g_entity_c*	g_player_c::selectRandomDeathmatchSpawnPoint()
{
	g_entity_c	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float		range, range1, range2;


	range1 = range2 = 99999;
	spot = spot1 = spot2 = NULL;

	while((spot = G_FindByClassName(spot, "info_player_deathmatch")) != NULL)
	{
		count++;
		range = rangeFromSpot(spot);
		if(range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if(range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if(!count)
	{
		trap_Com_Printf("g_player_c::selectRandomDeathmatchSpawnPoint: no spawn points\n");
		return NULL;
	}

	if(count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_FindByClassName(spot, "info_player_deathmatch");
		
		if(spot == spot1 || spot == spot2)
			selection++;
			
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint
================
*/
g_entity_c*	g_player_c::selectFarthestDeathmatchSpawnPoint()
{
	g_entity_c	*spot, *bestspot;
	float		bestdistance, bestplayerdistance;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while((spot = G_FindByClassName(spot, "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = rangeFromSpot(spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_FindByClassName(NULL, "info_player_deathmatch");

	return spot;
}

g_entity_c*	g_player_c::selectDeathmatchSpawnPoint()
{
	//if(dmflags->getInteger() & DF_SPAWN_FARTHEST)
	//	return selectFarthestDeathmatchSpawnPoint();
	//else
		return selectRandomDeathmatchSpawnPoint();
}


g_entity_c*	g_player_c::selectCoopSpawnPoint()
{
	int		index;
	g_entity_c	*spot = NULL;
	std::string	target;

	index = G_GetNumForEntity(this);

	// player 0 starts in normal player spawn point
	if(!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while(1)
	{
		spot = G_FindByClassName(spot, "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->_targetname;
		
		if(!target.length())
			target = "";
			
		if(X_stricmp(game.spawnpoint.c_str(), target.c_str()) == 0 )
		{	
			// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}

	return spot;
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
void	g_player_c::selectSpawnPoint(vec3_c &origin, vec3_c &angles)
{
	g_entity_c	*spot = NULL;

	if(deathmatch->getInteger())
		spot = selectDeathmatchSpawnPoint();
		
	else if(coop->getInteger())
		spot = selectCoopSpawnPoint();

	// find a single player start spot
	if(!spot)
	{
		while((spot = G_FindByClassName(spot, "info_player_start")) != NULL)
		{
			if(game.spawnpoint.empty() && spot->_targetname.empty())
				break;

			if(game.spawnpoint.empty() || spot->_targetname.empty())
				continue;

			if(X_stricmp(game.spawnpoint.c_str(), spot->_targetname.c_str()) == 0)
				break;
		}

		if(!spot)
		{
			if(game.spawnpoint.empty())
			{	
				// there wasn't a spawnpoint without a target, so use any
				spot = G_FindByClassName(spot, "info_player_start");
			}
			if(!spot)
				trap_Com_Error(ERR_DROP, "g_player_c::selectSpawnPoint: Couldn't find spawn point %s\n", game.spawnpoint.c_str());
		}
	}

	origin = spot->_s.origin;
	angles = spot->_angles;
}


void	g_player_c::clearAllButPersistant()
{
	_resp.clear();

	_buttons		= 0;
	_buttons_old		= 0;
	_buttons_latched	= 0;

	_v_angles.clear();		// aiming direction
	_v_forward.clear();
	_v_right.clear();
	_v_up.clear();
	_v_quat.identity();
	
	_v_height		= 0;		
	
	_xyspeed		= 0;
	
	_bob_time		= 0;
	_bob_move		= 0;
	_bob_cycle		= 0;		
	_bob_fracsin		= 0;		
	
	_old_pmove.clear();
	
	_showscores		= false;
	_showinventory		= false;
	_showhelp		= false;
	_showhelpicon		= false;

	_ammo_index		= 0;

	_anim_current		= 0;
	_anim_time		= 0;

	_anim_priority		= 0;
	
	_anim_duck		= false;
	_anim_run		= false;
	_anim_swim		= false;
	
	_anim_jump		= false;
	_anim_jump_prestep	= false;
	_anim_jump_style	= false;
	
	_anim_moveflags		= 0;
	_anim_moveflags_old	= 0;
	
	_anim_lastsent		= 0;

	_time_next_drown	= 0;
	_time_pickup_msg	= 0;
	_time_respawn		= 0;
	_time_jumppad		= 0;
	_time_air_finished	= 0;
	_time_fall		= 0;

	_flood_locktill		= 0;
	memset(_flood_when, 0, sizeof(_flood_when));
	_flood_whenhead		= 0;	

	_damage_armor		= 0;
	_damage_parmor		= 0;
	_damage_blood		= 0;
	_damage_knockback	= 0;
	_damage_from.clear();
	
	_killer_yaw		= 0;
	
	_newweapon		= NULL;
	_weapon_state		= WEAPON_ACTIVATING;
	_weapon_update		= 0;
	_weapon_thunk		= false;
	_weapon_fired		= false;
			
	_kick_angles.clear();
	_kick_origin.clear();
	_v_dmg_roll		= 0;
	_v_dmg_pitch		= 0;
	_v_dmg_time		= 0;
	_fall_value		= 0;
	_damage_alpha		= 0;
	_bonus_alpha		= 0;
	_damage_blend.clear();
	
	_oldviewangles.clear();
	_oldvelocity.clear();
	
	_old_waterlevel		= 0;
	_breather_sound		= 0;

	_machinegun_shots	= 0;
	
	_quad_framenum		= 0;
	_invincible_framenum	= 0;
	_breather_framenum	= 0;
	_enviro_framenum	= 0;
	
	_grenade_blew_up	= 0;
	_grenade_time		= 0;
	_weapon_sound		= 0;
}

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void	g_player_c::initClientPersistant()
{
	_pers.clear();

//	g_item_c *item = G_FindItem("blaster");

//	_pers.selected_item = G_GetNumForItem(item);
//	_pers.inventory[_pers.selected_item] = 1;

//	_pers.weapon = (g_item_weapon_c*)item;

	_pers.health		= 100;
	_pers.max_health	= 100;

	_pers.max_bullets	= 200;
	_pers.max_shells	= 100;
	_pers.max_rockets	= 10;
	_pers.max_grenades	= 50;
	_pers.max_cells		= 200;
	_pers.max_slugs		= 50;

	_pers.connected = true;
}


void	g_player_c::initClientResp()
{
	_resp.clear();
	
	_resp.enterframe = level.framenum;
	_resp.coop_respawn = _pers;
}


void	g_player_c::fetchClientEntData()
{
	_health = _pers.health;
	_max_health = _pers.max_health;
	_flags |= _pers.saved_flags;
	
	if(coop->getInteger())
		_resp.score = _pers.score;
}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void	g_player_c::putClientInServer()
{
	vec3_c	spawn_origin;
	vec3_c	spawn_angles;

	client_respawn_t	resp;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	selectSpawnPoint(spawn_origin, spawn_angles);	

	// deathmatch wipes most client data every spawn
	if(deathmatch->getInteger())
	{
		info_c	userinfo = _pers.userinfo;

		resp = _resp;
		
		initClientPersistant();
		clientUserinfoChanged(userinfo); 
	}
	else if(coop->getInteger())
	{
		info_c	userinfo = _pers.userinfo;

		resp = _resp;
		
		_pers = resp.coop_respawn;
		clientUserinfoChanged(userinfo);
		
		if(resp.score > _pers.score)
			_pers.score = resp.score;
	}
	else
	{
		resp.clear();
	}

	// clear everything but the persistant data
	clearAllButPersistant();
	
	if(_pers.health <= 0)
		initClientPersistant();
		
	_resp = resp;

	// copy some data from the client to the entity
	fetchClientEntData();

	// clear entity values
	_r.inuse = true;
	_r.solid = SOLID_BBOX;
	_r.clipmask = MASK_PLAYERSOLID;
	_r.bbox._mins.set(-15, -15, 0);
	_r.bbox._maxs.set(15, 15, 70);
	_r.svflags &= ~SVF_CORPSE;

	_groundentity = NULL;
	_takedamage = DAMAGE_AIM;
	_movetype = MOVETYPE_WALK;
	_v_height = VIEWHEIGHT_DEFAULT;
	_classname = "player";
	_mass = 200;
	_deadflag = DEAD_NO;
	_time_air_finished = level.time + 12000;
	//_model = "players/male/tris.md2";
	_waterlevel = 0;
	_watertype = 0;
	_flags &= ~FL_NO_KNOCKBACK;

	// clear playerstate values
	_r.ps.clear();

	if(deathmatch->getInteger() && (dmflags->getInteger() & DF_FIXED_FOV))
	{
		_r.ps.fov = 90;
	}
	else
	{
		_r.ps.fov = atoi(_pers.userinfo.getValueForKey("fov"));
		
		if(_r.ps.fov < 1)
			_r.ps.fov = 90;
			
		else if(_r.ps.fov > 160)
			_r.ps.fov = 160;
	}

//	_r.ps.gun_model_index = trap_SV_ModelIndex(_pers.weapon->getViewModel());
//	_r.ps.gun_anim_index = trap_SV_AnimationIndex(_pers.weapon->getActivateAnimationName());

	// clear entity state values
//	_s.clear();
//	_s.type = ET_PLAYER;
//	_s.index_model = 255;		// will use the skin specified model	
	_s.index_model = trap_SV_ModelIndex("models/players/marine/mpplayer.md5mesh");
	_s.index_animation = trap_SV_AnimationIndex("models/players/marine/fists_idle.md5anim");
	_s.index_light = 0;
	_s.frame = 0;
	_s.effects = 0;
	
	_anim_priority = ANIM_BASIC;
	_anim_current = PLAYER_ANIM_FISTS_IDLE;
//	_anim_time = 0;
	_anim_lastsent = -1; 

	// set origin
	_s.origin = spawn_origin;
	_s.origin[2] += 1.0;	// make sure off ground
	
//	_r.ps.pmove.origin = spawn_origin;

	// set angles
	spawn_angles[PITCH] = 0;
	spawn_angles[ROLL] = 0;

	_s.quat.fromAngles(spawn_angles);
	_r.ps.view_angles = spawn_angles;
	setViewAngles(spawn_angles);

	// set the delta angle
//	_r.ps.pmove.delta_angles = spawn_angles - _resp.cmd_angles;

	// check if spawning a spectator
	if(_pers.spectator) 
	{
		_resp.spectator = true;

		_movetype = MOVETYPE_NOCLIP;
		_r.solid = SOLID_NOT;
		_r.svflags |= SVF_NOCLIENT;
		_r.ps.gun_model_index = 0;
		return;
	}
	else
	{		
		_resp.spectator = false;
	}

	unlink();

	if(!killBox())
	{
		// could't spawn in?
		trap_Com_Printf("g_player_c::putClientInServer: could not spawn '%s' at %s\n", _pers.netname, _s.origin.toString());
	}

	trap_Com_Printf("g_player_c::putClientInServer: linking '%s' into world\n", _pers.netname);
	link();

	// force the current weapon up
	_newweapon = _pers.weapon;
	changeWeapon();
}


void	g_player_c::respawn()
{
	if(deathmatch->getInteger() || coop->getInteger())
	{
		// spectator's don't leave bodies
		//if (self->movetype != MOVETYPE_NOCLIP)
		//	CopyToBodyQue (self);
			
		_r.svflags &= ~SVF_NOCLIENT;
		
		putClientInServer();

		// add a teleportation effect
		//_s.event = EV_TELEPORT;
		
		//TODO add teleport effect via temp entity

		// hold in place briefly
		_r.ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		_r.ps.pmove.pm_time = 14;

		_time_respawn = level.time;
		return;
	}
}

/* 
 * only called when pers.spectator changes
 * note that resp.spectator should be the opposite of pers.spectator here
 */
void	g_player_c::respawnAsSpectator()
{
	int i, numspec;

	// if the user wants to become a spectator, make sure he doesn't
	// exceed max_spectators

	if(_pers.spectator)
	{
		const char *value = _pers.userinfo.getValueForKey("spectator");
		
		if(!X_strequal(spectator_password->getString(), "") && !X_strequal(spectator_password->getString(), "none") && !X_strequal(spectator_password->getString(), value))
		{
			trap_SV_CPrintf(this, PRINT_HIGH, "Spectator password incorrect.\n");
			_pers.spectator = false;
			trap_SV_WriteBits(SVC_STUFFTEXT, svc_bitcount);
			trap_SV_WriteString("spectator 0\n");
			trap_SV_Unicast(this, true);
			return;
		}

		// count spectators
		for(i=1, numspec=0; i <= maxclients->getInteger(); i++)
			if(g_entities[i]->_r.inuse && ((g_player_c*)g_entities[i])->_pers.spectator)
				numspec++;

		if(numspec >= maxspectators->getInteger()) 
		{
			trap_SV_CPrintf(this, PRINT_HIGH, "Server spectator limit is full.");
			_pers.spectator = false;
			trap_SV_WriteBits(SVC_STUFFTEXT, svc_bitcount);
			trap_SV_WriteString("spectator 0\n");
			trap_SV_Unicast(this, true);
			return;
		}
	}
	else
	{
		// he was a spectator and wants to join the game
		// he must have the right password
		const char *value = _pers.userinfo.getValueForKey("password");
		
		if(!X_strequal(password->getString(), "") && !X_strequal(password->getString(), "none") && !X_strequal(password->getString(), value))
		{
			trap_SV_CPrintf(this, PRINT_HIGH, "Password incorrect.\n");
			_pers.spectator = true;
			trap_SV_WriteBits(SVC_STUFFTEXT, svc_bitcount);
			trap_SV_WriteString("spectator 1\n");
			trap_SV_Unicast(this, true);
			return;
		}
	}

	// clear client on respawn
	_resp.score = _pers.score = 0;

	_r.svflags &= ~SVF_NOCLIENT;
	putClientInServer();

	// add a teleportation effect
	if(!_pers.spectator)
	{
		// send effect
		trap_SV_WriteBits(SVC_MUZZLEFLASH, svc_bitcount);
		trap_SV_WriteShort(_s.getNumber());	// - g_entities);
		trap_SV_WriteByte(MZ_LOGIN);
		trap_SV_Multicast(_s.origin, MULTICAST_PVS);

		// hold in place briefly
		_r.ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		_r.ps.pmove.pm_time = 14;
	}

	_time_respawn = level.time;

	if(_pers.spectator) 
		trap_SV_BPrintf(PRINT_HIGH, "%s has moved to the sidelines\n", _pers.netname);
	else
		trap_SV_BPrintf(PRINT_HIGH, "%s joined the game\n", _pers.netname);
}



/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void	g_player_c::beginServerFrame()
{
	int		buttonMask;
	
	//trap_Com_Printf("g_player_c::beginServerFrame:\n");

	if(level.intermission_time)
		return;

	if(deathmatch->getInteger() && _pers.spectator != _resp.spectator && (level.time - _time_respawn) >= 5)
	{
		respawnAsSpectator();
		return;
	}

	// run weapon animations if it hasn't been done by a ucmd_t
	if(!_weapon_thunk && !_resp.spectator)
		thinkWeapon();
	else
		_weapon_thunk = false;

	if(_deadflag)
	{
		// wait for any button just going down
		if(level.time > _time_respawn)
		{
			// in deathmatch, only wait for attack button
			if(deathmatch->getInteger())
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

			if((_buttons_latched & buttonMask) || (deathmatch->getInteger() && (dmflags->getInteger() & DF_FORCE_RESPAWN)))
			{
				respawn();
				_buttons_latched = 0;
			}
		}
		return;
	}

	_buttons_latched = 0;
}


/*
=================
ClientEndServerFrame

Called for each player at the end of the server frame
and right after spawning
=================
*/
void	g_player_c::endServerFrame()
{
	float	bobtime;
	
	// If the origin or velocity have changed since ClientThink(),
	// update the pmove values.  This will happen when the client
	// is pushed by a bmodel or kicked by an explosion.
	// 
	// If it wasn't updated here, the view position would lag a frame
	// behind the body position when pushed -- "sinking into plats"
	
//  	_r.ps.pmove.origin = _body->getPosition();
//  	_r.ps.pmove.velocity_linear = _body->getLinearVel();
//  	_r.ps.pmove.velocity_angular = _body->getAngularVel();
	
	_r.ps.pmove.origin = _s.origin;
	_r.ps.pmove.velocity_linear = _s.velocity_linear;
	_r.ps.pmove.velocity_angular = _s.velocity_angular;

	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	if(level.intermission_time)
	{
		// FIXME: add view drifting here?
		_r.ps.blend[3] = 0;
		_r.ps.fov = 90;
		updateStats();
		return;
	}

	Angles_ToVectors(_v_angles, _v_forward, _v_right, _v_up);

	// burn from lava, etc
	updateWorldEffects();


	// set model angles from view angles so other things in
	// the world can tell which direction you are looking
	if(_v_angles[PITCH] > 180)
		_angles[PITCH] = (-360 + _v_angles[PITCH])/3;
	else
		_angles[PITCH] = _v_angles[PITCH]/3;
		
	_angles[YAW] = _v_angles[YAW];
	_angles[ROLL] = 0;
	_angles[ROLL] = calcRoll(_angles, _s.velocity_linear)*4;
	
	_s.quat.fromAngles(_angles);


	// calculate speed and cycle to be used for
	// all cyclic walking effects
	_xyspeed = X_sqrt(_s.velocity_linear[0]*_s.velocity_linear[0] + _s.velocity_linear[1]*_s.velocity_linear[1]);

	//if(_xyspeed < 5)
	{
		_bob_move = 0;
		_bob_time = 0;	// start at beginning of cycle again
	}
	/*
	else if(_groundentity)
	{	
		// so bobbing only cycles when on ground
		if(_xyspeed > 210)
			_bob_move = 0.25;
			
		else if(_xyspeed > 100)
			_bob_move = 0.125;
			
		else
			_bob_move = 0.0625;
	}
	*/
	
	bobtime = (_bob_time += _bob_move);

	if(_r.ps.pmove.pm_flags & PMF_DUCKED)
		bobtime *= 4;

	_bob_cycle = (int)bobtime;
	_bob_fracsin = fabs(sin(bobtime*M_PI));

	// detect hitting the floor
	updateFallingDamage();

	// apply all the damage taken this frame
	updateDamageFeedback();

	// determine the view offsets
	calcViewOffset();

	// determine the gun offsets
	calcGunOffset();

	// determine the full screen color blend
	// must be after viewoffset, so eye contents can be
	// accurately determined
	// FIXME: with client prediction, the contents
	// should be determined by the client
	calcBlend();

	// chase cam stuff
	if(_resp.spectator)
		updateSpectatorStats();
	else
		updateStats();
		
	checkChaseStats();

	updateClientEvent();

	updateClientEffects();

	updateClientSound();

	updateClientFrame();

	_oldvelocity = _s.velocity_linear;
	_oldviewangles = _r.ps.view_angles;

	// clear weapon kicks
	_kick_origin.clear();
	_kick_angles.clear();

	// if the scoreboard is up, update it
	if(_showscores && !(level.framenum & 31))
	{
		deathmatchScoreBoardMessage();
		trap_SV_Unicast(this, false);
	}
	
	//if(!ent->client)
	//	trap_Com_Error(ERR_DROP, "G_ClientEndServerFrame: NULL ent->getClient()");
		
	//trap_Com_Printf("G_ClientEndServerFrame: %s\n", ent->_pers.netname);
	
	
}

void	g_player_c::resetHealth()
{
	if(_health > _pers.max_health)
			_health = _pers.max_health;
}

void	g_player_c::saveClientData()
{
	_pers.health = _health;
	_pers.max_health = _max_health;
	_pers.saved_flags = (_flags & (FL_ULTRAMANMODE|FL_NOTARGET|FL_POWER_ARMOR));
		
	if(coop->getInteger())
		_pers.score = _resp.score;
}

void	g_player_c::updateStats()
{
	//g_item_c		*item;
	//int			index
	//int			power_armor_type;
	//int cells = 0;

	
	//
	// health
	//
	_r.ps.stats[STAT_HEALTH_ICON] = trap_SV_ShaderIndex("gfx/hud/healthicon");
	_r.ps.stats[STAT_HEALTH] = _health;
	

	//
	// ammo
	//
	if(!_ammo_index  || !_pers.inventory[_ammo_index])
	{
		_r.ps.stats[STAT_AMMO_ICON] = trap_SV_ShaderIndex("gfx/hud/ammoicon");
		_r.ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		//item = G_GetItemByNum(_ammo_index);
		_r.ps.stats[STAT_AMMO_ICON] = trap_SV_ShaderIndex("gfx/hud/ammoicon"); //trap_SV_ShaderIndex (item->getIcon());
		_r.ps.stats[STAT_AMMO] = _pers.inventory[_ammo_index];
	}
	
	
	//
	// armor
	//
	/*
	power_armor_type = PowerArmorType(this);
	if(power_armor_type)
	{
		cells = _pers.inventory[G_GetNumForItem(G_FindItem ("cells"))];
		
		if (cells == 0)
		{	// ran out of cells for power armor
			_flags &= ~FL_POWER_ARMOR;
			trap_SV_StartSound(NULL, this, CHAN_ITEM, trap_SV_SoundIndex("misc/power2.wav"), 1, ATTN_NORM, 0);
			power_armor_type = 0;;
		}
	}
	*/

	/*
	index = ArmorIndex(this);
	if(power_armor_type && (!index || (level.framenum & 8) ) )
	{	
		// flash between power armor and other armor icon
		_r.ps.stats[STAT_ARMOR_ICON] = trap_SV_ShaderIndex("textures/pics/i_powershield");
		_r.ps.stats[STAT_ARMOR] = cells;
	}
	else if(index)
	{
		item = G_GetItemByNum(index);
		_r.ps.stats[STAT_ARMOR_ICON] = trap_SV_ShaderIndex("gfx/hud/armoricon"); //trap_SV_ShaderIndex (item->getIcon());
		_r.ps.stats[STAT_ARMOR] = _pers.inventory[index];
	}
	else
	*/
	{
		_r.ps.stats[STAT_ARMOR_ICON] = trap_SV_ShaderIndex("gfx/hud/armoricon");
		_r.ps.stats[STAT_ARMOR] = 0;
	}

	
	//
	// pickup message
	//
	if(level.time > _time_pickup_msg)
	{
		_r.ps.stats[STAT_PICKUP_ICON] = 0;
		_r.ps.stats[STAT_PICKUP_STRING] = 0;
	}


	//
	// timers
	//
	if (_quad_framenum > level.framenum)
	{
		_r.ps.stats[STAT_TIMER_ICON] = trap_SV_ShaderIndex("textures/pics/p_quad");
		_r.ps.stats[STAT_TIMER] = (short)((_quad_framenum - level.framenum)/10);
	}
	else if (_invincible_framenum > level.framenum)
	{
		_r.ps.stats[STAT_TIMER_ICON] = trap_SV_ShaderIndex("textures/pics/p_invulnerability");
		_r.ps.stats[STAT_TIMER] = (short)((_invincible_framenum - level.framenum)/10);
	}
	else if (_enviro_framenum > level.framenum)
	{
		_r.ps.stats[STAT_TIMER_ICON] = trap_SV_ShaderIndex("textures/pics/p_envirosuit");
		_r.ps.stats[STAT_TIMER] = (short)((_enviro_framenum - level.framenum)/10);
	}
	else if (_breather_framenum > level.framenum)
	{
		_r.ps.stats[STAT_TIMER_ICON] = trap_SV_ShaderIndex("textures/pics/p_rebreather");
		_r.ps.stats[STAT_TIMER] = (short)((_breather_framenum - level.framenum)/10);
	}
	else
	{
		_r.ps.stats[STAT_TIMER_ICON] = 0;
		_r.ps.stats[STAT_TIMER] = 0;
	}


	//
	// selected item
	//
	if (_pers.selected_item == -1)
		_r.ps.stats[STAT_SELECTED_ICON] = 0;
	//else
	//	_r.ps.stats[STAT_SELECTED_ICON] = trap_SV_ShaderIndex(g_items[_pers.selected_item]->getIcon());

	_r.ps.stats[STAT_SELECTED_ITEM] = _pers.selected_item;


	//
	// layouts
	//
	_r.ps.stats[STAT_LAYOUTS] = 0;

	if(deathmatch->getInteger())
	{
		if(_pers.health <= 0 || level.intermission_time || _showscores)
			_r.ps.stats[STAT_LAYOUTS] |= 1;
			
		if(_showinventory && _pers.health > 0)
			_r.ps.stats[STAT_LAYOUTS] |= 2;
	}
	else
	{
		if(_showscores || _showhelp)
			_r.ps.stats[STAT_LAYOUTS] |= 1;
			
		if(_showinventory && _pers.health > 0)
			_r.ps.stats[STAT_LAYOUTS] |= 2;
	}


	//
	// frags
	//
	_r.ps.stats[STAT_FRAGS_ICON] = trap_SV_ShaderIndex("gfx/hud/fragicon");
	_r.ps.stats[STAT_FRAGS] = _resp.score;


	//
	// help icon / current weapon if not shown
	//
	//if((_pers.hand == CENTER_HANDED || _r.ps.fov > 91) && _pers.weapon)
	//	_r.ps.stats[STAT_HELPICON] = trap_SV_ShaderIndex(_pers.weapon->getIcon());
	//else
		_r.ps.stats[STAT_HELPICON] = 0;

	_r.ps.stats[STAT_SPECTATOR] = 0;
	
	
	//
	// main hud status bar
	//
	_r.ps.stats[STAT_HUDBAR] = trap_SV_ShaderIndex("gfx/hud/hud_bar");
	
		
	//
	// weapon windows
	//
	//_r.ps.stats[STAT_WEAPON_WINDOW0_ICON] = trap_SV_ShaderIndex("gfx/hud/weapn_win.tga");
	//_r.ps.stats[STAT_WEAPON_WINDOW0_AMMO] = _pers.inventory[G_GetNumForItem(G_FindItem("rockets"))];
	
	//if(_pers.inventory[G_GetNumForItem(G_FindItem("rocket launcher"))])
	//{
	//	item = G_FindItem("blaster");
		
	//	_r.ps.stats[STAT_WEAPON_WINDOW0_WEAPON] = trap_SV_ModelIndex(item->getWorldModel());
	//}
	//else
	//	_r.ps.stats[STAT_WEAPON_WINDOW0_WEAPON] = 0;
		
	//_r.ps.stats[STAT_WEAPON_WINDOW1_ICON] = trap_SV_ShaderIndex ("gfx/statusbar/weapn_win.tga");
	//_r.ps.stats[STAT_WEAPON_WINDOW1] = _pers.inventory[G_GetNumForItem(G_FindItem("cells"))];
	
	//_r.ps.stats[STAT_WEAPON_WINDOW2_ICON] = trap_SV_ShaderIndex ("gfx/statusbar/weapn_win.tga");
	//_r.ps.stats[STAT_WEAPON_WINDOW2] = _pers.inventory[G_GetNumForItem(G_FindItem("cells"))];
	
	//_r.ps.stats[STAT_WEAPON_WINDOW3_ICON] = trap_SV_ShaderIndex ("gfx/statusbar/weapn_win.tga");
	//_r.ps.stats[STAT_WEAPON_WINDOW3] = _pers.inventory[G_GetNumForItem(G_FindItem("cells"))];
	
	//_r.ps.stats[STAT_WEAPON_WINDOW4_ICON] = trap_SV_ShaderIndex ("gfx/statusbar/weapn_win.tga");
	//_r.ps.stats[STAT_WEAPON_WINDOW4] = _pers.inventory[G_GetNumForItem(G_FindItem("cells"))];
	
	//_r.ps.stats[STAT_WEAPON_WINDOW5_ICON] = trap_SV_ShaderIndex ("gfx/statusbar/weapn_win.tga");
	//_r.ps.stats[STAT_WEAPON_WINDOW5] = _pers.inventory[G_GetNumForItem(G_FindItem("cells"))];
 	
	//
	// selected weapon 
	//
	/*
	{
		//item = &itemlist[_ammo_index];
		_r.ps.stats[STAT_SELECTED_WEAPON_ICON] = trap_SV_ShaderIndex ("gfx/hud/selec_weapn.tga");
		_r.ps.stats[STAT_SELECTED_WEAPON] = 0;  //TODO _weapon_index]
	}
	*/
	
}

void	g_player_c::updateSpectatorStats()
{
	updateStats();

	_r.ps.stats[STAT_SPECTATOR] = 1;

	// layouts are independant in spectator
	_r.ps.stats[STAT_LAYOUTS] = 0;
	
	if(_pers.health <= 0 || level.intermission_time || _showscores)
		_r.ps.stats[STAT_LAYOUTS] |= 1;
		
	if(_showinventory && _pers.health > 0)
		_r.ps.stats[STAT_LAYOUTS] |= 2;

	_r.ps.stats[STAT_CHASE] = 0;
}


void	g_player_c::checkChaseStats()
{
	for(int i=1; i <= maxclients->getInteger(); i++) 
	{
		g_player_c *player = (g_player_c*)g_entities[i];
		
		if(!player->_r.inuse)
			continue;
			
		memcpy(player->_r.ps.stats, _r.ps.stats, sizeof(player->_r.ps.stats));
	
		player->updateSpectatorStats();
	}
}



void	g_player_c::updateWorldEffects()
{
	bool	breather;
	bool	envirosuit;
	int			waterlevel, old_waterlevel;

	
	if(_movetype == MOVETYPE_NOCLIP)
	{
		_time_air_finished = level.time + 12000;	// don't need air
		return;
	}

	waterlevel = _waterlevel;
	old_waterlevel = _old_waterlevel;
	_old_waterlevel = waterlevel;

	breather = _breather_framenum > level.framenum;
	envirosuit = _enviro_framenum > level.framenum;

	//
	// if just entered a water volume, play a sound
	//
	if(!old_waterlevel && waterlevel)
	{
		//PlayerNoise(current_player, current_player->_s.origin, PNOISE_SELF);
		
		if(_watertype & X_CONT_LAVA)
			trap_SV_StartSound(NULL, this, CHAN_BODY, trap_SV_SoundIndex("player/lava_in.wav"), 1, ATTN_NORM, 0);
			
		else if(_watertype & X_CONT_SLIME)
			trap_SV_StartSound(NULL, this, CHAN_BODY, trap_SV_SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
			
		else if(_watertype & X_CONT_WATER)
			trap_SV_StartSound(NULL, this, CHAN_BODY, trap_SV_SoundIndex("player/watr_in.wav"), 1, ATTN_NORM, 0);
			
		_flags |= FL_INWATER;

		// clear damage_debounce, so the pain sound will play immediately
		_time_damage_debounce = level.time - 1000;
	}


	//
	// if just completely exited a water volume, play a sound
	//
	if(old_waterlevel && ! waterlevel)
	{
		//PlayerNoise(current_player, current_player->_s.origin, PNOISE_SELF);
		
		trap_SV_StartSound(NULL, this, CHAN_BODY, trap_SV_SoundIndex("player/watr_out.wav"), 1, ATTN_NORM, 0);
		_flags &= ~FL_INWATER;
	}


	//
	// check for head just going under water
	//
	if(old_waterlevel != 3 && waterlevel == 3)
	{
		trap_SV_StartSound(NULL, this, CHAN_BODY, trap_SV_SoundIndex("player/watr_un.wav"), 1, ATTN_NORM, 0);
	}


	//
	// check for head just coming out of water
	//
	if(old_waterlevel == 3 && waterlevel != 3)
	{
		if(_time_air_finished < level.time)
		{	
			// gasp for air
			trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex("player/gasp1.wav"), 1, ATTN_NORM, 0);
			
			//PlayerNoise(current_player, current_player->_s.origin, PNOISE_SELF);
		}
		else  if(_time_air_finished < level.time + 11000)
		{	
			// just break surface
			trap_SV_StartSound (NULL, this, CHAN_VOICE, trap_SV_SoundIndex("player/gasp2.wav"), 1, ATTN_NORM, 0);
		}
	}
	

	//
	// check for drowning
	//
	if(waterlevel == 3)
	{
		// breather or envirosuit give air
		if(breather || envirosuit)
		{
			_time_air_finished = level.time + 10000;

			if(((int)(_breather_framenum - level.framenum) % 25) == 0)
			{
				if(!_breather_sound)
					trap_SV_StartSound(NULL, this, CHAN_AUTO, trap_SV_SoundIndex("player/u_breath1.wav"), 1, ATTN_NORM, 0);
				else
					trap_SV_StartSound (NULL, this, CHAN_AUTO, trap_SV_SoundIndex("player/u_breath2.wav"), 1, ATTN_NORM, 0);
				
				_breather_sound ^= 1;
				
				//PlayerNoise(current_player, current_player->_s.origin, PNOISE_SELF);
				//FIXME: release a bubble?
			}
		}

		// if out of air, start drowning
		if(_time_air_finished < level.time)
		{
			// drown!
			if(_time_next_drown < level.time && _health > 0)
			{
				_time_next_drown = level.time + 1000;

				// take more damage the longer underwater
				_dmg += 2;
				
				if(_dmg > 15)
					_dmg = 15;

				// play a gurp sound instead of a normal pain sound
				if(_health <= _dmg)
					trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex("player/drown1.wav"), 1, ATTN_NORM, 0);
					
				else if (rand()&1)
					trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex("*gurp1.wav"), 1, ATTN_NORM, 0);
					
				else
					trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex("*gurp2.wav"), 1, ATTN_NORM, 0);

				_time_pain_debounce = level.time;

				takeDamage(((g_entity_c*)g_entities[0]), ((g_entity_c*)g_entities[0]), vec3_origin, _s.origin, vec3_origin, _dmg, 0, DAMAGE_NO_ARMOR, MOD_WATER);
			}
		}
	}
	else
	{
		_time_air_finished = level.time + 12000;
		_dmg = 2;
	}


	//
	// check for sizzle damage
	//
	if(waterlevel && (_watertype & (X_CONT_LAVA|X_CONT_SLIME)) )
	{
		if(_watertype & X_CONT_LAVA)
		{
			if(_health > 0 && _time_pain_debounce <= level.time && _invincible_framenum < level.framenum)
			{
				if(rand()&1)
					trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex("player/burn1.wav"), 1, ATTN_NORM, 0);
				else
					trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex("player/burn2.wav"), 1, ATTN_NORM, 0);
				
				_time_pain_debounce = level.time + 1000;
			}

			if(envirosuit)	// take 1/3 damage with envirosuit
				takeDamage(((g_entity_c*)g_entities[0]), ((g_entity_c*)g_entities[0]), vec3_origin, _s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_LAVA);
			else
				takeDamage(((g_entity_c*)g_entities[0]), ((g_entity_c*)g_entities[0]), vec3_origin, _s.origin, vec3_origin, 3*waterlevel, 0, 0, MOD_LAVA);
		}

		if(_watertype & X_CONT_SLIME)
		{
			if(!envirosuit)
			{
				// no damage from slime with envirosuit
				takeDamage(((g_entity_c*)g_entities[0]), ((g_entity_c*)g_entities[0]), vec3_origin, _s.origin, vec3_origin, 1*waterlevel, 0, 0, MOD_SLIME);
			}
		}
	}
}


void	g_player_c::updateFallingDamage()
{
	float	delta;
	int	damage;
	vec3_t	dir;
		
	
//	if(_s.index_model != 255)
//		return;		// not in the player model

	if(_movetype == MOVETYPE_NOCLIP)
		return;

	if((_oldvelocity[2] < 0) && (_s.velocity_linear[2] > _oldvelocity[2]) && (!_groundentity))
	{
		delta = _oldvelocity[2];
	}
	else
	{
		if(!_groundentity)
			return;
			
		delta = _s.velocity_linear[2] - _oldvelocity[2];
	}
	delta = delta*delta * 0.0001;

        // scale delta if was pushed by jump pad
	if(_time_jumppad && _time_jumppad < level.time )
	{
		delta /= (1000 + level.time - _time_jumppad) * 0.5 * 0.001;
		_time_jumppad = 0;
	}	
	
	// never take falling damage if completely underwater
	if(_waterlevel == 3)
		return;
		
	if(_waterlevel == 2)
		delta *= 0.25;
		
	if(_waterlevel == 1)
		delta *= 0.5;

	if(delta < 1)
		return;

	if(delta < 15)
	{
		_s.event = EV_PLAYER_FOOTSTEP;
		return;
	}

	_fall_value = delta*0.5;
	
	if(_fall_value > 40)
		_fall_value = 40;
		
	_time_fall = level.time + FALL_TIME;

	if(delta > 30)
	{
		if(_health > 0)
		{
			if (delta >= 55)
				_s.event = EV_PLAYER_FALL_FAR;
			else
				_s.event = EV_PLAYER_FALL_MEDIUM;
		}
		_time_pain_debounce = level.time;	// no normal pain sound
		damage = (int)((delta-30)/2);
		
		if(damage < 1)
			damage = 1;
		Vector3_Set(dir, 0, 0, 1);

		if(!deathmatch->getInteger() || !(dmflags->getInteger() & DF_NO_FALLING))
			takeDamage(((g_entity_c*)g_entities[0]), ((g_entity_c*)g_entities[0]), dir, _s.origin, vec3_origin, damage, 0, 0, MOD_FALLING);
	}
	else
	{
		_s.event = EV_PLAYER_FALL_SHORT;
		return;
	}
}

/*
===============
P_DamageFeedback

Handles color blends and view kicks
===============
*/
void	g_player_c::updateDamageFeedback()
{
	float	side;
	float	realcount, count, kick;
	vec3_c	v;
	int		r, l;
	static	vec3_t	power_color = {0.0, 1.0, 0.0};
	static	vec3_t	acolor = {1.0, 1.0, 1.0};
	static	vec3_t	bcolor = {1.0, 0.0, 0.0};

	// flash the backgrounds behind the status numbers
	_r.ps.stats[STAT_FLASHES] = 0;
	
	if(_damage_blood)
		_r.ps.stats[STAT_FLASHES] |= 1;
	
	if(_damage_armor && !(_flags & FL_ULTRAMANMODE) && (_invincible_framenum <= level.framenum))
		_r.ps.stats[STAT_FLASHES] |= 2;

	// total points of damage shot at the player this frame
	count = (_damage_blood + _damage_armor + _damage_parmor);
	
	if(count == 0)
		return;		// didn't take any damage
		
		

	// start a pain animation if still in the player model
	if(_anim_priority < ANIM_PAIN)// && _s.index_model == 255)
	{
		static int		i;

		_anim_priority = ANIM_PAIN;
		
		i = (i+1)%3;
		switch (i)
		{
			case 0:
				//_s.frame = FRAME_pain101-1;
				//_anim_end = FRAME_pain104;
				
				//TODO PAIN1 animation
				
				//_anim_current = PLAYER_ANIM_PAIN1;
				//_anim_time = PLAYER_ANIM_UPPER_PAIN1_TIME;
				break;
			
			/*
			case 1:
				//_s.frame = FRAME_pain201-1;
				//_anim_end = FRAME_pain204;
				
				//TODO PAIN2 animation
				
				_anim_current = PLAYER_ANIM_PAIN2;
				_anim_time = PLAYER_ANIM_UPPER_PAIN2_TIME;
				break;
				
			case 2:
				//_s.frame = FRAME_pain301-1;
				//_anim_end = FRAME_pain304;
				
				//TODO PAIN3 animation
				
				_anim_current = PLAYER_ANIM_PAIN3;
				_anim_time = PLAYER_ANIM_UPPER_PAIN3_TIME;
				break;
			*/
		}
	}

	realcount = count;
	if(count < 10)
		count = 10;	// always make a visible effect


	// play an apropriate pain sound
	if((level.time > _time_pain_debounce) && !(_flags & FL_ULTRAMANMODE) && (_invincible_framenum <= level.framenum))
	{
		r = 1 + (rand()&1);
		_time_pain_debounce = level.time + 700;
		
		if(_health < 25)
			l = 25;
			
		else if(_health < 50)
			l = 50;
			
		else if(_health < 75)
			l = 75;
			
		else
			l = 100;
			
		trap_SV_StartSound(NULL, this, CHAN_VOICE, trap_SV_SoundIndex(va("*pain%i_%i.wav", l, r)), 1, ATTN_NORM, 0);
	}

	// the total alpha of the blend is always proportional to count
	if(_damage_alpha < 0)
		_damage_alpha = 0;
		
	_damage_alpha += count*0.01;
	
	if(_damage_alpha < 0.2)
		_damage_alpha = 0.2;
		
	if(_damage_alpha > 0.6)
		_damage_alpha = 0.6;		// don't go too saturated

	// the color of the blend will vary based on how much was absorbed
	// by different armors
	v.clear();
	if(_damage_parmor)
		Vector3_MA(v, (float)_damage_parmor/realcount, power_color, v);
		
	if(_damage_armor)
		Vector3_MA(v, (float)_damage_armor/realcount,  acolor, v);
		
	if(_damage_blood)
		Vector3_MA(v, (float)_damage_blood/realcount,  bcolor, v);
		
	_damage_blend = v;


	// calculate view angle kicks
	kick = abs(_damage_knockback);
	
	if(kick && _health > 0)	// kick of 0 means no view adjust at all
	{
		kick = kick * 100 / _health;

		if (kick < count*0.5)
			kick = count*0.5;
			
		if (kick > 50)
			kick = 50;

		Vector3_Subtract(_damage_from, _s.origin, v);
		Vector3_Normalize(v);
		
		side = Vector3_DotProduct(v, _v_right);
		_v_dmg_roll = kick*side*0.3;
		
		side = -Vector3_DotProduct(v, _v_forward);
		_v_dmg_pitch = kick*side*0.3;

		_v_dmg_time = level.time + DAMAGE_TIME;
	}

	// clear totals
	_damage_blood = 0;
	_damage_armor = 0;
	_damage_parmor = 0;
	_damage_knockback = 0;
}


void	g_player_c::updateClientEffects()
{
	//int		pa_type;
	int		remaining;

	_s.effects = 0;
	_s.renderfx = 0;

	if(_health <= 0 || level.intermission_time)
		return;
		

	/*
	if(_powerarmor_time > level.time)
	{
		pa_type = PowerArmorType(this);
		if (pa_type == POWER_ARMOR_SCREEN)
		{
			//ent->_s.effects |= EF_POWERSCREEN;
		}
		else if (pa_type == POWER_ARMOR_SHIELD)
		{
			//_s.effects |= EF_COLOR_SHELL;
			//ent->_s.renderfx |= RF_SHELL_GREEN;
		}
	}
	*/

	if(_quad_framenum > level.framenum)
	{
		remaining = (int)(_quad_framenum - level.framenum);
		if(remaining > 30 || (remaining & 4))
			_s.effects |= EF_QUAD;
	}

	/*
	if (_invincible_framenum > level.framenum)
	{
		remaining = (int)(_invincible_framenum - level.framenum);
		if (remaining > 30 || (remaining & 4) )
			ent->_s.effects |= EF_PENT;
	}
	*/

	// show cheaters!!!
	if(_flags & FL_ULTRAMANMODE)
	{
		_s.effects |= EF_COLOR_SHELL;
		//ent->_s.renderfx |= (RF_SHELL_RED|RF_SHELL_GREEN|RF_SHELL_BLUE);
	}
}


void	g_player_c::updateClientEvent()
{
	if(_s.event)
		return;

	if(_groundentity && _xyspeed > 225)
	{
		if((int)(_bob_time + _bob_move) != _bob_cycle)
			_s.event = EV_PLAYER_FOOTSTEP;
	}
}

void	g_player_c::updateClientSound()
{
	/*
	const char	*weap;
	
	if(_pers.weapon)
		weap = _pers.weapon->getClassname();
	else
		weap = "";

	if(_waterlevel && (_watertype & (X_CONT_LAVA|X_CONT_SLIME)))
		_s.index_sound = trap_SV_SoundIndex("player/fry.wav");
	
	else if(strcmp(weap, "weapon_railgun") == 0)
		_s.index_sound = trap_SV_SoundIndex("weapons/rg_hum.wav");
	
	else if (strcmp(weap, "weapon_bfg") == 0)
		_s.index_sound = trap_SV_SoundIndex("weapons/bfg_hum.wav");
	
	else if (_weapon_sound)
		_s.index_sound = _weapon_sound;
	else
		_s.index_sound = 0;
	*/
}

void	g_player_c::updateClientFrame()
{
//	_anim_time = (1.0f) / (float);
//	_anim_time_current = level.time;
//	_anim_time_last = level.time;

	_s.frame = ++_s.frame % _anims[_anim_current]->getFramesNum();

	//bool	duck, run, swim;
	//bool	update_upper, update_lower;
	
	//int	final[PLAYER_BODY_PARTS_NUM];

	//if((_s.index_model != 255) || (_r.svflags & SVF_NOCLIENT))
	//	return;		// not in the player model
	
	/*	
	if(_anim_priority == ANIM_DEATH)
	{
		if(_anim_time > 0)
			_anim_time--;
		
		// keep values	
		for(int i=0; i<PLAYER_BODY_PARTS_NUM; i++)
		{
			final[i] = _anim_current[i];
		}
	}
	else
	{
		update_upper = false;
		update_lower = false;
	
		duck	= isDucking();
		
		run	= isRunning();
		
		swim	= isSwimming();
		
			
		//TODO
		
		// test
		final[PLAYER_BODY_PART_LOWER] = PLAYER_ANIM_LOWER_STAND;
		final[PLAYER_BODY_PART_UPPER] = PLAYER_ANIM_UPPER_STAND;
		final[PLAYER_BODY_PART_HEAD]  = PLAYER_ANIM_HEAD_IDLE;
		
	}
	*/
	
	
	//
	// set frame
	//
	//_s.frame = 0;
	
	
	/*
	//if(final[PLAYER_BODY_PART_LOWER] && final[PLAYER_BODY_PART_LOWER] != _anim_lastsent[PLAYER_BODY_PART_LOWER])
	{
		_s.frame |= final[PLAYER_BODY_PART_LOWER] & 0x3F;
		_anim_lastsent[PLAYER_BODY_PART_LOWER] = final[PLAYER_BODY_PART_LOWER];
	}
	
	//if(final[PLAYER_BODY_PART_UPPER] && final[PLAYER_BODY_PART_UPPER] != _anim_lastsent[PLAYER_BODY_PART_UPPER])
	{
		_s.frame |= (final[PLAYER_BODY_PART_UPPER] & 0x3F) << 6;
		_anim_lastsent[PLAYER_BODY_PART_UPPER] = final[PLAYER_BODY_PART_UPPER];
	}

	//if(final[PLAYER_BODY_PART_HEAD] && final[PLAYER_BODY_PART_HEAD] != _anim_lastsent[PLAYER_BODY_PART_HEAD])
	{
		_s.frame |= (final[PLAYER_BODY_PART_HEAD] & 0x3F) << 12;
		_anim_lastsent[PLAYER_BODY_PART_HEAD] = final[PLAYER_BODY_PART_HEAD];
	}
	*/
	
	
	
	/*
	if(_r.ps.pmove.pm_flags & PMF_DUCKED)
		duck = true;
	else
		duck = false;
	if(_xyspeed)
		run = true;
	else
		run = false;
	*/

	// check for stand/duck and stop/go transitions
	/*
	if(duck != _anim_duck && _anim_priority < ANIM_DEATH)
		goto newanim;
		
	if(run != _anim_run && _anim_priority == ANIM_BASIC)
		goto newanim;
		
	if(!_groundentity && _anim_priority <= ANIM_WAVE)
		goto newanim;

	if(_anim_priority == ANIM_REVERSE)
	{
		if(_s.frame > _anim_end)
		{
			_s.frame--;
			return;
		}
	}
	else if(_s.frame < _anim_end)
	{	
		// continue an animation
		_s.frame++;
		return;
	}
	*/

		
	/*	
	if(_anim_priority == ANIM_JUMP)
	{
		if(!_groundentity)
			return;		// stay there
			
		_anim_priority = ANIM_WAVE;
		_s.frame = FRAME_jump3;
		_anim_end = FRAME_jump6;
		return;
	}

newanim:
	// return to either a running or standing frame
	_anim_priority = ANIM_BASIC;
	_anim_duck = duck;
	_anim_run = run;

	if(!_groundentity)
	{
		_anim_priority = ANIM_JUMP;
		
		if(_s.frame != FRAME_jump2)
			_s.frame = FRAME_jump1;
			
		_anim_end = FRAME_jump2;
	}
	else if(run)
	{	
		// running
		if(duck)
		{
			_s.frame = FRAME_crwalk1;
			_anim_end = FRAME_crwalk6;
		}
		else
		{
			_s.frame = FRAME_run1;
			_anim_end = FRAME_run6;
		}
	}
	else
	{	// standing
		if(duck)
		{
			_s.frame = FRAME_crstnd01;
			_anim_end = FRAME_crstnd19;
		}
		else
		{
			_s.frame = FRAME_stand01;
			_anim_end = FRAME_stand40;
		}
	}
	*/
}



float	g_player_c::calcRoll(const vec3_c &angles, const vec3_c &velocity)
{
	float	sign;
	float	side;
	float	value;
	
	side = velocity.dotProduct(_v_right);
	sign = side < 0 ? -1 : 1;
	side = fabs(side);
	
	value = sv_rollangle->getValue();

	if(side < sv_rollspeed->getValue())
		side = side * value / sv_rollspeed->getValue();
	else
		side = value;
	
	return side*sign;
	
}


/*
===============
SV_CalcViewOffset

Auto pitching on slopes?

  fall from 128: 400 = 160000
  fall from 256: 580 = 336400
  fall from 384: 720 = 518400
  fall from 512: 800 = 640000
  fall from 640: 960 = 

  damage = deltavelocity*deltavelocity  * 0.0001

===============
*/
void	g_player_c::calcViewOffset()
{
	float		bob;
	float		ratio;
	float		delta;
	vec3_c		v;

	
	// if dead, fix the angle and don't add any kick
	if(_deadflag)
	{
		_r.ps.kick_angles.clear();

		_r.ps.view_angles[ROLL] = 40;
		_r.ps.view_angles[PITCH] = -15;
		_r.ps.view_angles[YAW] = _killer_yaw;
	}
	else
	{
		_r.ps.kick_angles.clear();

		// add angles based on damage kick
		ratio = (_v_dmg_time - level.time) / DAMAGE_TIME;
		if (ratio < 0)
		{
			ratio = 0;
			_v_dmg_pitch = 0;
			_v_dmg_roll = 0;
		}
		_r.ps.kick_angles[PITCH] += ratio * _v_dmg_pitch;
		_r.ps.kick_angles[ROLL] += ratio * _v_dmg_roll;
	
		// add pitch based on fall kick
		ratio = ((_time_fall - level.time) / FALL_TIME);// * 0.001;
		if (ratio < 0)
			ratio = 0;
		_r.ps.kick_angles[PITCH] += ratio * _fall_value;

		// add angles based on velocity
		delta = _s.velocity_linear.dotProduct(_v_forward);
		_r.ps.kick_angles[PITCH] += delta*run_pitch->getValue();
		
		delta = _s.velocity_linear.dotProduct(_v_right);
		_r.ps.kick_angles[ROLL] += delta*run_roll->getValue();

		// add angles based on bob
		delta = _bob_fracsin * bob_pitch->getValue() * _xyspeed;
		if (_r.ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
			
		_r.ps.kick_angles[PITCH] += delta;
		delta = _bob_fracsin * bob_roll->getValue() * _xyspeed;
		
		if(_r.ps.pmove.pm_flags & PMF_DUCKED)
			delta *= 6;		// crouching
		if(_bob_cycle & 1)
			delta = -delta;
			
		_r.ps.kick_angles[ROLL] += delta;
	}

	// base origin
	v.clear();

	// add view height
	v[2] += _v_height;

	// add fall height
	ratio = ((_time_fall - level.time) / FALL_TIME);// * 0.001;
	if(ratio < 0)
		ratio = 0;
	v[2] -= ratio * _fall_value * 0.4;

	// add bob height
	bob = _bob_fracsin * _xyspeed * bob_up->getValue();
	if(bob > 6)
		bob = 6;
	v[2] += bob;

	// add kick offset
	v += _kick_origin;

	// absolutely bound offsets
	// so the view can never be outside the player box
	X_clamp(v[0],-14, 14);
	X_clamp(v[1],-14, 14);
	X_clamp(v[2], MINS_Z+1, VIEWHEIGHT_DEFAULT-1);
	
	_r.ps.view_offset = v;
}

void	g_player_c::calcGunOffset()
{
	// gun angles from bobbing
	_r.ps.gun_angles[ROLL] = _xyspeed * _bob_fracsin * 0.005;
	_r.ps.gun_angles[YAW] = _xyspeed * _bob_fracsin * 0.01;
	if(_bob_cycle & 1)
	{
		_r.ps.gun_angles[ROLL] = -_r.ps.gun_angles[ROLL];
		_r.ps.gun_angles[YAW] = -_r.ps.gun_angles[YAW];
	}

	_r.ps.gun_angles[PITCH] = _xyspeed * _bob_fracsin * 0.005;

	// gun angles from delta movement
	for(int i=0; i<3; i++)
	{
		vec_t delta = _oldviewangles[i] - _r.ps.view_angles[i];
		
		if(delta > 180)
			delta -= 360;
		if(delta < -180)
			delta += 360;
		if(delta > 45)
			delta = 45;
		if(delta < -45)
			delta = -45;
		
		if(i == YAW)
			_r.ps.gun_angles[ROLL] += 0.1*delta;
		_r.ps.gun_angles[i] += 0.2 * delta;
	}

	// gun height
	_r.ps.gun_offset.clear();
	//ent->ps->gunorigin[2] += bob;

	// gun_x / gun_y / gun_z are development tools
	for(int i=0; i<3; i++)
	{
		_r.ps.gun_offset[i] += _v_forward[i]*(gun_y->getValue());
		_r.ps.gun_offset[i] += _v_right[i]*gun_x->getValue();
		_r.ps.gun_offset[i] += _v_up[i]* (-gun_z->getValue());
	}
}


void	g_player_c::calcBlend()
{
	int		contents;
	vec3_c		vieworg;
	int		remaining;
	
	
	_r.ps.blend.clear();

	// add for contents
	vieworg = _s.origin + _r.ps.view_offset;
	contents = G_PointContents(vieworg);
	
	if(contents & (X_CONT_LAVA|X_CONT_SLIME|X_CONT_WATER) )
		_r.ps.rdflags |= RDF_UNDERWATER;
	else
		_r.ps.rdflags &= ~RDF_UNDERWATER;

	if(contents & (X_CONT_SOLID|X_CONT_LAVA))
		addBlend(1.0, 0.3, 0.0, 0.6, _r.ps.blend);
		
	else if(contents & X_CONT_SLIME)
		addBlend(0.0, 0.1, 0.05, 0.6, _r.ps.blend);
		
	else if(contents & X_CONT_WATER)
		addBlend(0.5, 0.3, 0.2, 0.4, _r.ps.blend);

	// add for powerups
	if (_quad_framenum > level.framenum)
	{
		remaining = (int)(_quad_framenum - level.framenum);
		
		if (remaining == 30)	// beginning to fade
			trap_SV_StartSound(NULL, this, CHAN_ITEM, trap_SV_SoundIndex("items/damage2.wav"), 1, ATTN_NORM, 0);
		
		if (remaining > 30 || (remaining & 4) )
			addBlend (0, 0, 1, 0.08, _r.ps.blend);
	}
	else if (_invincible_framenum > level.framenum)
	{
		remaining = (int)(_invincible_framenum - level.framenum);
		
		if (remaining == 30)	// beginning to fade
			trap_SV_StartSound(NULL, this, CHAN_ITEM, trap_SV_SoundIndex("items/protect2.wav"), 1, ATTN_NORM, 0);
		
		if (remaining > 30 || (remaining & 4) )
			addBlend (1, 1, 0, 0.08, _r.ps.blend);
	}
	else if (_enviro_framenum > level.framenum)
	{
		remaining = (int)(_enviro_framenum - level.framenum);
		
		if (remaining == 30)	// beginning to fade
			trap_SV_StartSound(NULL, this, CHAN_ITEM, trap_SV_SoundIndex("items/airout.wav"), 1, ATTN_NORM, 0);
		
		if (remaining > 30 || (remaining & 4) )
			addBlend (0, 1, 0, 0.08, _r.ps.blend);
	}
	else if (_breather_framenum > level.framenum)
	{
		remaining = (int)(_breather_framenum - level.framenum);
		if (remaining == 30)	// beginning to fade
			trap_SV_StartSound(NULL, this, CHAN_ITEM, trap_SV_SoundIndex("items/airout.wav"), 1, ATTN_NORM, 0);
		if (remaining > 30 || (remaining & 4) )
			addBlend (0.4, 1, 0.4, 0.04, _r.ps.blend);
	}

	// add for damage
	if(_damage_alpha > 0)
		addBlend (_damage_blend[0],_damage_blend[1]
		,_damage_blend[2], _damage_alpha, _r.ps.blend);

	if(_bonus_alpha > 0)
		addBlend (0.85, 0.7, 0.3, _bonus_alpha, _r.ps.blend);

	// drop the damage value
	_damage_alpha -= 0.06;
	if (_damage_alpha < 0)
		_damage_alpha = 0;

	// drop the bonus value
	_bonus_alpha -= 0.1;
	if (_bonus_alpha < 0)
		_bonus_alpha = 0;
}

void	g_player_c::addBlend(float r, float g, float b, float a, vec4_c &v_blend)
{
	float	a2, a3;

	if (a <= 0)
		return;
	a2 = v_blend[3] + (1-v_blend[3])*a;	// new total alpha
	a3 = v_blend[3]/a2;		// fraction of color from old

	v_blend[0] = v_blend[0]*a3 + r*(1-a3);
	v_blend[1] = v_blend[1]*a3 + g*(1-a3);
	v_blend[2] = v_blend[2]*a3 + b*(1-a3);
	v_blend[3] = a2;
}

bool	g_player_c::isDucking()
{
	if(_r.ps.pmove.pm_flags & PMF_DUCKED)
		return true;
	else
		return false;
}

bool	g_player_c::isRunning()
{
	if(_xyspeed)
		return true;
	else
		return false;
}

bool	g_player_c::isSwimming()
{
	if(_waterlevel > 2)
		return true;
	
	if(_waterlevel && !_groundentity)
	{
		if(!isStepping())
			return true;
	}
	
	return false;
}

#define PM_STEPSIZE	18
bool	g_player_c::isStepping()
{
	/*
	vec3_c	point;
	trace_t	trace;
	
	point = _s.origin;
	point[2] -= (1.6*PM_STEPSIZE);
	
	// trace to point
	trace = trap_SV_Trace(_s.origin, _r.bbox, point, this, MASK_PLAYERSOLID);
	
	if(!trace.ent || (trace.plane._normal[2] < 0.7 && !trace.startsolid))
	{
		return false;
	}
	
	//found solid.
	return true;
	*/
	
	return false;
}

bool	g_player_c::scanAnimations(const std::string &model)
{
	trap_Com_Printf("g_player_c::scanAnimations('%s') from '%s' ...\n", model.c_str(), _pers.netname);
	
	for(uint_t i=0; i<PLAYER_ANIMS_NUM; i++)
	{
		std::string name = "models/players/" + model + "/" + std::string(player_anims[i].name) + ".md5anim";
		
		if(trap_VFS_FLoad(name, NULL) <= 0)
		{
			Com_Error(ERR_DROP, "g_player_c::scanAnimations: player model '%s' does not provide animation '%s'", model.c_str(), name.c_str());
			return false;
		}
		
		//trap_Com_Printf("setting '%s' ...\n", name.c_str());
		_anims[i] = scanAnimation(name);
	}

	return true;
}

animation_c*	g_player_c::scanAnimation(const std::string &name)
{
	// tell client to load animation
	trap_SV_AnimationIndex(name);

	// return proper animation pointer
	return trap_CM_RegisterAnimation(name);
}

void 	g_player_c::selectNextItem(int item_flags)
{
	/*
	int		i, index;
	g_item_c*	item;

	// scan  for the next valid one
	for(i=1; i<=MAX_ITEMS; i++)
	{
		index = (_pers.selected_item + i)%MAX_ITEMS;
		
		if(!_pers.inventory[index])
			continue;
			
		item = G_GetItemByNum(index);
		
		if(!item->hasUse())
			continue;
		
		if(!(item->getFlags() & item_flags))
			continue;

		_pers.selected_item = index;
		return;
	}
	*/

	_pers.selected_item = -1;
}


void 	g_player_c::selectPrevItem(int item_flags)
{
	/*
	int		i, index;
	g_item_c*	item;
	
	// scan  for the next valid one
	for(i=1; i<=MAX_ITEMS; i++)
	{
		index = (_pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		
		if(!_pers.inventory[index])
			continue;
		
		item = G_GetItemByNum(index);
		
		if(!item->hasUse())
			continue;
			
		if(!(item->getFlags() & item_flags))
			continue;

		_pers.selected_item = index;
		return;
	}
	*/

	_pers.selected_item = -1;
}


void	g_player_c::validateSelectedItem()
{
	if(_pers.inventory[_pers.selected_item])
		return;		// valid

	// try next
	selectNextItem(-1);
}

/*
g_entity_c*	g_player_c::dropItem(g_item_c *item)
{
	vec3_c	forward, right, up;
	vec3_c	offset(24, 0, 0);
	vec3_c	origin;

	Angles_ToVectors(_v_angles, forward, right, up);
		
	G_ProjectSource(_s.origin, offset, forward, right, origin);
	
	//trace = trap_SV_Trace(_s.origin, dropped->_r.bbox, origin, this, X_CONT_SOLID);
	
	quaternion_c orientation;
	orientation.fromAngles(0, _v_angles[YAW], 0);
	
	vec3_c velocity = forward * 10;
	velocity[2] = 30 * 3;
	
	g_item_dropable_c *dropped = new g_item_dropable_c(this, item, origin, orientation, velocity);
		
	//trap_Com_Printf("g_player_c::dropItem: body at %s\n", dropped->_s.origin.toString());

	return dropped;
}
*/


/*
==================
Cmd_Give_f

Give items to a client
==================
*/
void 	g_player_c::give_f()
{
	const char*	name;
	bool		give_all;
//	g_entity_c*	item;

	if(deathmatch->getValue() && !sv_cheats->getValue())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	name = trap_Cmd_Args();

	if (X_stricmp(name, "all") == 0)
		give_all = true;
	else
		give_all = false;

	if(give_all || X_stricmp(trap_Cmd_Argv(1), "health") == 0)
	{
		if(trap_Cmd_Argc() == 3)
			_health = atoi(trap_Cmd_Argv(2));
		else
			_health = _max_health;
		
		if(!give_all)
			return;
	}

#if 0
	if(give_all || X_stricmp(name, "weapons") == 0)
	{
		for(unsigned int i=0; i<g_items.size(); i++)
		{
			it = g_items[i];
			
			if(!it->hasPickup())
				continue;
				
			if(!(it->getFlags() & IT_WEAPON))
				continue;
				
			_pers.inventory[i] += 1;
		}
		
		if(!give_all)
			return;
	}
#endif

#if 0
	if(give_all || X_stricmp(name, "ammo") == 0)
	{
		for(unsigned int i=0; i<g_items.size(); i++)
		{
			it = g_items[i];
			
			if(!it->hasPickup())
				continue;
				
			if(!(it->getFlags() & IT_AMMO))
				continue;
				
			addAmmo(it, 1000);
		}
		
		if(!give_all)
			return;
	}
#endif

#if 0
	if(give_all || X_stricmp(name, "dummy") == 0)
	{
		for(unsigned int i=0; i<g_items.size(); i++)
		{
			it = g_items[i];
			
			if(!it->hasPickup())
				continue;
				
			if(!(it->getFlags() & IT_WEAPON))
				continue;
				
			_pers.inventory[i] += 1;
		}
		
		if(!give_all)
			return;
	}
#endif

#if 0
	if(give_all || X_stricmp(name, "armor") == 0)
	{
		gitem_armor_t	*info;

		it = G_FindItem("Jacket Armor");
		ent->_pers.inventory[G_GetNumForItem(it)] = 0;

		it = G_FindItem("Combat Armor");
		ent->_pers.inventory[G_GetNumForItem(it)] = 0;

		it = G_FindItem("Body Armor");
		info = (gitem_armor_t*)it->getInfo();
		ent->_pers.inventory[G_GetNumForItem(it)] = info->max_count;

		if (!give_all)
			return;
	}
#endif

#if 0
	if (give_all || X_stricmp(name, "Power Shield") == 0)
	{
		it = G_FindItem("Power Shield");
		//it_ent = G_Spawn();
		it_ent = new g_entity_c();
		it_ent->_classname = it->getClassname();
		
		G_SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		
		if (it_ent->_r.inuse)
			delete it_ent;

		if (!give_all)
			return;
	}
#endif

#if 0
	if(give_all)
	{
		for(unsigned int i=0; i<g_items.size(); i++)
		{
			it = g_items[i];
			
			if(!it->hasPickup())
				continue;
				
			if(it->getFlags() & (IT_ARMOR | IT_WEAPON | IT_AMMO))
				continue;
				
			_pers.inventory[i] = 1;
		}
		return;
	}

	it = G_FindItem (name);
	if(!it)
	{
		name = trap_Cmd_Argv(1);
		it = G_FindItem (name);
		if (!it)
		{
			trap_SV_CPrintf(this, PRINT_HIGH, "unknown item\n");
			return;
		}
	}

	if(!it->hasPickup())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "non-pickup item\n");
		return;
	}

	index = G_GetNumForItem(it);

	if(it->getFlags() & IT_AMMO)
	{
		if(trap_Cmd_Argc() == 3)
			_pers.inventory[index] = atoi(trap_Cmd_Argv(2));
		else
			_pers.inventory[index] += it->getQuantity();
	}
	else
	{
		/*
		//it_ent = G_Spawn();
		it_ent = new g_entity_dropable_c();
		it_ent->_classname = it->getClassname();
		G_SpawnItem (it_ent, it);
		Touch_Item (it_ent, ent, NULL, NULL);
		
		if (it_ent->_r.inuse)
			delete it_ent;
		*/
	}
#endif
}


/*
==================
Cmd_Ultraman_f

Sets client to ultraman mode

argv(0) ultraman
==================
*/
void	g_player_c::ultraman_f()
{
	char	*msg;

	if(deathmatch->getValue() && !sv_cheats->getValue())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	_flags ^= FL_ULTRAMANMODE;
	
	if(!(_flags & FL_ULTRAMANMODE))
		msg = "ultraman mode OFF\n";
	else
		msg = "ultraman mode ON\n";

	trap_SV_CPrintf(this, PRINT_HIGH, msg);
}


/*
==================
Cmd_Notarget_f

Sets client to notarget

argv(0) notarget
==================
*/
void 	g_player_c::notarget_f()
{
	char	*msg;

	if(deathmatch->getValue() && !sv_cheats->getValue())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	_flags ^= FL_NOTARGET;
	
	if(!(_flags & FL_NOTARGET))
		msg = "notarget OFF\n";
	else
		msg = "notarget ON\n";

	trap_SV_CPrintf(this, PRINT_HIGH, msg);
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void 	g_player_c::noclip_f()
{
	char	*msg;

	if(deathmatch->getValue() && !sv_cheats->getValue())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "You must run the server with '+set cheats 1' to enable this command.\n");
		return;
	}

	if(_movetype == MOVETYPE_NOCLIP)//!_space->isEnabled())
	{
		_movetype = MOVETYPE_WALK;
//		_body->setGravityMode(1);
//		_space->enable();
		msg = "noclip OFF\n";
	}
	else
	{
		_movetype = MOVETYPE_NOCLIP;
//		_body->setGravityMode(0);
//		_space->disable();
		msg = "noclip ON\n";
	}

	trap_SV_CPrintf(this, PRINT_HIGH, msg);
}


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void 	g_player_c::use_f()
{
	/*
	int			index;
	g_item_c		*item;
	const char		*s;

	s = trap_Cmd_Args();
	item = G_FindItem(s);
	
	if(!item)
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	
	if(!item->hasUse())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	
	index = G_GetNumForItem(item);
	
	if(!_pers.inventory[index])
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	item->use(this);
	*/
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
void	g_player_c::drop_f()
{
	/*
	int			index;
	g_item_c		*item;
	const char		*s;

	s = trap_Cmd_Args();
	item = G_FindItem(s);
	
	if(!item)
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	
	if(!item->hasDrop())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "Item %s is not dropable.\n", item->getClassname());
		return;
	}
	
	index = G_GetNumForItem(item);
	
	if(!_pers.inventory[index])
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	item->drop(this);
	*/
}


/*
=================
Cmd_Inven_f
=================
*/
void 	g_player_c::inven_f()
{
	/*
	_showscores = false;
	_showhelp = false;

	if(_showinventory)
	{
		_showinventory = false;
		return;
	}

	_showinventory = true;

	trap_SV_WriteBits(SVC_INVENTORY, svc_bitcount);
	for(int i=0; i<MAX_ITEMS; i++)
	{
		trap_SV_WriteShort(_pers.inventory[i]);
	}
	trap_SV_Unicast(this, true);
	*/
}

/*
=================
Cmd_InvUse_f
=================
*/
void 	g_player_c::invUse_f()
{
	/*
	g_item_c	*item;

	validateSelectedItem();

	if(_pers.selected_item == -1)
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "No item to use.\n");
		return;
	}

	item = G_GetItemByNum(_pers.selected_item);
	
	if(!item->hasUse())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	
	item->use(this);
	*/
}

/*
=================
Cmd_InvDrop_f
=================
*/
void 	g_player_c::invDrop_f()
{
	/*
	g_item_c		*item;

	validateSelectedItem();

	if(_pers.selected_item == -1)
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	item = G_GetItemByNum(_pers.selected_item);
	
	if(!item->hasDrop())
	{
		trap_SV_CPrintf(this, PRINT_HIGH, "Item %s is not dropable.\n", item->getClassname());
		return;
	}
	
	item->drop(this);
	*/
}


/*
=================
Cmd_WeapPrev_f
=================
*/
void	g_player_c::weapPrev_f()
{
	/*
	int			i, index;
	g_item_c		*item;
	int			selected_weapon;


	if(!_pers.weapon)
		return;

	selected_weapon = G_GetNumForItem(_pers.weapon);

	// scan  for the next valid one
	for(i=1; i<=MAX_ITEMS; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		
		if(!_pers.inventory[index])
			continue;
			
		item = G_GetItemByNum(index);
		
		if(!item->hasUse())
			continue;
			
		if(!(item->getFlags() & IT_WEAPON))
			continue;
			
		item->use(this);
		
		if(_pers.weapon == item)
			return;	// successful
	}
	*/
}

/*
=================
Cmd_WeapNext_f
=================
*/
void 	g_player_c::weapNext_f()
{
	/*
	if(!_pers.weapon)
		return;

	int selected_weapon = G_GetNumForItem(_pers.weapon);

	// scan  for the next valid one
	for(int i=1; i<=MAX_ITEMS; i++)
	{
		int index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		
		if(!_pers.inventory[index])
			continue;
			
		g_item_c *item = G_GetItemByNum(index);
		
		if(!item->hasUse())
			continue;
			
		if(!(item->getFlags() & IT_WEAPON))
			continue;
			
		item->use(this);
		
		if(_pers.weapon == item)
			return;	// successful
	}
	*/
}

/*
=================
Cmd_WeapLast_f
=================
*/
void	g_player_c::weapLast_f()
{
	/*
	int			index;
	g_item_c		*item;


	if(!_pers.weapon || !_pers.lastweapon)
		return;

	index = G_GetNumForItem(_pers.lastweapon);
	
	if(!_pers.inventory[index])
		return;
		
	item = G_GetItemByNum(index);
	
	if(!item->hasUse())
		return;
		
	if(!(item->getFlags() & IT_WEAPON))
		return;
		
	item->use(this);
	*/
}

void	g_player_c::weapReload_f()
{
	/*
	if(!_pers.weapon)
		return;
		
	if(!(_pers.weapon->getFlags() & IT_WEAPON))
	{
		trap_Com_Error(ERR_DROP, "g_player_c::weapReload_f: _pers.weapon has no IT_WEAPON flag");
		return;
	}
	
	_weapon_state = WEAPON_RELOADING;
	//_weapon_update = level.time;
	_r.ps.gun_model_index = trap_SV_ModelIndex(_pers.weapon->getViewModel());
	_r.ps.gun_anim_frame = 0;//_pers.weapon->getReloadAnimationFirstFrame();
	_r.ps.gun_anim_index = trap_SV_AnimationIndex(_pers.weapon->getReloadAnimationName());
	
	_pers.weapon->reload(this);
	*/
}


/*
=================
Cmd_Kill_f
=================
*/
void	g_player_c::kill_f()
{
	if((level.time - _time_respawn) < 5000)
		return;
		
	_flags &= ~FL_ULTRAMANMODE;
	_health = 0;
	
	meansOfDeath = MOD_SUICIDE;
	
	die(this, this, 100000, vec3_origin);
}

/*
=================
Cmd_PutAway_f
=================
*/
void 	g_player_c::putAway_f()
{
	_showscores = false;
	_showhelp = false;
	_showinventory = false;
}


/*
static int 	PlayerSort(void const *a, void const *b)
{
	
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = g_clients[anum]->ps.stats[STAT_FRAGS];
	bnum = g_clients[bnum]->ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
		
	if (anum > bnum)
		return 1;
	
	return 0;
}
*/

void 	g_player_c::players_f()
{
	/*
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];
	
	g_player_c *player;

	count = 0;
	for(i=0; i<maxclients->getInteger(); i++)
	{
		if(g_clients[i]->pers.connected)
		{
			index[count] = i;
			count++;
		}
	}

	// sort by frags
	qsort(index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for(i=0; i<count; i++)
	{
		Com_sprintf(small, sizeof(small), "%3i %s\n", g_clients[index[i]]->ps.stats[STAT_FRAGS], g_clients[index[i]]->pers.netname);
			
		if(strlen (small) + strlen(large) > sizeof(large) - 100 )
		{
			// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat(large, small);
	}

	trap_SV_CPrintf(ent, PRINT_HIGH, "%s\n%i players\n", large, count);
	*/
}

/*
=================
Cmd_Wave_f
=================
*/
void 	g_player_c::wave_f()
{
	int		i;

	i = atoi (trap_Cmd_Argv(1));

	// can't wave when ducked
	//if(_r.ps.pmove.pm_flags & PMF_DUCKED)
	//	return;

	if(_anim_priority > ANIM_WAVE)
		return;

	_anim_priority = ANIM_WAVE;

	switch(i)
	{
		/*
		case 0:
			trap_SV_CPrintf(this, PRINT_HIGH, "flipoff\n");
			_s.frame = FRAME_flip01-1;
			_anim_end = FRAME_flip12;
			break;
		
		case 1:
			trap_SV_CPrintf(this, PRINT_HIGH, "salute\n");
			_s.frame = FRAME_salute01-1;
			_anim_end = FRAME_salute11;
			break;
			
		case 2:
			trap_SV_CPrintf(this, PRINT_HIGH, "taunt\n");
			_s.frame = FRAME_taunt01-1;
			_anim_end = FRAME_taunt17;
			break;
			
		case 3:
			trap_SV_CPrintf(this, PRINT_HIGH, "wave\n");
			_s.frame = FRAME_wave01-1;
			_anim_end = FRAME_wave11;
			break;
		
		case 4:
		*/
		
		default:
			trap_SV_CPrintf(this, PRINT_HIGH, "point\n");
			//_s.frame = FRAME_point01-1;
			//_anim_end = FRAME_point12;
			
			//_s.frame = (PLAYER_ANIM_UPPER_GESTURE & 0x3F);
			_anim_current = PLAYER_ANIM_FISTS_IDLE;
			_anim_time = PLAYER_ANIM_UPPER_GESTURE_TIME;
			break;
	}
}

/*
==================
Cmd_Say_f
==================
*/
void 	g_player_c::say_f(bool team, bool arg0)
{
	int		i, j;
	g_player_c	*other;
	char	*p;
	char	text[2048];
	//g_client_c *cl;

	if(trap_Cmd_Argc () < 2 && !arg0)
		return;

	if(!(dmflags->getInteger() & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if(team)
		Com_sprintf(text, sizeof(text), "(%s): ", _pers.netname);
	else
		Com_sprintf(text, sizeof(text), "%s: ", _pers.netname);

	if (arg0)
	{
		strcat (text, trap_Cmd_Argv(0));
		strcat (text, " ");
		strcat (text, trap_Cmd_Args());
	}
	else
	{
		p = (char*)trap_Cmd_Args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if(flood_msgs->getInteger()) 
	{
		//cl = ent->getClient();

        	if(level.time < _flood_locktill)
		{
			trap_SV_CPrintf(this, PRINT_HIGH, "You can't talk for %d more seconds\n", (int)(_flood_locktill - level.time));
			return;
        	}
        
		i = _flood_whenhead - flood_msgs->getInteger() + 1;
        	
		if(i < 0)
        		i = (sizeof(_flood_when)/sizeof(_flood_when[0])) + i;
		
		if(_flood_when[i] && level.time - _flood_when[i] < flood_persecond->getInteger()) 
		{
			_flood_locktill = level.time + flood_waitdelay->getInteger();
			
			trap_SV_CPrintf(this, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n", (int)flood_waitdelay->getInteger());
			return;
        	}	
		
		_flood_whenhead = (_flood_whenhead + 1) % (sizeof(_flood_when)/sizeof(_flood_when[0]));
		_flood_when[_flood_whenhead] = level.time;
	}

	if(dedicated->getInteger())
		trap_SV_CPrintf(NULL, PRINT_CHAT, "%s", text);


	for(j=1; j<=game.maxclients; j++)
	{
		other = (g_player_c*)g_entities[j];
		
		if(!other->_r.inuse)
			continue;
			
		if(team)
		{
			if(!G_OnSameTeam(this, other))
				continue;
		}
		trap_SV_CPrintf(other, PRINT_CHAT, "%s", text);
	}
}

void 	g_player_c::playerList_f()
{
	int i;
	char st[80];
	char text[1400];
	g_player_c *player = NULL;

	// connect time, ping, score, name
	*text = 0;
	for(i=0; i<maxclients->getInteger(); i++) 
	{
		player = (g_player_c*)g_entities[i+1];
		
		if(!player->_r.inuse)
			continue;
			
		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s%s\n",
			(level.framenum - player->_resp.enterframe) / 600,
			((level.framenum - player->_resp.enterframe) % 600)/10,
			player->_r.ping,
			player->_resp.score,
			player->_pers.netname,
			player->_resp.spectator ? " (spectator)" : "");
			
		if(strlen(text) + strlen(st) > sizeof(text) - 50) 
		{
			sprintf(text+strlen(text), "And more...\n");
			trap_SV_CPrintf(this, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	trap_SV_CPrintf(this, PRINT_HIGH, "%s", text);
}


void	g_player_c::score_f()
{
	_showinventory = false;
	_showhelp = false;

	if(!deathmatch->getInteger() && !coop->getInteger())
		return;

	if(_showscores)
	{
		_showscores = false;
		return;
	}

	_showscores = true;
	deathmatchScoreBoard();
}

void	g_player_c::animNext_f()
{
	_anim_current = X_bound(PLAYER_ANIM_FIRST, _anim_current+1, PLAYER_ANIM_LAST);
	_s.index_animation = trap_SV_AnimationIndex(_anims[_anim_current]->getName());
}

void	g_player_c::animPrev_f()
{
	_anim_current = X_bound(PLAYER_ANIM_FIRST, _anim_current-1, PLAYER_ANIM_LAST);
	_s.index_animation = trap_SV_AnimationIndex(_anims[_anim_current]->getName());
}

void	g_player_c::animReset_f()
{
	_s.frame = 0;
}

void	g_player_c::spawnRocket_f()
{
	vec3_c	start;
	vec3_c	offset(24, 8, _v_height - 8);
	
	projectSource(offset, start);
	
	_kick_origin = _v_forward * -2;
	_kick_angles[0] = -1;

	new g_projectile_rocket_c(this, start, _v_quat, 150, 20, 120, 120);
}


void	g_player_c::deathmatchScoreBoardMessage()
{
	char		entry[1024];
	char		string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;

	int		x, y;
	g_player_c	*player;


	// sort the clients by score
	total = 0;
	for(i=0; i<game.maxclients; i++)
	{
		player = (g_player_c*)g_entities[1+i];
		
		if(!player->_r.inuse || player->_resp.spectator)
			continue;
			
		score = player->_resp.score;
		
		for(j=0; j<total; j++)
		{
			if(score > sortedscores[j])
				break;
		}
		for(k=total; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}


	// print level name and exit rules
	string[0] = 0;
	stringlength = strlen(string);

	// add the clients in sorted order
	if(total > 12)
		total = 12;

	for(i=0; i<total; i++)
	{
		player = (g_player_c*)g_entities[1 + sorted[i]];

		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// send the layout						
		Com_sprintf(entry, sizeof(entry), "client %i %i %i %i %i %i ", x, y, sorted[i], player->_resp.score, player->_r.ping, (level.framenum - player->_resp.enterframe)/600);
		j = strlen(entry);
	
		if(stringlength + j > 1024)
			break;
			
		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	trap_SV_WriteBits(SVC_LAYOUT, svc_bitcount);
	trap_SV_WriteString(string);
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void	g_player_c::deathmatchScoreBoard()
{
	deathmatchScoreBoardMessage();
	trap_SV_Unicast(this, true);
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void	g_player_c::thinkWeapon()
{
	/*
	trap_Com_DPrintf("g_player_c::thinkWeapon() from '%s'\n", _pers.netname);

	// if just died, put the weapon away
	if(_health < 1)
	{
		_newweapon = NULL;
		changeWeapon();
	}

	// call active weapon think routine
	if(_pers.weapon && _pers.weapon->isWeapon())
	{
		//is_quad = (ent->_quad_framenum > level.framenum);
		
		//if(ent->_silencer_shots)
		//	is_silenced = MZ_SILENCED;
		//else
		//is_silenced = 0;
			
		_pers.weapon->weaponThink(this);
	}
	*/
}

/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void	g_player_c::changeWeapon()
{
	/*
	if (ent->_grenade_time)
	{
		ent->_grenade_time = level.time;
		ent->_weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->_grenade_time = 0;
	}
	*/

	/*
	_pers.lastweapon = _pers.weapon;
	_pers.weapon = _newweapon;
	_newweapon = NULL;
	_machinegun_shots = 0;

	if(_pers.weapon && _pers.weapon->getAmmo())
		_ammo_index = G_GetNumForItem(G_FindItem(_pers.weapon->getAmmo()));
	else
		_ammo_index = 0;

	if(!_pers.weapon)
	{	
		// dead
		_r.ps.gun_model_index = 0;
		return;
	}
	
	_weapon_state = WEAPON_ACTIVATING;
	_weapon_update = level.time;
	_r.ps.gun_model_index = trap_SV_ModelIndex(_pers.weapon->getViewModel());
	_r.ps.gun_anim_frame = 0;//_pers.weapon->getActivateAnimationFirstFrame();
	_r.ps.gun_anim_index = trap_SV_AnimationIndex(_pers.weapon->getActivateAnimationName());
	

	// FIXME
	_anim_priority = ANIM_PAIN;
	_anim_current = PLAYER_ANIM_FISTS_IDLE;
	_anim_time = PLAYER_ANIM_UPPER_FLIPIN_TIME;
	*/
}

void	g_player_c::tossWeapon()
{
	/*
	g_item_c	*item;
	g_entity_c	*drop;
	bool	quad;
	float		spread;

	//if (!deathmatch->getInteger())
		return;

	item = _pers.weapon;
	
	if(!_pers.inventory[_ammo_index] )
		item = NULL;
		
	if(item && (X_stricmp (item->getPickupName(), "Blaster") == 0))
		item = NULL;

	if(!(dmflags->getInteger() & DF_QUAD_DROP))
		quad = false;
	else
		quad = (_quad_framenum > (level.framenum + 10));

	if(item && quad)
		spread = 22.5;
	else
		spread = 0.0;

	if(item)
	{
		_v_angles[YAW] -= spread;
		drop = dropItem(item);
		_v_angles[YAW] += spread;
		drop->_spawnflags = DROPPED_PLAYER_ITEM;
	}
	*/

	/*
	if (quad)
	{
		self->_v_angles[YAW] += spread;
		drop = Drop_Item (self, G_FindItemByClassname ("item_quad"));
		self->_v_angles[YAW] -= spread;
		drop->spawnflags |= DROPPED_PLAYER_ITEM;

		drop->touch = Touch_Item;
		drop->nextthink = level.time + (self->_quad_framenum - level.framenum) * FRAMETIME;
		drop->think = G_FreeEdict;
	}
	*/
}

void	g_player_c::incWeaponFrame(float time)
{
	if((_weapon_update + time) < level.time)
	{
		_r.ps.gun_anim_frame++;	
		_weapon_update = level.time;
	}
}

/*
bool	g_player_c::addAmmo(g_item_c *item, int count)
{
	int			index;
	int			max;

	
	if(item->getTag() == AMMO_BULLETS)
		max = _pers.max_bullets;
		
	else if (item->getTag() == AMMO_SHELLS)
		max = _pers.max_shells;
		
	else if (item->getTag() == AMMO_ROCKETS)
		max = _pers.max_rockets;
		
	else if (item->getTag() == AMMO_GRENADES)
		max = _pers.max_grenades;
		
	else if (item->getTag() == AMMO_CELLS)
		max = _pers.max_cells;
		
	else if (item->getTag() == AMMO_SLUGS)
		max = _pers.max_slugs;
	else
		return false;

	index = G_GetNumForItem(item);

	if(_pers.inventory[index] == max)
		return false;

	_pers.inventory[index] += count;

	if(_pers.inventory[index] > max)
		_pers.inventory[index] = max;

	return true;
}
*/

void	g_player_c::noAmmoWeaponChange()
{
	/*
	if(_pers.inventory[G_GetNumForItem(G_FindItem("slugs"))] &&  _pers.inventory[G_GetNumForItem(G_FindItem("railgun"))] )
	{
		_newweapon = G_FindItem("railgun");
		return;
	}
	
	if(_pers.inventory[G_GetNumForItem(G_FindItem("cells"))] && _pers.inventory[G_GetNumForItem(G_FindItem("hyperblaster"))] )
	{
		_newweapon = G_FindItem("hyperblaster");
		return;
	}
	
	if(_pers.inventory[G_GetNumForItem(G_FindItem("bullets"))] &&  _pers.inventory[G_GetNumForItem(G_FindItem("chaingun"))] )
	{
		_newweapon = G_FindItem("chaingun");
		return;
	}
	
	if(_pers.inventory[G_GetNumForItem(G_FindItem("bullets"))] &&  _pers.inventory[G_GetNumForItem(G_FindItem("machinegun"))] )
	{
		_newweapon = G_FindItem("machinegun");
		return;
	}
	
	if(_pers.inventory[G_GetNumForItem(G_FindItem("shells"))] > 1 &&  _pers.inventory[G_GetNumForItem(G_FindItem("super shotgun"))] )
	{
		_newweapon = G_FindItem("super shotgun");
		return;
	}
	
	if(_pers.inventory[G_GetNumForItem(G_FindItem("shells"))] &&  _pers.inventory[G_GetNumForItem(G_FindItem("shotgun"))] )
	{
		_newweapon = G_FindItem("shotgun");
		return;
	}
	*/
	
//	_newweapon = (g_item_weapon_c*)G_FindItem("blaster");
}


void	g_player_c::projectSource(const vec3_c &offset, vec3_c &result)
{
	vec3_c noffset(offset);
	
	if(_pers.hand == LEFT_HANDED)
		noffset[1] *= -1;
		
	else if(_pers.hand == CENTER_HANDED)
		noffset[1] = 0;
		
	G_ProjectSource(_s.origin, noffset, _v_forward, _v_right, result);
}

/*
void	g_player_c::projectSource(const vec3_c &point, const vec3_c &distance, const vec3_c &forward, const vec3_c &right, vec3_c &result)
{
	vec3_c _distance = distance;
	
	if(_pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
		
	else if(_pers.hand == CENTER_HANDED)
		_distance[1] = 0;
		
	G_ProjectSource(point, _distance, forward, right, result);
}
*/

void	g_player_c::setViewAngles(const vec3_c &angles)
{
	_v_angles = angles;
	
	Angles_ToVectors(angles, _v_forward, _v_right, _v_up);
	
	_v_quat.fromAngles(angles);
}

void	g_player_c::moveToIntermission()
{
	if(deathmatch->getInteger() || coop->getInteger())
		_showscores = true;
		
	_s.origin = level.intermission_origin;
	
	_r.ps.pmove.origin[0] = level.intermission_origin[0];
	_r.ps.pmove.origin[1] = level.intermission_origin[1];
	_r.ps.pmove.origin[2] = level.intermission_origin[2];
	
	_r.ps.view_angles = level.intermission_angles;
	
	_r.ps.pmove.pm_type = PM_FREEZE;
	_r.ps.gun_model_index = 0;
	_r.ps.blend[3] = 0;
	_r.ps.rdflags &= ~RDF_UNDERWATER;

	// clean up powerup info
	_quad_framenum = 0;
	_invincible_framenum = 0;
	_breather_framenum = 0;
	_enviro_framenum = 0;
	_grenade_blew_up = false;
	_grenade_time = 0;

	_v_height = 0;
	_s.index_model = 0;
	_s.effects = 0;
	_s.index_sound = 0;
	_r.solid = SOLID_NOT;

	// add the layout
	if(deathmatch->getInteger() || coop->getInteger())
	{
		deathmatchScoreBoardMessage();
		trap_SV_Unicast(this, true);
	}
}

bool	g_player_c::hitFragLimit() const
{
	return _resp.score >= fraglimit->getInteger();
}







