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
#ifndef G_PLAYER_H
#define G_PLAYER_H

/// includes ===================================================================
// system -------------------------------------------------------------------
#include <deque>

// shared -------------------------------------------------------------------
#include "../x_shared.h"

// xreal --------------------------------------------------------------------
#include "g_entity.h"

enum //player_anim_torso_times_t	
{
	PLAYER_ANIM_BOTH_DEATH1_TIME	= 30,			// death animation
	PLAYER_ANIM_BOTH_DEATH2_TIME	= 30,
	PLAYER_ANIM_BOTH_DEATH3_TIME	= 30,
	
	PLAYER_ANIM_UPPER_GESTURE_TIME	= 40,			// wave	
	PLAYER_ANIM_UPPER_ATTACK1_TIME	= 6,			
	PLAYER_ANIM_UPPER_ATTACK2_TIME	= 6,
	PLAYER_ANIM_UPPER_FLIPOUT_TIME	= 5,			// weapon drop
	PLAYER_ANIM_UPPER_FLIPIN_TIME	= 4,			// weapon raise
	PLAYER_ANIM_UPPER_DROP_TIME	= 5,
	PLAYER_ANIM_UPPER_PAIN1_TIME	= 4,
	PLAYER_ANIM_UPPER_PAIN2_TIME	= 4,
	PLAYER_ANIM_UPPER_PAIN3_TIME	= 4
};


// client_t->anim_priority
enum player_anim_priority_t
{
	ANIM_BASIC,		// stand / run
	ANIM_WAVE,
	ANIM_PAIN,
	ANIM_ATTACK,
	ANIM_DEATH
};

// Jalisk0 - movement flags for animation control
enum
{
	ANIMMOVE_FRONT		= (1<<0),		//Player is pressing fordward
	ANIMMOVE_BACK		= (1<<1),		//Player is pressing backpedal
	ANIMMOVE_LEFT		= (1<<2),		//Player is pressing sideleft
	ANIMMOVE_RIGHT		= (1<<3),		//Player is pressing sideright
	ANIMMOVE_WALK		= (1<<4)		//Player is pressing the walk key
};

enum weapon_state_t
{
	WEAPON_ACTIVATING,
	WEAPON_READY,
	WEAPON_ATTACKING1,
	WEAPON_ATTACKING2,
	WEAPON_RELOADING,
	WEAPON_DEACTIVATING
};



//
// forward declarations
//
class g_item_c;
class g_item_weapon_c;


// client data that stays across multiple level loads
struct client_persistant_t
{
	void	clear()
	{
		userinfo.clear();
		memset(netname, 0, sizeof(netname));
		hand		= 0;

		connected	= false;
	
		health		= 0;
		max_health	= 0;
		saved_flags	= 0;
		
		selected_item	= 0;
		memset(inventory, 0, sizeof(inventory));

		max_bullets	= 0;
		max_shells	= 0;
		max_rockets	= 0;
		max_grenades	= 0;
		max_cells	= 0;
		max_slugs	= 0;
		
		weapon		= NULL;
		lastweapon	= NULL;

		score		= 0;

		spectator	= false;
	}
	
	info_c			userinfo;
	char			netname[16];
	int			hand;

	bool			connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;
	int			saved_flags;

	int			selected_item;
	int			inventory[MAX_ITEMS];

	// ammo capacities
	int			max_bullets;
	int			max_shells;
	int			max_rockets;
	int			max_grenades;
	int			max_cells;
	int			max_slugs;

	g_item_weapon_c*	weapon;
	g_item_weapon_c*	lastweapon;

	int			score;				// for calculating total unit score in coop game

	bool			spectator;			// client is a spectator
};



// client data that stays across deathmatch respawns
struct client_respawn_t
{
	void	clear()
	{
		coop_respawn.clear();
		enterframe	= 0;
		score		= 0;
		cmd_angles.clear();
		
		spectator	= false;
	}

	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	vec3_c			cmd_angles;			// angles sent over in the last command

	bool			spectator;			// client is a spectator
};






class g_player_c : public g_entity_c
{
public:
	//
	// constructor / destructor
	//
	g_player_c();
	virtual ~g_player_c();
	
		
	//
	// virtual functions
	//
//	virtual void		think();
//	virtual void		pain(g_entity_c *other, float kick, int damage);
	virtual void		die(g_entity_c *inflictor, g_entity_c *attacker, int damage, vec3_t point);
	
	virtual std::string	clientTeam();
	virtual void		takeDamage(	g_entity_c *inflictor, 	g_entity_c *attacker,
						vec3_t dir, vec3_t point, vec3_t normal,
						int damage, int knockback, int dflags, int mod	);
						
	
	
	//
	// functions
	//
	
	// game exporting functions
public:
	bool		clientConnect(info_c &userinfo);
	void		clientBegin();
private:
	void		clientBeginDeathmatch();
public:
	void		clientUserinfoChanged(info_c &userinfo);
	void		clientDisconnect();
	void		clientCommand();
	void		clientThink(const usercmd_t &cmd);
	
	
private:
	// misc
	void		lookAtKiller(g_entity_c *inflictor, g_entity_c *attacker);
	bool		isFemale();
	bool		isNeutral();
	void		clientObituary(g_entity_c *inflictor, g_entity_c *attacker);
	
public:
	void		moveToIntermission();
	
private:	
	// spawnpoint selection
	float		rangeFromSpot(g_entity_c *spot);
	g_entity_c*	selectRandomDeathmatchSpawnPoint();
	g_entity_c*	selectFarthestDeathmatchSpawnPoint();
	g_entity_c*	selectDeathmatchSpawnPoint();
	g_entity_c*	selectCoopSpawnPoint();
	void		selectSpawnPoint(vec3_c &origin, vec3_c &angles);
	
	// client setup
	void 		initClientPersistant();
	void		initClientResp();
	void		fetchClientEntData();
	void		putClientInServer();

public:
	void		respawn();
	void		respawnAsSpectator();
	
public:
	void		beginServerFrame();
	void		endServerFrame();

private:
	// endServerFrame specific functions	
	void		updateStats();
	void		updateSpectatorStats();
	void		checkChaseStats();
	void		updateWorldEffects();
	void		updateFallingDamage();
	void		updateDamageFeedback();
	void		updateClientEffects();
	void		updateClientEvent();
	void		updateClientSound();
	void		updateClientFrame();
	float		calcRoll(const vec3_c &angles, const vec3_c &velocity);
	void		calcViewOffset();
	void		calcGunOffset();
	void		calcBlend();
	void		addBlend(float r, float g, float b, float a, float *v_blend);
	
	// animation handling
	bool		isDucking();
	bool		isRunning();
	bool		isSwimming();
	bool		isStepping();
	
	bool		scanAnimations(const std::string &model);
	int		scanAnimation(const std::string &name);

public:
	// item handling
	void		selectNextItem(int item_flags);
	void		selectPrevItem(int item_flags);
	void		validateSelectedItem();
	g_entity_c*	dropItem(g_item_c *item);	

private:
	// client commands
	void		give_f();
	void		ultraman_f();
	void		notarget_f();
	void		noclip_f();
	
	void		use_f();
	void		drop_f();
	
	void		inven_f();
	void		invUse_f();
	void		invDrop_f();
	
	void		weapPrev_f();
	void		weapNext_f();
	void		weapLast_f();
	void		weapReload_f();
	
	void		kill_f();
	void		putAway_f();
	void		wave_f();
	void		say_f(bool team, bool arg0);
	void		playerList_f();
	void		players_f();
	void		score_f();	
	

public:
	// weapon handling
	void		thinkWeapon();
	void		changeWeapon();
	void		tossWeapon();
	void		incWeaponFrame(float time = (1.0/24.0));
		
	bool		addAmmo(g_item_c *item, int count);
	void		noAmmoWeaponChange();
	
	void		projectSource(const vec3_c &offset, vec3_c &result);
	//void		projectSource(const vec3_c &point, const vec3_c &offset, const vec3_c &forward, const vec3_c &right, vec3_c &result);
	
	
	void			setViewAngles(const vec3_c &angles);
	
	//
	// access
	//
	const vec3_c&		getViewAngles() const		{return _v_angles;}
	const vec3_c&		getViewForward() const		{return _v_forward;}
	const vec3_c&		getViewRight() const		{return _v_right;}
	const vec3_c&		getViewUp() const		{return _v_up;}
	const quaternion_c&	getViewQuaternion() const	{return _v_quat;}
	const vec_t		getViewHeight() const		{return _v_height;}
	
	const int		getAmmoIndex() const		{return _ammo_index;}
	
	//
	// members
	//
private:
	vec3_c			_v_angles;		// aiming direction
	vec3_c			_v_forward;
	vec3_c			_v_right;
	vec3_c			_v_up;
	quaternion_c		_v_quat;
	
	vec_t			_v_height;		// height above origin where eyesight is determined
	
	float			_xyspeed;
	
	float			_bob_time;		// so off-ground doesn't change it
	float			_bob_move;
	int			_bob_cycle;		// odd cycles are right foot going forward
	float			_bob_fracsin;		// sin(bobfrac*M_PI)
	
	pmove_state_t		_old_pmove;	// for detecting out-of-pmove changes
	
	bool			_showscores;		// set layout stat
	bool			_showinventory;		// set layout stat
	bool			_showhelp;
	bool			_showhelpicon;

	int			_ammo_index;
	
public:
	client_persistant_t	_pers;
	client_respawn_t	_resp;

	// cmd input
	std::deque<usercmd_t>	_cmds;
	int			_buttons;
	int			_buttons_old;
	int			_buttons_latched;	

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int			_damage_armor;		// damage absorbed by armor
	int			_damage_parmor;		// damage absorbed by power armor
	int			_damage_blood;		// damage taken out of health
	int			_damage_knockback;	// impact damage
	vec3_c			_damage_from;		// origin for vector calculation

	float			_killer_yaw;		// when dead, look at killer
	
	g_item_weapon_c*	_newweapon;
	weapon_state_t		_weapon_state;
	float			_weapon_update;		// last time the player state gun frame was changed
	bool			_weapon_thunk;
	bool			_weapon_fired;
			
	vec3_c			_kick_angles;		// weapon kicks
	vec3_c			_kick_origin;
	float			_v_dmg_roll;
	float			_v_dmg_pitch;
	float			_v_dmg_time;		// damage kicks
	float			_fall_time;
	float			_fall_value;		// for view drop on fall
	float			_damage_alpha;
	float			_bonus_alpha;
	vec3_c			_damage_blend;
	
	vec3_c			_oldviewangles;
	vec3_c			_oldvelocity;

	float			_next_drown_time;
	int			_old_waterlevel;
	int			_breather_sound;

	int			_machinegun_shots;	// for weapon raising
	
	float			_air_finished;

	// animation vars
	int			_anim_priority;
	
	bool			_anim_duck;
	bool			_anim_run;
	bool			_anim_swim;
	
	bool			_anim_jump;
	bool			_anim_jump_prestep;
	int			_anim_jump_style;
	
	int			_anim_moveflags;
	int			_anim_moveflags_old;
	
	int			_anim_current;
		
	int			_anim_time;
	
	int			_anim_lastsent;
	
	std::map<std::string, int>	_anims;		// key = name, value = time
	

	// powerup timers
	float			_quad_framenum;
	float			_invincible_framenum;
	float			_breather_framenum;
	float			_enviro_framenum;

	// grenade specific
	bool			_grenade_blew_up;
	float			_grenade_time;
	int			_weapon_sound;

	// flood protection
	float			_flood_locktill;	// locked from talking
	float			_flood_when[10];	// when messages were said
	int			_flood_whenhead;	// head pointer for when said	

	// timers
	float			_pickup_msg_time;
	float			_respawn_time;		// can respawn when time > this
	float			_jumppad_time;
	
	// ode
	d_amotor_joint_c	_joint_amotor;		// to keep player straight upwards
};




#endif // G_PLAYER_H


